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
  | Author: Trond Norbye	   <trond.norbye@gmail.com>					 |
  +----------------------------------------------------------------------+
*/
#include "internal.h"

/* @todo This is a copy of the logic used by the cluster management
 *       I should refactor this out so I have a generic http execution
 *       method.
 */

struct flush_ctx {
	lcb_error_t error;
	lcb_http_status_t status;
	char *payload;
};

static void flush_callback(lcb_http_request_t request,
						   lcb_t instance,
						   const void *cookie,
						   lcb_error_t error,
						   const lcb_http_resp_t *resp)
{
	struct flush_ctx *ctx = (void *)cookie;
	assert(cookie != NULL);
	ctx->error = error;
	ctx->payload = NULL;

	if (resp->version != 0) {
		/* @todo add an error code I may use */
		ctx->error = LCB_NOT_SUPPORTED;
	} else {
		ctx->status = resp->v.v0.status;
		if (resp->v.v0.nbytes != 0) {
			ctx->payload = emalloc(resp->v.v0.nbytes + 1);
			if (ctx->payload != NULL) {
				memcpy(ctx->payload, resp->v.v0.bytes, resp->v.v0.nbytes);
				ctx->payload[resp->v.v0.nbytes] = '\0';
			}
		}
	}
}

PHP_COUCHBASE_LOCAL
void php_couchbase_flush_impl(INTERNAL_FUNCTION_PARAMETERS, int oo)
{
	php_couchbase_res *couchbase_res;
	struct flush_ctx ctx;
	lcb_error_t rc;
	lcb_http_cmd_t cmd;
	lcb_t instance;
	char *path;
	lcb_http_complete_callback old;

	int argflags = oo ? PHP_COUCHBASE_ARG_F_OO : PHP_COUCHBASE_ARG_F_FUNCTIONAL;
	PHP_COUCHBASE_GET_PARAMS(couchbase_res, argflags, "");

	instance = couchbase_res->handle;
	path = ecalloc(strlen(couchbase_res->bucket) + 80, 1);
	sprintf(path, "/pools/default/buckets/%s/controller/doFlush",
			couchbase_res->bucket);

	memset(&ctx, 0, sizeof(ctx));
	memset(&cmd, 0, sizeof(cmd));
	cmd.v.v0.path = path;
	cmd.v.v0.npath = strlen(path);
	cmd.v.v0.method = LCB_HTTP_METHOD_POST;
	cmd.v.v0.content_type = "application/x-www-form-urlencoded";

	old = lcb_set_http_complete_callback(instance, flush_callback);
	lcb_behavior_set_syncmode(instance, LCB_SYNCHRONOUS);
	rc = lcb_make_http_request(instance, &ctx, LCB_HTTP_TYPE_MANAGEMENT,
							   &cmd, NULL);
	lcb_behavior_set_syncmode(instance, LCB_ASYNCHRONOUS);
	old = lcb_set_http_complete_callback(instance, old);

	efree(path);
	if (rc == LCB_SUCCESS) {
		rc = ctx.error;
	}
	couchbase_res->rc = rc;

	if (rc != LCB_SUCCESS) {
		/* An error occured occurred on libcouchbase level */
		char errmsg[256];
		snprintf(errmsg, sizeof(errmsg), "Failed to flush bucket: %s",
				 lcb_strerror(instance, rc));

		if (ctx.payload) {
			efree(ctx.payload);
		}

		if (oo) {
			zend_throw_exception(cb_lcb_exception, errmsg, 0 TSRMLS_CC);
			return ;
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", errmsg);
			RETURN_FALSE;
		}
	}

	switch (ctx.status)  {
	case LCB_HTTP_STATUS_OK:
	case LCB_HTTP_STATUS_ACCEPTED:
		efree(ctx.payload);
		RETURN_TRUE;

	case LCB_HTTP_STATUS_UNAUTHORIZED:
		if (oo) {
			zend_throw_exception(cb_auth_exception, "Incorrect credentials",
								 0 TSRMLS_CC);
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
							 "Incorrect credentials");
			RETVAL_FALSE;
		}
		break;

	default:
		if (ctx.payload == NULL) {
			char message[200];
			sprintf(message, "{\"errors\":{\"http response\": %d }}",
					(int)ctx.status);
			if (oo) {
				zend_throw_exception(cb_server_exception, message, 0 TSRMLS_CC);
			} else {
				php_error_docref(NULL TSRMLS_CC, E_WARNING,
								 "%s", message);
				RETVAL_FALSE;
			}
		} else {
			if (oo) {
				zend_throw_exception(cb_server_exception, ctx.payload,
									 0 TSRMLS_CC);
			} else {
				php_error_docref(NULL TSRMLS_CC, E_WARNING,
								 "%s", ctx.payload);
				RETVAL_FALSE;
			}
		}
	}

	if (ctx.payload != NULL) {
		efree(ctx.payload);
	}
	/* exception already thrown */
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
