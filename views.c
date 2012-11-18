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
  | Author: Mark Nunberg	   <mnunberg@haskalah.org>					 |
  +----------------------------------------------------------------------+
*/

#include "internal.h"

/* {{{ static void php_couchbase_complete_callback(...)
*/

/* append a string literal to a smart_str */
#define APPEND_URI_s(u, s) \
	smart_str_appendl(u, s, sizeof(s)-1)

typedef struct _php_couchbase_htinfo {
	int htstatus;
	size_t ndata;
	char data[1];
} php_couchbase_htinfo;


static void php_couchbase_complete_callback(lcb_http_request_t request,
                                     lcb_t instance,
                                     const void *cookie,
                                     lcb_error_t error,
                                     const lcb_http_resp_t *resp)
{
	php_couchbase_ctx *ctx = (php_couchbase_ctx *)cookie;
	php_couchbase_htinfo *hti;

	pcbc_stop_loop(ctx->res);

	if (resp->version != 0) {
		ctx->extended_value = NULL;
		ctx->res->rc = LCB_ERROR;
		return;
	}

	/** We have one extra byte in 'data' */
	hti = emalloc(sizeof(*hti) + resp->v.v0.nbytes);
	hti->ndata = resp->v.v0.nbytes;

	if (hti->ndata) {
		memcpy(hti->data, resp->v.v0.bytes, hti->ndata);
	}

	hti->data[hti->ndata] = '\0';

	ctx->res->rc = error;
	ctx->extended_value = hti;

	hti->htstatus = resp->v.v0.status;
}
/* }}} */


static void php_couchbase_extract_view_options(zval *options,
                                               smart_str *uri TSRMLS_DC)
{
	smart_str_appendc(uri, '?');

	for (
	    zend_hash_internal_pointer_reset(Z_ARRVAL_P(options));
	    zend_hash_has_more_elements(Z_ARRVAL_P(options)) == SUCCESS;
	    zend_hash_move_forward(Z_ARRVAL_P(options))) {

		char *key;
		uint klen;
		ulong idx;
		int type;
		zval **ppzval = NULL, tmpcopy;

		type = zend_hash_get_current_key_ex(Z_ARRVAL_P(options),
		                                    &key, &klen, &idx, 0, NULL);

		if (type != HASH_KEY_IS_STRING || klen == 0) {
			php_error_docref(NULL TSRMLS_CC, E_ERROR,
			                 "Got non- (or empty) string value "
			                 "for view parameters (%d)",
			                 type);
			continue;
		}

		if (FAILURE ==
		        zend_hash_get_current_data(
		            Z_ARRVAL_P(options), (void **)&ppzval)) {

			php_error_docref(NULL TSRMLS_CC, E_ERROR,
			                 "Couldn't get value for %*s", klen, key);
			continue;
		}

		/* Yes! The length *includes* the NUL byte */
		smart_str_appendl(uri, key, klen - 1);
		smart_str_appendc(uri, '=');

		tmpcopy = **ppzval;
		zval_copy_ctor(&tmpcopy);
		convert_to_string(&tmpcopy);
		smart_str_appendl(uri,
		                  Z_STRVAL(tmpcopy), Z_STRLEN(tmpcopy));
		zval_dtor(&tmpcopy);

		smart_str_appendc(uri, '&');
	}

	/* trim the last '&' from the uri */
	if (uri->c[uri->len - 1] == '&') {
		uri->len--;
	}
}

static void php_couchbase_sanitize_path(smart_str *uri,
                                        char *doc_name, long doc_name_len,
                                        char *view_name, long view_name_len)
{
	if (strcmp(doc_name, "_all_docs") == 0) {
		/* special case of _all_docs, which does not have its own design doc */
		APPEND_URI_s(uri, "_all_docs");

	} else if (view_name_len == 0) {
		/* i.e. we only have a single argument */
		smart_str_appendl(uri, doc_name, doc_name_len);

	} else {
		/* both design and doc. this is 'proper' usage */
		APPEND_URI_s(uri, "_design/");
		smart_str_appendl(uri, doc_name, doc_name_len);

		APPEND_URI_s(uri, "/_view/");
		smart_str_appendl(uri, view_name, view_name_len);
	}
}

/**
 * Extract errors from the view response and make it into a nice message.
 * If we have an error and reason field, return those. Otherwise return the
 * HTTP content string
 */
static char *php_couchbase_view_convert_to_error(zval *decoded,
                                                 php_couchbase_htinfo *hti)
{
	char *err = NULL, *reason = NULL;
	char *ret = NULL;

	zval **zverr = NULL, **zvreason = NULL;
	int nerr = 0, nreason = 0;

	if (IS_ARRAY == Z_TYPE_P(decoded)) {

		zend_hash_find(Z_ARRVAL_P(decoded),
		               "error", sizeof("error"),
		               (void **)&zverr);

		zend_hash_find(Z_ARRVAL_P(decoded),
		               "reason", sizeof("reason"),
		               (void **)&zvreason);
	}

	if (zverr == NULL && zvreason == NULL) {
		spprintf(&ret, 2048, "[%d, %*s]",
		         hti->htstatus, (int)hti->ndata, hti->data);

		return ret;
	}

	if (!zverr) {
		err = "Unknown Error";
		nerr = strlen(err);

	} else {
		err = Z_STRVAL_PP(zverr);
		nerr = Z_STRLEN_PP(zverr);
	}

	if (!zvreason) {
		reason = "Unknown reason";
		nreason = strlen(reason);

	} else {
		reason = Z_STRVAL_PP(zvreason);
		nreason = Z_STRLEN_PP(zvreason);
	}

	spprintf(&ret, 2048, "[%d, %*s, %*s]",
	         hti->htstatus,
	         (int)nerr, err, (int)nreason, reason);
	return ret;
}

PHP_COUCHBASE_LOCAL
void php_couchbase_view_impl(INTERNAL_FUNCTION_PARAMETERS, int oo) /* {{{ */
{
	lcb_http_request_t htreq;
	zval *res, *options = NULL;
	char *doc_name = NULL, *view_name = NULL;
	long doc_name_len = 0, view_name_len = 0;
	zend_bool return_errors = 0;

	lcb_error_t retval;
	smart_str uri = {0};
	php_couchbase_res *couchbase_res;
	php_couchbase_ctx ctx = {0};
	lcb_http_cmd_t cmd = {0};

	if (oo) {
		zval *self = getThis();
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
		                          "s|sab",
		                          &doc_name, &doc_name_len,
		                          &view_name, &view_name_len,
		                          &options, &return_errors) == FAILURE) {
			return;
		}
		res = zend_read_property(couchbase_ce, self,
		                         ZEND_STRL(COUCHBASE_PROPERTY_HANDLE),
		                         1 TSRMLS_CC);
		if (ZVAL_IS_NULL(res) || IS_RESOURCE != Z_TYPE_P(res)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "unintilized couchbase");
			RETURN_FALSE;
		}
	} else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
		                          "rs|sab", &res,
		                          &doc_name, &doc_name_len,
		                          &view_name, &view_name_len,
		                          &options, &return_errors) == FAILURE) {
			return;
		}
	}

	ZEND_FETCH_RESOURCE2(couchbase_res, php_couchbase_res *, &res, -1,
	                     PHP_COUCHBASE_RESOURCE, le_couchbase, le_pcouchbase);
	if (!couchbase_res->is_connected) {
		php_error(E_WARNING, "There is no active connection to couchbase.");
		RETURN_FALSE;
	}
	if (couchbase_res->async) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
		                 "there are some results should be fetched "
		                 "before do any sync request");
		RETURN_FALSE;
	}

	APPEND_URI_s(&uri, "/");

	php_couchbase_sanitize_path(&uri, doc_name, doc_name_len,
	                            view_name, view_name_len);

	if (options && memchr(uri.c, '?', uri.len) == NULL) {
		php_couchbase_extract_view_options(options, &uri TSRMLS_CC);
	}

	ctx.res = couchbase_res;

	memset(&cmd, 0, sizeof(cmd));

	cmd.v.v0.path = uri.c;
	cmd.v.v0.npath = uri.len;

	cmd.v.v0.method = LCB_HTTP_METHOD_GET;
	cmd.v.v0.content_type = "application/json";

	retval = lcb_make_http_request(couchbase_res->handle,
	                               (const void *)&ctx,
	                               LCB_HTTP_TYPE_VIEW,
	                               &cmd, &htreq);

	smart_str_free(&uri);

	if (LCB_SUCCESS != retval) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
		                 "Failed to schedule couch request: %s", lcb_strerror(couchbase_res->handle, retval));
		RETURN_FALSE;
	}

	pcbc_start_loop(couchbase_res);

	if (ctx.extended_value) {
		php_couchbase_htinfo *hti = ctx.extended_value;

#ifdef HAVE_JSON_API
# if HAVE_JSON_API_5_2
		php_json_decode(return_value, hti->data, hti->ndata, 1 TSRMLS_CC);
# elif HAVE_JSON_API_5_3
		php_json_decode(return_value, hti->data, hti->ndata, 1, 512 TSRMLS_CC);
# else
		ZVAL_FALSE(return_value);
		php_error_docref(NULL TSRMLS_CC, E_ERROR,
		                 "Unrecognized JSON decoder");
# endif

#else
		ZVAL_FALSE(return_value);
		php_error_docref(NULL TSRMLS_CC, E_ERROR,
		                 "Couldn't find JSON decoder");
#endif

		if (ctx.res->rc != LCB_SUCCESS && return_errors == 0) {
			char *errstr = php_couchbase_view_convert_to_error(
			                   return_value, hti);
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
			                 "Failed to execute view: Server Response: %s",
			                 errstr);

			efree(errstr);
			zval_dtor(return_value);
			ZVAL_FALSE(return_value);
		}

		efree(hti);
		return;

	} else {
		ZVAL_FALSE(return_value);
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
		                 "Failed to execute view: %s",
		                 lcb_strerror(couchbase_res->handle, ctx.res->rc));
	}
}

#undef APPEND_URI_s

PHP_COUCHBASE_LOCAL
void php_couchbase_callbacks_view_init(lcb_t handle)
{
	lcb_set_http_complete_callback(handle, php_couchbase_complete_callback);
}

/* }}} */
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet expandtab sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
