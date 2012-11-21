/*
  +----------------------------------------------------------------------+
  | PHP Version 5														 |
  +----------------------------------------------------------------------+
  | Copyright 2012 Couchbase, Inc.										 |
  +----------------------------------------------------------------------+
  | Licensed under the Apache License, Version 2.0 (the "License");		 |
  | you may not use this file except in compliance with the License.	 |
  | You may obtain a copy of the License at								 |
  |		http://www.apache.org/licenses/LICENSE-2.0						 |
  | Unless required by applicable law or agreed to in writing, software	 |
  | distributed under the License is distributed on an "AS IS" BASIS,	 |
  | WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or		 |
  | implied. See the License for the specific language governing		 |
  | permissions and limitations under the License.						 |
  +----------------------------------------------------------------------+
  | Author: Michael Jackson											     |
  | Author: Mark Nunberg	   <mnunberg@haskalah.org>					 |
  +----------------------------------------------------------------------+
*/

/* This file contains the internal functions that drive observe (polling and
 * not) support. This is abstracted out because not only do the basic
 * observe and polling observe ("durability") functions need to use this, but
 * also the various mutation operations if the user has requested that the
 * extension try to guarantee this or that durability threshold.
 */

#include "internal.h"

#define COUCHBASE_OBSERVE_PERSIST_TO_KEY "persist_to"
#define COUCHBASE_OBSERVE_REPLICATE_TO_KEY "replicate_to"
#define COUCHBASE_OBSERVE_TIMEOUT_KEY "timeout"
#define COUCHBASE_OBSERVE_INTERVAL_KEY "interval"

#define COUCHBASE_OBSERVE_RESKEY_RESPONSES "responses"
#define COUCHBASE_OBSERVE_RESKEY_REPLICAS "replicas"
#define COUCHBASE_OBSERVE_RESKEY_PERSISTS "persisted"
#define COUCHBASE_OBSERVE_RESKEY_TTR "ttr"
#define COUCHBASE_OBSERVE_RESKEY_TTP "ttp"

#define COUCHBASE_OBSERVE_RESKEY_ERRSTR "error"
#define COUCHBASE_OBSERVE_RESKEY_ERRNUM "error_code"

//#define obs_debug(...) fprintf(stderr, __VA_ARGS__)
#define obs_debug(...)

struct observe_collection;

struct observe_expectation {
	short replicate;
	short persist;
	lcb_cas_t cas;
};

struct observe_pollprefs {
	long timeout;
	long interval;
};

struct observe_keystate {
	/**
	 * TODO: Make this into a linked list
	 */

	/**
	 * Actual command. This contains the key, so good to keep handy!
	 */
	struct lcb_observe_cmd_st ocmd;
	struct observe_collection *parent;

	/* whether we're done for this response. We should never set this twice! */
	int done;

	struct {
		short resp;
		char persist_master;
		char replicate;
		char persist;
		lcb_time_t ttp;
		lcb_time_t ttr;

		char *errstr;
		int errcode;

	} got;

	/* contains requirements mandated by the user for durability */
	struct observe_expectation expected;
};

struct observe_collection {
	php_couchbase_res *res;
	struct observe_keystate *ks;
	int nks;
	int remaining;

	/* whether the prefixes have been appended to the keys */
	int prefix_appended;

};

static void make_prefixed_hk(php_couchbase_res *res,
							 const char *key, int nkey,
							 pcbc_ht_key *hk)
{
	if (!res->prefix_key_len) {
		pcbc_ht_key_create(key, nkey, hk);

	} else {
		char *hks;
		spprintf(&hks, 0, "%s_%*s", res->prefix_key, nkey, key);
		pcbc_ht_key_create(hks, strlen(hks), hk);
		efree(hks);
	}
}

static lcb_cas_t cas_from_zval(zval *zv)
{
	char casbuf[64] = { 0 };
	char *endptr = NULL;
	lcb_cas_t ret = 0;

	/* convert the cas */
	if (Z_TYPE_P(zv) == IS_NULL ||
			(Z_TYPE_P(zv) == IS_LONG && Z_LVAL_P(zv) == 0) ||
			(Z_TYPE_P(zv) == IS_BOOL && Z_BVAL_P(zv) == 0)) {
		/* invalid, but fals-ish */
		return 0;

	}

	if (IS_STRING != Z_TYPE_P(zv) || Z_STRLEN_P(zv) > sizeof(casbuf)) {
		goto GT_ERR;
	}

	/* TODO: make this portable */
	memcpy(casbuf, Z_STRVAL_P(zv), Z_STRLEN_P(zv));
	ret = strtoull(casbuf, &endptr, 10);

	if (*endptr != '\0') {
		goto GT_ERR;
	}

	return ret;

GT_ERR:
	php_error(E_RECOVERABLE_ERROR,
			  "Invalid CAS Specified (must be a numeric string)");
	return -1;
}

/**
 * TODO:
 *
 * Use the new pure C structures; simply update their values.
 * i.e.
 */
static void oks_set_error(struct observe_keystate *oks,
						  const char *errstr, int clear_old)
{
	if (oks->got.errstr) {
		if (!clear_old) {
			return;
		}
		efree(oks->got.errstr);
		oks->got.errstr = NULL;
	}

	oks->got.errstr = estrdup(errstr);
}


static int oks_durability_satisfied(struct observe_keystate *oks)
{
	int ok_repl, ok_pers;

	ok_repl = (oks->expected.replicate == 0 ||
			   (oks->got.persist + oks->got.replicate >= oks->expected.replicate));

	ok_pers = (oks->expected.persist == 0 ||
			   (oks->got.persist >= oks->expected.persist &&
				oks->got.persist_master));

	return (ok_repl && ok_pers);
}

static void oks_set_done(struct observe_keystate *oks)
{
	if (oks->done) {
		return;
	}
	oks->done = 1;
	oks->parent->remaining--;

}

static void oks_update(struct observe_keystate *oks,
					   const lcb_observe_resp_t *resp)
{
	oks->got.resp++;

	if (resp->v.v0.status == LCB_OBSERVE_NOT_FOUND) {
		return; /* nothing to do here */
	}

	if (resp->v.v0.cas && oks->expected.cas &&
			resp->v.v0.cas != oks->expected.cas) {

		obs_debug("Cas Mismatch: Got %llu, Expected %llu\n",
				  resp->v.v0.cas, oks->expected.cas);

		oks_set_error(oks, "CAS Mismatch", 1);
		oks_set_done(oks);

		oks->got.persist = -1;
		oks->got.replicate = -1;
		oks->got.errcode = LCB_KEY_EEXISTS; /* meh */
		return;
	}

	switch (resp->v.v0.status) {

	case LCB_OBSERVE_PERSISTED:

		if (resp->v.v0.from_master) {
			oks->got.persist_master = 1;
		}

		oks->got.persist++;
		break;

	case LCB_OBSERVE_FOUND:

		if (resp->v.v0.from_master == 0) {
			oks->got.replicate++;
		}
		break;

	default:
		php_error(E_RECOVERABLE_ERROR,
				  "Got unhandled observe status (%d)",
				  resp->v.v0.status);
		break;

	}

	oks->got.ttp = resp->v.v0.ttp;
	oks->got.ttr = resp->v.v0.ttr;
	if (oks_durability_satisfied(oks)) {
		oks_set_done(oks);
	}
}
/**
 * Populates key-details for an oks structure
 */
static void oks_to_zvarray(struct observe_keystate *oks, zval *arry)
{
	add_assoc_long(arry, COUCHBASE_OBSERVE_RESKEY_RESPONSES, oks->got.resp);
	add_assoc_long(arry, COUCHBASE_OBSERVE_RESKEY_REPLICAS, oks->got.replicate);
	add_assoc_long(arry, COUCHBASE_OBSERVE_RESKEY_PERSISTS, oks->got.persist);
	add_assoc_long(arry, COUCHBASE_OBSERVE_RESKEY_TTR, oks->got.ttr);
	add_assoc_long(arry, COUCHBASE_OBSERVE_RESKEY_TTP, oks->got.ttp);

	if (oks->got.errstr) {
		zval *zvtmp;
		ALLOC_INIT_ZVAL(zvtmp);
		ZVAL_STRING(zvtmp, oks->got.errstr, 0);
		add_assoc_zval(arry, COUCHBASE_OBSERVE_RESKEY_ERRSTR, zvtmp);

		oks->got.errstr = NULL; /* zend takes ownership */

	} else {
		add_assoc_null(arry, COUCHBASE_OBSERVE_RESKEY_ERRSTR);
	}
	if (oks->got.errcode) {
		add_assoc_long(arry, COUCHBASE_OBSERVE_RESKEY_ERRNUM, oks->got.errcode);
	}
}

static int oks_get_boolval(struct observe_keystate *oks)
{
	int ret = oks_durability_satisfied(oks);
	if (ret) {
		ret = oks->got.errcode != LCB_KEY_EEXISTS;
	}
	return ret;
}

static int oks_extract_durability(php_couchbase_res *res,
								  struct observe_expectation *expectation,
								  struct observe_pollprefs *pollprefs,
								  zval *adurability)
{
	zval *tmpval;
	long tmplong = 0;
	int available = lcb_get_num_replicas(res->handle);

#define _must_get_long(k, gt) \
	tmplong = 0; \
	tmpval = NULL; \
	if ((tmpval = pcbc_ht_find(adurability, k, -1))) { \
		if (IS_LONG != Z_TYPE_P(tmpval)) { \
			php_error(E_RECOVERABLE_ERROR, k " must be numeric"); \
			return -1; \
		} \
		tmplong = Z_LVAL_P(tmpval); \
		if (tmplong < gt) { \
			php_error(E_RECOVERABLE_ERROR, k " must be greater than %d", gt); \
		} \
	}

	_must_get_long(COUCHBASE_OBSERVE_PERSIST_TO_KEY, 0);
	expectation->persist = tmplong;

	_must_get_long(COUCHBASE_OBSERVE_REPLICATE_TO_KEY, 0);
	expectation->replicate = tmplong;

	if (expectation->replicate > available) {
		php_error(E_WARNING,
				  "Not enough replicas (want=%d, max=%d). Capping",
				  expectation->replicate, available);
		expectation->replicate = available;
	}
	if (expectation->persist > available + 1) {
		php_error(E_WARNING,
				  "Not enough nodes for persistence (want=%d, max=%d). Capping",
				  expectation->persist, available + 1);
		expectation->persist = available + 1;
	}

	if (pollprefs) {
		_must_get_long(COUCHBASE_OBSERVE_TIMEOUT_KEY, 1);
		if (tmplong) {
			pollprefs->timeout = tmplong;
		} else {
			pollprefs->timeout = INI_INT(PCBC_INIENT_OBS_TIMEOUT);
		}

		_must_get_long(COUCHBASE_OBSERVE_INTERVAL_KEY, 1);
		if (tmplong) {
			pollprefs->interval = tmplong;
		} else {
			pollprefs->interval = INI_INT(PCBC_INIENT_OBS_INTERVAL);
		}

		if (pollprefs->timeout <= 0 || pollprefs->interval <= 0) {
			php_error(E_RECOVERABLE_ERROR,
					  "interval or timeout must be greater than 0");
			return -1;
		}

	}

	return 0;
#undef _must_get_long
}

static void oks_cleanup_context(struct observe_collection *ocoll)
{
	int ii;
	for (ii = 0; ii < ocoll->nks; ii++) {
		struct observe_keystate *oks = ocoll->ks + ii;
		if (oks->got.errstr) {
			efree(oks->got.errstr);
			oks->got.errstr = NULL;
		}

		if (oks->ocmd.v.v0.key) {
			efree((void *)oks->ocmd.v.v0.key);
			oks->ocmd.v.v0.key = NULL;
		}
	}

	efree(ocoll->ks);
	ocoll->ks = NULL;
	ocoll->nks = 0;
}


/**
 * Populate an observe collection with the appropriate key contexts
 * @param res the couchbase object
 * @param ocoll an already allocated collection object
 * @param akc a zend array mapping keys to their expected CAS values
 * @param expectation an array of durability requirements
 * @param append_prefix - whether the keys should be appended with the
 * 	couchbase-level prefix
 */
static int oks_build_context(php_couchbase_res *res,
							 struct observe_collection *ocoll,
							 struct observe_expectation *expectation,
							 zval *akc,
							 int append_prefix)
{
	int nks, ix;
	struct observe_keystate *ks;

	nks = pcbc_ht_len(akc);
	ks = ecalloc(sizeof(*ks), nks);

	for (ix = 0, pcbc_ht_iter_init(akc);
			pcbc_ht_iter_remaining(akc);
			pcbc_ht_iter_next(akc), ix++) {

		pcbc_ht_entry *kv = pcbc_ht_iter_entry(akc);
		struct observe_keystate *oks = ks + ix;

		if (kv->key_info->key_len == 0) {
			ix--;
			continue;
		}

		oks->parent = ocoll;
		if (expectation) {
			oks->expected = *expectation;
		}

		if ((oks->expected.cas = cas_from_zval(kv->data)) == -1) {
			pcbc_ht_entry_free(kv);
			goto GT_CLEANUP;
		}

		if (append_prefix && res->prefix_key_len &&
				ocoll->prefix_appended == 0) {

			pcbc_ht_key prefixed_ki;
			make_prefixed_hk(res, kv->key_info->key, kv->key_info->key_len,
							 &prefixed_ki);

			oks->ocmd.v.v0.nkey = prefixed_ki.key_len;
			oks->ocmd.v.v0.key = emalloc(prefixed_ki.key_len);

			memcpy((char *)oks->ocmd.v.v0.key,
				   prefixed_ki.key,
				   prefixed_ki.key_len);

			pcbc_ht_key_cleanup(&prefixed_ki);

		} else {
			oks->ocmd.v.v0.key = emalloc(kv->key_info->key_len);
			oks->ocmd.v.v0.nkey = kv->key_info->key_len;

			memcpy((char *)oks->ocmd.v.v0.key,
				   kv->key_info->key,
				   oks->ocmd.v.v0.nkey);
		}

		pcbc_ht_entry_free(kv);

		if (append_prefix && res->prefix_key_len) {
			ocoll->prefix_appended = 1;
		}
	}

	ocoll->ks = ks;
	ocoll->nks = ix;
	ocoll->remaining = ocoll->nks;
	ocoll->res = res;
	return 0;

GT_CLEANUP:
	for (ix = 0; ix < nks; ix++) {
		struct observe_keystate *oks = ks + ix;
		if (oks->ocmd.v.v0.key) {
			efree((void *) oks->ocmd.v.v0.key);
		}
	}
	efree(ks);
	return -1;
}


PHP_COUCHBASE_LOCAL
void php_couchbase_observe_callback(lcb_t instance,
									const void *cookie,
									lcb_error_t error,
									const lcb_observe_resp_t *resp)
{
	struct observe_keystate *oks = (struct observe_keystate *)cookie;

	oks->parent->res->rc = error;

	if (!resp->v.v0.nkey) {
		return;
	}

	if (error != LCB_SUCCESS) {
		oks_set_error(oks, lcb_strerror(instance, error), 1);
		oks_set_done(oks);
	}

	oks_update(oks, resp);
}

static int observe_iterate(php_couchbase_res *res,
						   struct observe_collection *ocoll)
{
	int ii, scheduled = 0;
	obs_debug("Have %d total requests\n", ocoll->nks);

	for (ii = 0; ii < ocoll->nks; ii++) {
		lcb_error_t err;
		struct observe_keystate *oks = ocoll->ks + ii;
		const lcb_observe_cmd_t *cmdp;

		if (oks->done) {
			continue;
		}

		/* reset the per-wait counters */
		if (oks->got.errstr) {
			efree(oks->got.errstr);
			oks->got.errstr = NULL;
		}

		memset(&oks->got, 0, sizeof(oks->got));

		cmdp = &oks->ocmd;
		err = lcb_observe(res->handle, oks, 1, &cmdp);

		if (err != LCB_SUCCESS) {
			oks_set_error(oks, lcb_strerror(res->handle, err), 1);
			oks_set_done(oks);
		}

		scheduled++;
	}

	if (!scheduled) {
		obs_debug("No commands scheduled..\n");
		return 0;
	}

	obs_debug("Waiting for %d commands\n", scheduled);
	pcbc_start_loop(res);

	/**
	 * Iterate again over the responses.
	 */

	for (ii = 0; ii < ocoll->nks; ii++) {
		struct observe_keystate *oks = ocoll->ks + ii;
		if (oks->done) {
			continue;
		}

		/**
		 * We get responses from the nodes each time. If we don't have enough
		 * total responses (NOT_FOUND or otherwise) it means there aren't
		 * that many nodes online.
		 */

		if (oks->expected.persist > oks->got.resp ||
				oks->expected.replicate > oks->got.resp) {
			oks_set_error(oks, "Not enough nodes for durability criteria", 1);
			oks_set_done(oks);
		}
	}

	return ocoll->remaining;
}

/**
 * The usec implementation seems to not work, and I don't know enough math to
 * fix it. Kittens will not die because we used milliseconds
 */
#define USE_MSEC_TIMINGS
#ifdef USE_MSEC_TIMINGS
static unsigned long get_msec_time(void)
{
	struct timeval tv;
	unsigned long ret = 0;
	gettimeofday(&tv, NULL);
	ret = tv.tv_sec * 1000;
	ret += tv.tv_sec / 1000;
	return ret;
}
#endif

static void observe_poll(php_couchbase_res *res,
						 struct observe_collection *ocoll,
						 struct observe_pollprefs *tprefs)
{

#ifdef USE_MSEC_TIMINGS
	unsigned long endtime = get_msec_time() + (tprefs->timeout / 1000);
#else
	/* doesn't seem to work. Let's use milliseconds */

	struct timeval start_time, end_time, curr_time;
	gettimeofday(&start_time, NULL);
	if (start_time.tv_usec + tprefs->timeout > 1000000) {
		end_time.tv_sec = start_time.tv_sec + 1;
		end_time.tv_usec = (start_time.tv_usec + tprefs->timeout) - 1000000;

	} else {
		end_time.tv_sec = start_time.tv_sec;
		end_time.tv_usec = start_time.tv_usec + tprefs->timeout;
	}
#endif

	while (observe_iterate(res, ocoll)) {
#ifdef USE_MSEC_TIMINGS
		unsigned long now = get_msec_time();
		if (now > endtime) {
			break;
		}

		usleep(tprefs->interval);
#else
		gettimeofday(&curr_time, NULL);

		if (curr_time.tv_sec > end_time.tv_sec ||
				(curr_time.tv_sec == end_time.tv_sec &&
				 curr_time.tv_usec > end_time.tv_usec)) {

		} else {
			usleep(tprefs->interval);
		}
#endif
	}
}

/**
 * TODO: Refactor this.
 * This is to be called from the other API functions. It should also include
 * output params for both detailed and summary results
 */
PHP_COUCHBASE_LOCAL
void observe_polling_internal(php_couchbase_ctx *ctx,
							  zval *adurability,
							  int modify_rv)
{
	struct observe_collection ocoll = { 0 };
	struct observe_expectation expect = { 0 };
	struct observe_pollprefs pollprefs = { 0 };

	if (oks_extract_durability(ctx->res, &expect, &pollprefs, adurability)
			== -1) {
		return;
	}

	if (oks_build_context(ctx->res, &ocoll, &expect, ctx->cas, 0) == -1) {
		return;
	}

	observe_poll(ctx->res, &ocoll, &pollprefs);

}
/* }}} */

/**
 * Populate the snapshot results from the observe operations
 * @param res the couchbase object
 * @param ocoll the observe collection
 * @param abools an array (or single) boolean
 * @param adetails an array of details to be indexed by key
 * @param multi boolean param - this determines whether abools is a scalar or
 *  an array
 */
static void oks_populate_results(php_couchbase_res *res,
								 struct observe_collection *ocoll,
								 zval *abools,
								 zval *adetails,
								 int multi)
{
	int ii;

	if (multi && IS_ARRAY != Z_TYPE_P(abools)) {
		array_init(abools);
	}

	for (ii = 0; ii < ocoll->nks; ii++) {
		/* get the key */
		pcbc_ht_key reski;
		zval *tmpary;

		struct observe_keystate *oks = ocoll->ks + ii;

		pcbc_ht_key_create(
			oks->ocmd.v.v0.key,
			oks->ocmd.v.v0.nkey,
			&reski);

		if (multi) {
			pcbc_ht_hkstoreb(abools,
							 &reski, oks_get_boolval(oks));

		} else {

			if (oks_get_boolval(oks)) {
				ZVAL_TRUE(abools);

			} else {
				ZVAL_FALSE(abools);
			}
		}

		if (adetails != NULL) {
			ALLOC_INIT_ZVAL(tmpary);
			array_init(tmpary);
			oks_to_zvarray(oks, tmpary);
			pcbc_ht_hkstorez(adetails, &reski, tmpary);
		}

		pcbc_ht_key_cleanup(&reski);
	}
}

PHP_COUCHBASE_LOCAL
void php_couchbase_observe_impl(INTERNAL_FUNCTION_PARAMETERS,
								int multi, int oo, int poll)
{

	zval *adurability = NULL,
		  *adetails = NULL; /* zval passed by ref, will be stuffed with details if given */

	php_couchbase_res *couchbase_res;
	struct observe_collection ocoll = { 0 };
	struct observe_expectation expect = { 0 };
	struct observe_pollprefs pollprefs;
	int argflags = oo ? PHP_COUCHBASE_ARG_F_OO : PHP_COUCHBASE_ARG_F_FUNCTIONAL;

	/* param handling, return value setup */
	if (multi) {
		zval *akey_to_cas;
		if (poll) {
			PHP_COUCHBASE_GET_PARAMS(couchbase_res, argflags, "aa",
									 &akey_to_cas, &adurability);
		} else {
			PHP_COUCHBASE_GET_PARAMS(couchbase_res, argflags,
									 "a|z", &akey_to_cas, &adetails);
		}

		array_init(return_value);

		if (poll) {
			if (-1 == oks_extract_durability(couchbase_res,
											 &expect, &pollprefs, adurability)) {
				RETURN_FALSE;
			}
		}

		if (-1 == oks_build_context(couchbase_res,
									&ocoll, &expect, akey_to_cas, 1)) {
			RETURN_FALSE;
		}

	} else { /* single */

		char *key = NULL;
		long nkey = 0;
		zval *cas;
		zval *akc_dummy = NULL;

		pcbc_ht_key dummy_hk;
		lcb_cas_t tmpcas = 0;
		ZVAL_FALSE(return_value);

		if (poll) {
			PHP_COUCHBASE_GET_PARAMS(couchbase_res, argflags,
									 "sza", &key, &nkey, &cas, &adurability);
		} else {
			PHP_COUCHBASE_GET_PARAMS(couchbase_res, argflags,
									 "sz|z", &key, &nkey, &cas, &adetails);
		}

		if (adetails && IS_ARRAY != Z_TYPE_P(adetails)) {
			array_init(adetails);
		}

		if (key == NULL || nkey == 0) {
			/* empty key */
			RETURN_FALSE;
		}

		make_prefixed_hk(couchbase_res, key, nkey, &dummy_hk);
		tmpcas = cas_from_zval(cas);

		if (tmpcas == -1) {
			pcbc_ht_key_cleanup(&dummy_hk);
			RETURN_FALSE;
		}

		ALLOC_INIT_ZVAL(akc_dummy);
		array_init(akc_dummy);

		if (tmpcas) {
			pcbc_ht_hkstores(akc_dummy, &dummy_hk,
							 Z_STRVAL_P(cas), Z_STRLEN_P(cas));
		} else {
			pcbc_ht_hkstoreb(akc_dummy, &dummy_hk, 0);
		}

		/* Weird block right here to sanely free the structures allocated */
		{
			int have_failure = 0;
			do {

				if (poll) {
					if (-1 == oks_extract_durability(couchbase_res,
													 &expect,
													 &pollprefs,
													 adurability)) {
						have_failure = 1;
						break;
					}
				}

				if (-1 == oks_build_context(
							couchbase_res, &ocoll, &expect, akc_dummy, 0)) {

					have_failure = 1;
					break;
				}
			} while (0);

			/** >> CLEANUP HERE */
			zval_ptr_dtor(&akc_dummy);
			pcbc_ht_key_cleanup(&dummy_hk);

			if (have_failure) {
				RETURN_FALSE;
			}
		}
	}

	if (adetails && Z_TYPE_P(adetails) != IS_ARRAY) {
		array_init(adetails);
	}

	if (poll) {
		observe_poll(couchbase_res, &ocoll, &pollprefs);

	} else {
		observe_iterate(couchbase_res, &ocoll);
	}

	oks_populate_results(couchbase_res,
						 &ocoll, return_value, adetails, multi);

	oks_cleanup_context(&ocoll);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

PHP_COUCHBASE_LOCAL
void php_couchbase_callbacks_observe_init(lcb_t handle)
{
	lcb_set_observe_callback(handle, php_couchbase_observe_callback);
}
