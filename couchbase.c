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
  | Author: Xinchen Hui	   <laruence@php.net>							 |
  +----------------------------------------------------------------------+
*/

#include "internal.h"

PHP_COUCHBASE_LOCAL
void pcbc_start_loop(struct _php_couchbase_res *res)
{
	lcb_wait(res->handle);
}

PHP_COUCHBASE_LOCAL
void pcbc_stop_loop(struct _php_couchbase_res *res)
{
	lcb_breakout(res->handle);
}


PHP_COUCHBASE_LOCAL
void php_couchbase_res_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC) /* {{{ */
{
	php_couchbase_res *couchbase_res = (php_couchbase_res *)rsrc->ptr;
	if (couchbase_res) {
		if (couchbase_res->handle) {
			lcb_destroy(couchbase_res->handle);
		}
		if (couchbase_res->prefix_key) {
			efree((void *)couchbase_res->prefix_key);
		}
		efree(couchbase_res);
	}
}
/* }}} */

PHP_COUCHBASE_LOCAL
void php_couchbase_pres_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC) /* {{{ */
{
	php_couchbase_res *couchbase_res = (php_couchbase_res *)rsrc->ptr;
	if (couchbase_res) {
		if (couchbase_res->handle) {
			lcb_destroy(couchbase_res->handle);
		}
		if (couchbase_res->prefix_key) {
			free((void *)couchbase_res->prefix_key);
		}
		free(couchbase_res);
	}
}
/* }}} */

/* callbacks */
static void php_couchbase_error_callback(lcb_t handle, lcb_error_t error, const char *errinfo) /* {{{ */
{
	php_couchbase_res *res = (php_couchbase_res *)lcb_get_cookie(handle);
	/**
	 * @FIXME: when connect to a non-couchbase-server port (but the socket is valid)
	 * like a apache server, process will be hanged by event_loop
	 */
	if (res && res->seqno < 0) {
		pcbc_stop_loop(res);
	}
}
/* }}} */


/* {{{ static void php_couchbase_flush_callback(...) */
static void php_couchbase_flush_callback(lcb_t handle,
                                         const void *cookie,
                                         lcb_error_t error,
                                         const lcb_flush_resp_t *resp)
{
	php_couchbase_ctx *ctx = (php_couchbase_ctx *)cookie;
	const char *server_endpoint = resp->v.v0.server_endpoint;
	php_ignore_value(handle);

	if (--ctx->res->seqno == 0) {
		pcbc_stop_loop(ctx->res);
	}

	if (server_endpoint) {
		ctx->extended_value = (void *)estrdup(server_endpoint);
	}
	ctx->res->rc = error;
}
/* }}} */


/* {{{ static void php_couchbase_stat_callback(...) */
static void php_couchbase_stat_callback(lcb_t handle,
                                        const void *cookie,
                                        lcb_error_t error,
                                        const lcb_server_stat_resp_t *resp)
{
	php_couchbase_ctx *ctx = (php_couchbase_ctx *)cookie;
	const char *server_endpoint = resp->v.v0.server_endpoint;
	const void *key = resp->v.v0.key;
	char *string_key;
	size_t nkey = resp->v.v0.nkey;
	const void *bytes = resp->v.v0.bytes;
	size_t nbytes = resp->v.v0.nbytes;

	php_ignore_value(handle);

	ctx->res->rc = error;
	if (LCB_SUCCESS != error || nkey == 0) {
		--ctx->res->seqno;
		pcbc_stop_loop(ctx->res);
		return;
	} else if (nkey > 0) {
		zval *node, *val;
		zval **ppzval;
		if (IS_ARRAY != Z_TYPE_P(ctx->rv)) {
			array_init(ctx->rv);
		}

		if (zend_hash_find(Z_ARRVAL_P(ctx->rv), (char *)server_endpoint, strlen(server_endpoint) + 1, (void **)&ppzval) == SUCCESS) {
			node = *ppzval;
		} else {
			MAKE_STD_ZVAL(node);
			array_init(node);
			zend_hash_add(Z_ARRVAL_P(ctx->rv), (char *)server_endpoint, strlen(server_endpoint) + 1, (void **)&node, sizeof(zval *), NULL);
		}

		string_key = emalloc(nkey + 1);
		memcpy(string_key, key, nkey);
		string_key[nkey] = '\0';

		add_assoc_string(node, string_key, (char *)bytes, 1);

		efree(string_key);
	}
}
/* }}} */

/* {{{ static void php_couchbase_version_callback(...) */
static void
php_couchbase_version_callback(lcb_t handle,
                               const void *cookie,
                               lcb_error_t error,
                               const lcb_server_version_resp_t *resp)
{
	php_couchbase_ctx *ctx = (php_couchbase_ctx *)cookie;
	const char *server_endpoint = resp->v.v0.server_endpoint;
	const char *version_string = resp->v.v0.vstring;
	lcb_size_t nversion_string = resp->v.v0.nvstring;
	php_ignore_value(handle);

	ctx->res->rc = error;
	if (LCB_SUCCESS != error || nversion_string == 0 || server_endpoint == NULL) {
		--ctx->res->seqno;
		pcbc_stop_loop(ctx->res);
		return;
	} else if (nversion_string > 0) {
		zval *v;
		zval **ppzval;

		if (IS_ARRAY != Z_TYPE_P(ctx->rv)) {
			array_init(ctx->rv);
		}

		if (zend_hash_find(Z_ARRVAL_P(ctx->rv), (char *)server_endpoint, strlen(server_endpoint) + 1, (void **)&ppzval) != SUCCESS) {
			MAKE_STD_ZVAL(v);
			ZVAL_STRINGL(v, version_string, nversion_string, 1);
			zend_hash_add(Z_ARRVAL_P(ctx->rv), (char *)server_endpoint, strlen(server_endpoint) + 1, (void **)&v, sizeof(zval *), NULL);
		}
	}
}
/* }}} */

struct php_couchbase_nodeinfo_st;

struct php_couchbase_nodeinfo_st {
	struct php_couchbase_nodeinfo_st *next;
	char *host;
	php_url *url;
};

struct php_couchbase_connparams_st {
	struct php_couchbase_nodeinfo_st *nodes;
	struct php_couchbase_nodeinfo_st *tail;

	char *host_string;
	char *bucket;
	char *username;
	char *password;
};

static int php_couchbase_parse_host(const char *host,
                                    size_t host_len,
                                    struct php_couchbase_connparams_st *cparams TSRMLS_DC)
{
	php_url *url = NULL;
	struct php_couchbase_nodeinfo_st *curnode;
	curnode = ecalloc(1, sizeof(*curnode));

	if (!cparams->tail) {
		cparams->nodes = cparams->tail = curnode;
	} else {
		cparams->tail->next = curnode;
		cparams->tail = curnode;
	}


	if (strncasecmp(host, "http://", sizeof("http://") - 1) != 0
	        && strncasecmp(host, "https://", sizeof("https://") - 1)
	        != 0) {
		/* simple host string */
		curnode->host = ecalloc(1, host_len + 1);
		memcpy(curnode->host, host, host_len);
		curnode->host[host_len] = '\0';
		return 1;
	}


	if (!(url = php_url_parse_ex(host, host_len))) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "malformed host url %s", host);
		return 0;
	}

	if (!url->host) {
		php_url_free(url);
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "malformed host url %s", host);
		return 0;
	}

	curnode->url = url;
	return 1;
}

static int _append_host_port(char *oldstr, char **newstr,
                             const char *host,
                             unsigned short port)
{
	if (oldstr) {
		if (port) {
			return spprintf(newstr, 0, "%s;%s:%d", oldstr, host, port);
		} else {
			return spprintf(newstr, 0, "%s;%s", oldstr, host);
		}
	} else {
		if (port) {
			return spprintf(newstr, 0, "%s:%d", host, port);
		} else {
			return spprintf(newstr, 0, "%s", host);
		}
	}
}

PHP_COUCHBASE_LOCAL
long pcbc_check_expiry(long expiry)
{
	if (expiry < 0) {
		php_error(E_RECOVERABLE_ERROR, "Expiry must not be negative (%ld given).", expiry);
	}
	return expiry;
}

static void php_couchbase_make_params(struct php_couchbase_connparams_st *cparams)
{
	struct php_couchbase_nodeinfo_st *ni;
	char *curstr = NULL;
	int curlen;

	for (ni = cparams->nodes; ni; ni = ni->next) {

		char *newstr = NULL;
		char *curhost;

		if (ni->url) {

			_append_host_port(curstr, &newstr, ni->url->host, ni->url->port);

			if (cparams->username == NULL) {
				cparams->username = ni->url->user;
			}

			if (cparams->password == NULL) {
				cparams->password = ni->url->pass;
			}

			if (cparams->bucket == NULL && ni->url->path != NULL && ni->url->path[0] == '/') {

				char *bucket = ni->url->path;
				int i = 0, j = strlen(bucket);

				if (*(bucket + j - 1) == '/') {
					*(bucket + j - 1) = '\0';
				}

				for (; i < j; i++) {
					bucket[i] = bucket[i + 1];
				}

				cparams->bucket = bucket;
			}

		} else {
			_append_host_port(curstr, &newstr, ni->host, 0);
		}

		if (curstr != NULL) {
			efree(curstr);
		}
		curstr = newstr;
	}
	cparams->host_string = curstr;
}

static void php_couchbase_free_connparams(
    struct php_couchbase_connparams_st *cparams)
{
	struct php_couchbase_nodeinfo_st *ni = cparams->nodes;
	while (ni) {

		struct php_couchbase_nodeinfo_st *next = ni->next;
		if (ni->url) {
			php_url_free(ni->url);
		} else if (ni->host) {
			efree(ni->host);
		}

		efree(ni);
		ni = next;
	}

	if (cparams->host_string) {
		efree(cparams->host_string);
	}
}

/* internal implementions */
PHP_COUCHBASE_LOCAL
void php_couchbase_create_impl(INTERNAL_FUNCTION_PARAMETERS, int oo) /* {{{ */
{
	char *user = NULL, *passwd = NULL, *bucket = NULL;
	int host_len = 0, user_len = 0, passwd_len = 0, bucket_len = 0;
	zend_bool persistent = 0;
	zval *zvhosts = NULL;
	struct php_couchbase_connparams_st cparams = { NULL };

	memset(&cparams, 0, sizeof(cparams));

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|zsssb",
	                          &zvhosts,
	                          &user, &user_len,
	                          &passwd, &passwd_len,
	                          &bucket, &bucket_len,
	                          &persistent) == FAILURE) {
		return;
	} else {
		lcb_t handle;
		lcb_error_t retval;
		php_couchbase_res *couchbase_res;
		char *hashed_key;
		uint hashed_key_len = 0;

		if (ZEND_NUM_ARGS() >= 4) {
			if (bucket_len) {
				cparams.bucket = bucket;
			}
			if (user_len) {
				cparams.username = user;
			}
			if (passwd_len) {
				cparams.password = passwd;
			}
		}
		if (zvhosts == NULL) {
			char host_localhost[] = "127.0.0.1";
			php_couchbase_parse_host(host_localhost, sizeof(host_localhost) - 1,
			                         &cparams TSRMLS_CC);

		} else if (Z_TYPE_P(zvhosts) == IS_STRING) {
			if (!php_couchbase_parse_host(
			            Z_STRVAL_P(zvhosts), Z_STRLEN_P(zvhosts), &cparams TSRMLS_CC)) {
				php_couchbase_free_connparams(&cparams);
				RETURN_FALSE;
			}
		} else if (Z_TYPE_P(zvhosts) == IS_ARRAY) {
			int nhosts;
			zval **curzv = NULL;
			HashTable *hthosts = Z_ARRVAL_P(zvhosts);
			HashPosition htpos;
			int ii;
			nhosts = zend_hash_num_elements(hthosts);

			for (ii = 0, zend_hash_internal_pointer_reset_ex(hthosts, &htpos);
			        ii < nhosts &&
			        zend_hash_get_current_data_ex(hthosts, (void **)&curzv, &htpos) == SUCCESS;
			        zend_hash_move_forward_ex(hthosts, &htpos), ii++) {
				if (!Z_TYPE_PP(curzv) == IS_STRING) {

					php_error_docref(NULL TSRMLS_CC, E_WARNING,
					                 "Couldn't get string from node lists");
					php_couchbase_free_connparams(&cparams);
					RETURN_FALSE;
				}
				if (!php_couchbase_parse_host(Z_STRVAL_PP(curzv),
				                              Z_STRLEN_PP(curzv),
				                              &cparams TSRMLS_CC)) {
					php_couchbase_free_connparams(&cparams);
					RETURN_FALSE;
				}
			}

		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
			                 "Hosts is neither a string nor an array");
			php_couchbase_free_connparams(&cparams);
			RETURN_FALSE;
		}

		php_couchbase_make_params(&cparams);


		if (persistent) {
			zend_rsrc_list_entry *le;
			hashed_key_len = spprintf(&hashed_key, 0, "couchbase_%s_%s_%s_%s",
			                          cparams.host_string, user, passwd, bucket);
			if (zend_hash_find(&EG(persistent_list), hashed_key, hashed_key_len + 1, (void **) &le) == FAILURE) {
				goto create_new_link;
			}
			couchbase_res = le->ptr;
			couchbase_res->seqno = 0;
			couchbase_res->async = 0;
			couchbase_res->serializer = COUCHBASE_G(serializer_real);
			couchbase_res->compressor = COUCHBASE_G(compressor_real);
			couchbase_res->ignoreflags = 0;
			efree(hashed_key);
		} else {
			struct lcb_create_st create_options;

create_new_link:
			if (!cparams.bucket) {
				cparams.bucket = "default";
			}

			memset(&create_options.version, 0, sizeof(create_options));
			create_options.v.v0.host = cparams.host_string;
			create_options.v.v0.user = cparams.username;
			create_options.v.v0.passwd = cparams.password;
			create_options.v.v0.bucket = cparams.bucket;

			if (lcb_create(&handle, &create_options) != LCB_SUCCESS) {
				php_couchbase_free_connparams(&cparams);
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to create libcouchbase instance");
				RETURN_FALSE;
			}

			php_ignore_value(lcb_set_error_callback(handle, php_couchbase_error_callback));

			retval = lcb_connect(handle);

			if (LCB_SUCCESS != retval) {
				php_error(E_WARNING, "Failed to connect libcouchbase to the server: %s", lcb_strerror(handle, retval));
			}

			php_couchbase_callbacks_arithmetic_init(handle);
			php_couchbase_callbacks_get_init(handle);
			php_couchbase_callbacks_store_init(handle);
			php_couchbase_callbacks_remove_init(handle);
			php_couchbase_callbacks_touch_init(handle);
			php_couchbase_callbacks_observe_init(handle);
			php_couchbase_callbacks_view_init(handle);

			php_ignore_value(lcb_set_flush_callback(handle, php_couchbase_flush_callback));
			php_ignore_value(lcb_set_stat_callback(handle, php_couchbase_stat_callback));
			php_ignore_value(lcb_set_version_callback(handle, php_couchbase_version_callback));

			couchbase_res = pecalloc(1, sizeof(php_couchbase_res), persistent);
			couchbase_res->handle = handle;
			couchbase_res->seqno = -1; /* tell error callback stop event loop when error occurred */
			couchbase_res->async = 0;
			couchbase_res->serializer = COUCHBASE_G(serializer_real);
			couchbase_res->compressor = COUCHBASE_G(compressor_real);
			couchbase_res->ignoreflags = 0;

			lcb_set_cookie(handle, (const void *)couchbase_res);

			/* wait for the connection established */
			if (LCB_SUCCESS == retval) { /* earlier lcb_connect's retval */
				lcb_wait(handle);
			}

			couchbase_res->seqno = 0;
			if (LCB_SUCCESS != (retval = lcb_get_last_error(handle))) {
				couchbase_res->rc = retval;
				couchbase_res->is_connected = 0;
				php_error(E_WARNING, "Failed to establish libcouchbase connection to server: %s", lcb_strerror(handle, retval));
			} else {
				couchbase_res->is_connected = 1;
			}

			if (persistent && couchbase_res->is_connected) {
				zend_rsrc_list_entry le;
				Z_TYPE(le) = le_pcouchbase;
				le.ptr = couchbase_res;
				if (zend_hash_update(&EG(persistent_list), hashed_key, hashed_key_len + 1, (void *) &le, sizeof(zend_rsrc_list_entry), NULL) == FAILURE) {
					php_couchbase_free_connparams(&cparams);
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to store persistent link");
				}
				efree(hashed_key);
			}
		}

		ZEND_REGISTER_RESOURCE(return_value, couchbase_res, persistent ? le_pcouchbase : le_couchbase);
		if (oo) {
			zval *self = getThis();
			zend_update_property(couchbase_ce, self, ZEND_STRL(COUCHBASE_PROPERTY_HANDLE), return_value TSRMLS_CC);
		} else if (!couchbase_res->is_connected) { /* !oo && !connected */
			php_couchbase_free_connparams(&cparams);
			RETURN_FALSE;
		}

		php_couchbase_free_connparams(&cparams);
	}
}
/* }}} */





PHP_COUCHBASE_LOCAL
void php_couchbase_flush_impl(INTERNAL_FUNCTION_PARAMETERS, int oo) /* {{{ */
{
	php_couchbase_res* couchbase_res;

	int argflags = oo ? PHP_COUCHBASE_ARG_F_OO : PHP_COUCHBASE_ARG_F_FUNCTIONAL;
	PHP_COUCHBASE_GET_PARAMS(couchbase_res, argflags, "");

	{
		lcb_error_t retval;
		php_couchbase_ctx *ctx;

		ctx = ecalloc(1, sizeof(php_couchbase_ctx));
		ctx->res = couchbase_res;
		{
			lcb_flush_cmd_t cmd;
			lcb_flush_cmd_t *commands[] = { &cmd };
			memset(&cmd, 0, sizeof(cmd));
			retval = lcb_flush(couchbase_res->handle, (const void *)ctx,
			                   1, (const lcb_flush_cmd_t * const *)commands);
		}
		if (LCB_SUCCESS != retval) {
			if (ctx->extended_value) {
				efree(ctx->extended_value);
			}
			efree(ctx);
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
			                 "Failed to schedule flush request: %s", lcb_strerror(couchbase_res->handle, retval));
			RETURN_FALSE;
		}

		couchbase_res->seqno += 1;
		pcbc_start_loop(couchbase_res);
		if (LCB_SUCCESS != ctx->res->rc) {
			if (ctx->extended_value) {
				efree(ctx->extended_value);
			}
			efree(ctx);
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
			                 "Failed to flush node %s: %s", ctx->extended_value ? (char *)ctx->extended_value : "", lcb_strerror(couchbase_res->handle, ctx->res->rc));
			RETURN_FALSE;
		}
		if (ctx->extended_value) {
			efree(ctx->extended_value);
		}
		efree(ctx);
	}

	if (oo) {
		RETURN_ZVAL(getThis(), 1, 0);
	}

	RETURN_TRUE;
}
/* }}} */


PHP_COUCHBASE_LOCAL
void php_couchbase_stats_impl(INTERNAL_FUNCTION_PARAMETERS, int oo) /* {{{ */
{
	php_couchbase_res *couchbase_res;
	int argflags = oo ? PHP_COUCHBASE_ARG_F_OO : PHP_COUCHBASE_ARG_F_FUNCTIONAL;
	PHP_COUCHBASE_GET_PARAMS(couchbase_res, argflags, "");

	{
		lcb_error_t retval;
		php_couchbase_ctx *ctx;

		ctx = ecalloc(1, sizeof(php_couchbase_ctx));
		ctx->res = couchbase_res;
		ctx->rv = return_value;

		{
			lcb_server_stats_cmd_t cmd;
			lcb_server_stats_cmd_t *commands[] = { &cmd };
			memset(&cmd, 0, sizeof(cmd));
			retval = lcb_server_stats(couchbase_res->handle, (const void *)ctx,
			                          1, (const lcb_server_stats_cmd_t * const *)commands);
		}
		if (LCB_SUCCESS != retval) {
			efree(ctx);
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
			                 "Failed to schedule stat request: %s", retval, lcb_strerror(couchbase_res->handle, retval));
			RETURN_FALSE;
		}

		couchbase_res->seqno += 1;
		pcbc_start_loop(couchbase_res);
		if (LCB_SUCCESS != ctx->res->rc) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
			                 "Failed to stat: %s", ctx->res->rc, lcb_strerror(couchbase_res->handle, ctx->res->rc));
			efree(ctx);
			RETURN_FALSE;
		}
		efree(ctx);
	}
}
/* }}} */

PHP_COUCHBASE_LOCAL
void php_couchbase_version_impl(INTERNAL_FUNCTION_PARAMETERS, int oo) /* {{{ */
{
	php_couchbase_res *couchbase_res;
	int argflags = oo ? PHP_COUCHBASE_ARG_F_OO : PHP_COUCHBASE_ARG_F_FUNCTIONAL;
	PHP_COUCHBASE_GET_PARAMS(couchbase_res, argflags, "");

	{
		lcb_error_t retval;
		php_couchbase_ctx *ctx;

		ctx = ecalloc(1, sizeof(php_couchbase_ctx));
		ctx->res = couchbase_res;
		ctx->rv = return_value;

		{
			lcb_server_version_cmd_t cmd;
			lcb_server_version_cmd_t *commands[] = { &cmd };
			memset(&cmd, 0, sizeof(cmd));
			retval = lcb_server_versions(couchbase_res->handle, (const void *)ctx,
			                             1, (const lcb_server_version_cmd_t * const *)commands);
		}
		if (LCB_SUCCESS != retval) {
			efree(ctx);
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
			                 "Failed to schedule server version request: %s", lcb_strerror(couchbase_res->handle, retval));
			RETURN_FALSE;
		}

		couchbase_res->seqno += 1;
		pcbc_start_loop(couchbase_res);
		if (LCB_SUCCESS != ctx->res->rc) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
			                 "Failed to fetch server version (%u): %s", ctx->res->rc, lcb_strerror(couchbase_res->handle, ctx->res->rc));
			efree(ctx);
			RETURN_FALSE;
		}
		efree(ctx);
	}
}
/* }}} */


PHP_COUCHBASE_LOCAL
void php_couchbase_set_option_impl(INTERNAL_FUNCTION_PARAMETERS, int oo) /* {{{ */
{
	long option;
	zval *res, *value;
	php_couchbase_res *couchbase_res;
	int argflags =
			(oo ? PHP_COUCHBASE_ARG_F_OO : PHP_COUCHBASE_ARG_F_FUNCTIONAL) |
			PHP_COUCHBASE_ARG_F_NOCONN | PHP_COUCHBASE_ARG_F_ASYNC;

	PHP_COUCHBASE_GET_PARAMS(couchbase_res, argflags, "lz", &option, &value);

	switch (option) {
	case COUCHBASE_OPT_SERIALIZER: {
		convert_to_long_ex(&value);
		switch (Z_LVAL_P(value)) {
		case COUCHBASE_SERIALIZER_PHP:
		case COUCHBASE_SERIALIZER_JSON:
		case COUCHBASE_SERIALIZER_JSON_ARRAY:
#ifdef HAVE_JSON_API
			couchbase_res->serializer = Z_LVAL_P(value);
			if (oo) {
				RETURN_ZVAL(getThis(), 1, 0);
			}
			RETURN_TRUE;
#else
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "json serializer is not supported");
			RETURN_FALSE;
#endif
			break;
		default:
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "unsupported serializer: %ld", Z_LVAL_P(value));
		}
	}
	case COUCHBASE_OPT_PREFIX_KEY: {
		convert_to_string_ex(&value);
		if (couchbase_res->prefix_key) {
			efree(couchbase_res->prefix_key);
		}
		couchbase_res->prefix_key = estrndup(Z_STRVAL_P(value), Z_STRLEN_P(value));
		couchbase_res->prefix_key_len = Z_STRLEN_P(value);
	}
	break;
	case COUCHBASE_OPT_COMPRESSION: {
		convert_to_long_ex(&value);
		switch (Z_LVAL_P(value)) {
		case COUCHBASE_COMPRESSION_NONE:
		case COUCHBASE_COMPRESSION_FASTLZ:
		case COUCHBASE_COMPRESSION_ZLIB:
			couchbase_res->compressor = Z_LVAL_P(value);
			if (oo) {
				RETURN_ZVAL(getThis(), 1, 0);
			}
			RETURN_TRUE;
			break;
		default:
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "unsupported compressor: %ld", Z_LVAL_P(value));
			break;
		}
	}
	break;
	case COUCHBASE_OPT_IGNOREFLAGS: {
		convert_to_long_ex(&value);
		couchbase_res->ignoreflags = Z_LVAL_P(value);
		break;
	}
	default:
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "unknown option type: %ld", option);
		break;
	}
	RETURN_FALSE;
}
/* }}} */

PHP_COUCHBASE_LOCAL
void php_couchbase_get_option_impl(INTERNAL_FUNCTION_PARAMETERS, int oo) /* {{{ */
{
	long option;
	php_couchbase_res *couchbase_res;
	int argflags =
			(oo ? PHP_COUCHBASE_ARG_F_OO : PHP_COUCHBASE_ARG_F_FUNCTIONAL) |
			PHP_COUCHBASE_ARG_F_ONLYVALID;

	PHP_COUCHBASE_GET_PARAMS(couchbase_res, argflags, "l", &option);

	switch (option) {
	case COUCHBASE_OPT_SERIALIZER:
		RETURN_LONG(couchbase_res->serializer);
		break;
	case COUCHBASE_OPT_PREFIX_KEY:
		if (couchbase_res->prefix_key_len) {
			RETURN_STRINGL((char *)couchbase_res->prefix_key, couchbase_res->prefix_key_len, 1);
		} else {
			ZVAL_EMPTY_STRING(return_value);
			return;
		}
		break;
	case COUCHBASE_OPT_COMPRESSION:
		RETURN_LONG(couchbase_res->compressor);
		break;
	case COUCHBASE_OPT_IGNOREFLAGS:
		RETURN_LONG(couchbase_res->ignoreflags);
		break;
	default:
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "unknown option type: %ld", option);
		break;
	}
	RETURN_FALSE;
}
/* }}} */

PHP_COUCHBASE_LOCAL
void php_couchbase_get_result_message_impl(INTERNAL_FUNCTION_PARAMETERS, int oo) /* {{{ */
{
	php_couchbase_res *couchbase_res;
	char *str;
	int str_len;
	int argflags = (oo ? PHP_COUCHBASE_ARG_F_OO : PHP_COUCHBASE_ARG_F_FUNCTIONAL)
			| PHP_COUCHBASE_ARG_F_ONLYVALID;
	PHP_COUCHBASE_GET_PARAMS(couchbase_res, argflags, "");

	str_len = spprintf(&str, 0, "%s", lcb_strerror(couchbase_res->handle, couchbase_res->rc));
	RETURN_STRINGL(str, str_len, 0);
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
