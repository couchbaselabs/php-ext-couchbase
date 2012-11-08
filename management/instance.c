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
  | Author: Trond Norbye	<trond.norbye@couchbase.com>				 |
  +----------------------------------------------------------------------+
*/

#include "internal.h"
#include "instance.h"

static void lcb_http_callback(lcb_http_request_t request,
                              lcb_t instance,
                              const void *cookie,
                              lcb_error_t error,
                              const lcb_http_resp_t *resp)
{
	struct lcb_http_ctx *ctx = (void *)cookie;
	assert(cookie != NULL);
	ctx->error = error;
	ctx->payload = NULL;

	if (resp->version != 0) {
		/* @todo add an error code I may use */
		ctx->error = LCB_NOT_SUPPORTED;
	} else {
		ctx->status = resp->v.v0.status;
		if (resp->v.v0.nbytes != 0) {
            if (ctx->use_emalloc) {
                ctx->payload = emalloc(resp->v.v0.nbytes + 1);
            } else {
                ctx->payload = malloc(resp->v.v0.nbytes + 1);
            }
			if (ctx->payload != NULL) {
				memcpy(ctx->payload, resp->v.v0.bytes, resp->v.v0.nbytes);
				ctx->payload[resp->v.v0.nbytes] = '\0';
			}
		}
	}
}


PHP_COUCHBASE_LOCAL
void ccm_create_impl(INTERNAL_FUNCTION_PARAMETERS)
{
	char *user = NULL;
	char *passwd = NULL;
	int ulen = 0;
	int plen = 0;
	zval *hosts = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zss",
                              &hosts, &user, &ulen,
	                          &passwd, &plen) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
		                 "Failed to parse parameters");
		RETURN_FALSE;
	} else {
		lcb_t handle;
		lcb_error_t retval;
		lcb_io_opt_t iops;
		php_couchbase_res *couchbase_res;
		php_couchbase_ctx *ctx = NULL;
		struct lcb_create_st copts = {0};
        char *allochosts = NULL;
        char *host = NULL;

        if (ulen == 0 || plen == 0) {
            zend_throw_exception(ccm_exception,
                                 "CouchbaseClusterManager require username/password",
                                 0 TSRMLS_CC);
            return;
        }

        if (hosts != NULL) {
            allochosts = calloc(4096, 1);
            host = allochosts;

            switch (Z_TYPE_P(hosts)) {
            case IS_ARRAY:
                {
                    int nhosts;
                    zval **curzv = NULL;
                    HashTable *hthosts = Z_ARRVAL_P(hosts);
                    HashPosition htpos;
                    int ii;
                    int pos = 0;

                    nhosts = zend_hash_num_elements(hthosts);

                    zend_hash_internal_pointer_reset_ex(hthosts, &htpos);

                    for (ii = 0; ii < nhosts && zend_hash_get_current_data_ex(hthosts, (void **)&curzv, &htpos) == SUCCESS; zend_hash_move_forward_ex(hthosts, &htpos), ii++) {
                        if (!Z_TYPE_PP(curzv) == IS_STRING) {
                            zend_throw_exception(ccm_exception,
                                                 "Element in the host array is not a string",
                                                 0 TSRMLS_CC);
                            free(allochosts);
                            return;
                        }
                        memcpy(allochosts + pos, Z_STRVAL_PP(curzv), Z_STRLEN_PP(curzv));
                        pos += Z_STRLEN_PP(curzv);
                        pos += sprintf(allochosts + pos, ";");
                    }
                }
                break;

            case IS_STRING:
                if (allochosts == NULL) {
                    zend_throw_exception(ccm_exception,
                                         "Failed to allocate memory",
                                         0 TSRMLS_CC);
                    return;
                }
                memcpy(allochosts, Z_STRVAL_P(hosts), Z_STRLEN_P(hosts));
                break;

            default:
                zend_throw_exception(ccm_exception,
                                     "hosts should be array or string",
                                     0 TSRMLS_CC);
                return;
            }
        }

		copts.version = 1;
		copts.v.v1.host = host;
		copts.v.v1.user = user;
		copts.v.v1.passwd = passwd;
		copts.v.v1.type = LCB_TYPE_CLUSTER;

		if (lcb_create(&handle, &copts) != LCB_SUCCESS) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
			                 "Failed to create libcouchbase instance");
            free(allochosts);
			RETURN_FALSE;
		}
        free(allochosts);

		// @todo fixme!
		/* lcb_set_error_callback(handle, php_couchbase_error_callback); */
		lcb_behavior_set_syncmode(handle, LCB_SYNCHRONOUS);
        lcb_set_http_complete_callback(handle, lcb_http_callback);

		ZEND_REGISTER_RESOURCE(return_value, handle, le_couchbase_cluster);
		zend_update_property(couchbase_cluster_ce, getThis(),
		                     ZEND_STRL(COUCHBASE_PROPERTY_HANDLE),
		                     return_value TSRMLS_CC);
	}
}

PHP_COUCHBASE_LOCAL
void ccm_get_info_impl(INTERNAL_FUNCTION_PARAMETERS)
{
	static const char uri[] = "/pools/default";
	zval *res;

	lcb_t instance;
	struct lcb_http_ctx ctx = { 0 };
	lcb_http_cmd_t cmd = { 0 };
	lcb_error_t rc;

    res = zend_read_property(couchbase_ce, getThis(),
                             ZEND_STRL(COUCHBASE_PROPERTY_HANDLE), 1
                             TSRMLS_CC);
    if (ZVAL_IS_NULL(res) || IS_RESOURCE != Z_TYPE_P(res)) {
        zend_throw_exception(ccm_exception, "unintilized couchbase",
                             0 TSRMLS_CC);
        return;
    }

    ctx.use_emalloc = 1;
	ZEND_FETCH_RESOURCE2(instance, lcb_t, &res, -1,
	                     PHP_COUCHBASE_CLUSTER_RESOURCE,
	                     le_couchbase_cluster, le_pcouchbase_cluster);

	cmd.v.v0.path = uri;
	cmd.v.v0.npath = strlen(uri);
	cmd.v.v0.body = NULL;
	cmd.v.v0.nbody = 0;
	cmd.v.v0.method = LCB_HTTP_METHOD_GET;
	cmd.v.v0.content_type = "application/x-www-form-urlencoded";

	rc = lcb_make_http_request(instance, &ctx,
	                           LCB_HTTP_TYPE_MANAGEMENT, &cmd, NULL);

	if (rc != LCB_SUCCESS || ctx.error != LCB_SUCCESS) {
		char errmsg[512];
		if (rc == LCB_SUCCESS) {
			rc = ctx.error;
		}
		snprintf(errmsg, sizeof(errmsg),
                 "Failed to get cluster information: %s",
		         lcb_strerror(instance, rc));
		zend_throw_exception(ccm_lcb_exception, errmsg, 0 TSRMLS_CC);
		efree(ctx.payload);
		return ;
	}

	switch (ctx.status)  {
	case LCB_HTTP_STATUS_OK:
	case LCB_HTTP_STATUS_ACCEPTED:
        RETURN_STRING(ctx.payload, 0);
        abort();

	case LCB_HTTP_STATUS_UNAUTHORIZED:
		zend_throw_exception(ccm_auth_exception, "Incorrect credentials",
		                     0 TSRMLS_CC);
		break;

	default:
		if (ctx.payload == NULL) {
			char message[200];
			sprintf(message, "{\"errors\":{\"http response\": %d }}",
			        (int)ctx.status);
			zend_throw_exception(ccm_server_exception, message, 0 TSRMLS_CC);
		} else {
			zend_throw_exception(ccm_server_exception, ctx.payload,
			                     0 TSRMLS_CC);
		}
	}

	/* exception is already thrown */
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
