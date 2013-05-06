/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright 2012 Couchbase, Inc.                                       |
  +----------------------------------------------------------------------+
  | Licensed under the Apache License, Version 2.0 (the "License");      |
  | you may not use this file except in compliance with the License.     |
  | You may obtain a copy of the License at                              |
  |     http://www.apache.org/licenses/LICENSE-2.0                       |
  | Unless required by applicable law or agreed to in writing, software  |
  | distributed under the License is distributed on an "AS IS" BASIS,    |
  | WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or      |
  | implied. See the License for the specific language governing         |
  | permissions and limitations under the License.                       |
  +----------------------------------------------------------------------+
  | Author: Trond Norbye    <trond.norbye@couchbase.com>                 |
  +----------------------------------------------------------------------+
*/

#include "internal.h"

struct http_ctx {
	lcb_error_t error;
	lcb_http_status_t status;
	char *payload;
};

static void http_callback(lcb_http_request_t request,
						  lcb_t instance,
						  const void *cookie,
						  lcb_error_t error,
						  const lcb_http_resp_t *resp)
{
	struct http_ctx *ctx = (void *)cookie;
	ctx->error = error;
	ctx->payload = NULL;

	if (resp->version != 0) {
		/* @todo add an error code I may use */
		ctx->error = LCB_NOT_SUPPORTED;
	} else {
		ctx->status = resp->v.v0.status;
		if (resp->v.v0.nbytes != 0) {
			ctx->payload = emalloc(resp->v.v0.nbytes + 1);
			if (ctx->payload == NULL) {
				ctx->error = LCB_CLIENT_ENOMEM;
			} else {
				memcpy(ctx->payload, resp->v.v0.bytes, resp->v.v0.nbytes);
				ctx->payload[resp->v.v0.nbytes] = '\0';
			}
		}
	}
}

PHP_COUCHBASE_LOCAL
void php_couchbase_delget_design_doc_impl(INTERNAL_FUNCTION_PARAMETERS, int oo, int rem)
{
	char *path;
	char *name = NULL;
	long nname = 0;
	php_couchbase_res *couchbase_res;
	int argflags;
	lcb_t instance;
	lcb_error_t rc;
	struct http_ctx ctx;
	lcb_http_cmd_t cmd;
	lcb_http_complete_callback old;
	int len;

	if (oo) {
		argflags = PHP_COUCHBASE_ARG_F_OO;
	} else {
		argflags = PHP_COUCHBASE_ARG_F_FUNCTIONAL;
	}

	PHP_COUCHBASE_GET_PARAMS(couchbase_res, argflags, "s", &name, &nname);

	if (!nname) {
		couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
							   cb_illegal_key_exception,
							   "You have to specify the name of the design doc");
		return;
	}

	memset(&ctx, 0, sizeof(ctx));
	memset(&cmd, 0, sizeof(cmd));
	instance = couchbase_res->handle;
	path = ecalloc(1, 80 + nname);
	len = sprintf(path, "/_design/%*s", (int)nname, name);
	cmd.v.v0.path = path;
	cmd.v.v0.npath = len;
	if (rem) {
		cmd.v.v0.method = LCB_HTTP_METHOD_DELETE;
	} else {
		cmd.v.v0.method = LCB_HTTP_METHOD_GET;
	}
	cmd.v.v0.content_type = "application/json";

	old = lcb_set_http_complete_callback(instance, http_callback);
	lcb_behavior_set_syncmode(instance, LCB_SYNCHRONOUS);
	rc = lcb_make_http_request(instance, &ctx, LCB_HTTP_TYPE_VIEW, &cmd, NULL);
	lcb_behavior_set_syncmode(instance, LCB_ASYNCHRONOUS);
	old = lcb_set_http_complete_callback(instance, old);

	efree(path);
	if (rc == LCB_SUCCESS) {
		rc = ctx.error;
	}
	couchbase_res->rc = rc;

	if (rc != LCB_SUCCESS) {
		/* An error occured occurred on libcouchbase level */
		efree(ctx.payload);
		couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
							   cb_lcb_exception,
							   "Failed to retrieve design doc: %s",
							   lcb_strerror(instance, rc));
		return;
	}

	switch (ctx.status)  {
	case LCB_HTTP_STATUS_OK:
		if (rem) {
			efree(ctx.payload);
			RETURN_TRUE;
		} else {
			RETURN_STRING(ctx.payload, 0);
		}
		/* not reached */

	case LCB_HTTP_STATUS_NOT_FOUND:
		efree(ctx.payload);
		RETURN_FALSE;

	case LCB_HTTP_STATUS_UNAUTHORIZED:
		couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
							   cb_auth_exception, "Incorrect credentials");
		break;

	default:
		if (ctx.payload == NULL) {
			couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
								   cb_server_exception,
								   "{\"errors\":{\"http response\": %d }}",
								   (int)ctx.status);
		} else {
			couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
								   cb_server_exception,
								   ctx.payload);
		}
	}

	efree(ctx.payload);
}

PHP_COUCHBASE_LOCAL
void php_couchbase_set_design_doc_impl(INTERNAL_FUNCTION_PARAMETERS, int oo)
{
	char *path;
	char *name = NULL;
	long nname = 0;
	char *doc = NULL;
	long ndoc = 0;
	php_couchbase_res *couchbase_res;
	int argflags;
	lcb_t instance;
	lcb_error_t rc;
	struct http_ctx ctx;
	lcb_http_cmd_t cmd;
	lcb_http_complete_callback old;
	int len;

	if (oo) {
		argflags = PHP_COUCHBASE_ARG_F_OO;
	} else {
		argflags = PHP_COUCHBASE_ARG_F_FUNCTIONAL;
	}

	PHP_COUCHBASE_GET_PARAMS(couchbase_res, argflags, "ss", &name, &nname,
							 &doc, &ndoc);

	if (!nname) {
		couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
							   cb_illegal_key_exception,
							   "You have to specify the name of the design doc");
		return;
	}
	if (!doc) {
		couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
							   cb_exception,
							   "You can't store an empty document");
		return;
	}

	memset(&ctx, 0, sizeof(ctx));
	memset(&cmd, 0, sizeof(cmd));
	instance = couchbase_res->handle;
	path = ecalloc(1, 80 + nname);
	len = sprintf(path, "/_design/%*s", (int)nname, name);
	cmd.v.v0.path = path;
	cmd.v.v0.npath = len;
	cmd.v.v0.body = doc;
	cmd.v.v0.nbody = ndoc;
	cmd.v.v0.method = LCB_HTTP_METHOD_PUT;
	cmd.v.v0.content_type = "application/json";

	old = lcb_set_http_complete_callback(instance, http_callback);
	lcb_behavior_set_syncmode(instance, LCB_SYNCHRONOUS);
	rc = lcb_make_http_request(instance, &ctx, LCB_HTTP_TYPE_VIEW, &cmd, NULL);
	lcb_behavior_set_syncmode(instance, LCB_ASYNCHRONOUS);
	old = lcb_set_http_complete_callback(instance, old);

	efree(path);
	if (rc == LCB_SUCCESS) {
		rc = ctx.error;
	}
	couchbase_res->rc = rc;

	if (rc != LCB_SUCCESS) {
		/* An error occured occurred on libcouchbase level */
		efree(ctx.payload);
		couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
							   cb_lcb_exception,
							   "Failed to store design doc: %s",
							   lcb_strerror(instance, rc));
		return;
	}

	switch (ctx.status)  {
	case LCB_HTTP_STATUS_OK:
	case LCB_HTTP_STATUS_CREATED:
	case LCB_HTTP_STATUS_ACCEPTED:
		efree(ctx.payload);
		RETURN_TRUE;

	case LCB_HTTP_STATUS_UNAUTHORIZED:
		couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
							   cb_auth_exception, "Incorrect credentials");
		break;

	default:
		if (ctx.payload == NULL) {
			couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
								   cb_server_exception,
								   "{\"errors\":{\"http response\": %d }}",
								   (int)ctx.status);
		} else {
			couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
								   cb_server_exception,
								   ctx.payload);
		}
	}

	efree(ctx.payload);
}

PHP_COUCHBASE_LOCAL
void php_couchbase_list_design_docs_impl(INTERNAL_FUNCTION_PARAMETERS, int oo)
{
	php_couchbase_res *couchbase_res;
	int argflags;
	lcb_t instance;
	lcb_error_t rc;
	struct http_ctx ctx;
	lcb_http_cmd_t cmd;
	lcb_http_complete_callback old;
	int len;
	char *path;

	if (oo) {
		argflags = PHP_COUCHBASE_ARG_F_OO;
	} else {
		argflags = PHP_COUCHBASE_ARG_F_FUNCTIONAL;
	}

	PHP_COUCHBASE_GET_PARAMS(couchbase_res, argflags, "");
	instance = couchbase_res->handle;

	memset(&ctx, 0, sizeof(ctx));
	memset(&cmd, 0, sizeof(cmd));

	path = ecalloc(strlen(couchbase_res->bucket) + 80, 1);
	len = sprintf(path, "/pools/default/buckets/%s/ddocs",
				  couchbase_res->bucket);
	cmd.v.v0.path = path;
	cmd.v.v0.npath = len;
	cmd.v.v0.method = LCB_HTTP_METHOD_GET;
	cmd.v.v0.content_type = "application/json";

	old = lcb_set_http_complete_callback(instance, http_callback);
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
		efree(ctx.payload);
		couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
							   cb_lcb_exception,
							   "Failed to retrieve design doc: %s",
							   lcb_strerror(instance, rc));
		return;
	}

	switch (ctx.status)  {
	case LCB_HTTP_STATUS_OK:
		RETURN_STRING(ctx.payload, 0);

	case LCB_HTTP_STATUS_UNAUTHORIZED:
		couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
							   cb_auth_exception, "Incorrect credentials");
		break;

	default:
		if (ctx.payload == NULL) {
			couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
								   cb_server_exception,
								   "{\"errors\":{\"http response\": %d }}",
								   (int)ctx.status);
		} else {
			couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
								   cb_server_exception,
								   ctx.payload);
		}
	}

	efree(ctx.payload);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet expandtab sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
