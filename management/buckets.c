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

/**
 * Usage:
 *
 *     $cb = new CouchbaseClusterManager("localhost:8091",
 *                                       "Administrator",
 *                                       "asdasd");
 *     $cb->createBucket("mybucket", array("type" => "couchbase",
 *                                         "quota" => 400,
 *                                         "replicas" => 1,
 *                                         "enable flush" => 1,
 *                                         "parallel compaction" => true,
 *                                         "auth" => "none",
 *                                         "port" => 11212));
 *
 *     $cb->modifyBucket("mybucket", array("auth" => "sasl",
 *                                         "password" => "secret",
 *                                         "port" => 11212));
 *     $cb->deleteBucket("mybucket");
 */

#include "instance.h"
#include "buckets.h"

struct string {
	char *buffer;
	unsigned len;
};

struct field {
	struct string value;
	int present;
	int allocated;
};

struct bucket_meta_info {
	struct field name;
	struct field auth_type;
	struct field bucket_type;
	struct field flush_enabled;
	struct field parallel_compaction;
	struct field proxy_port;
	struct field ram_quota;
	struct field replica_index;
	struct field replica_number;
	struct field sasl_password;
};

static void init_bucket_meta(struct bucket_meta_info *meta)
{
	memset(meta, 0, sizeof(*meta));
}

static void release_bucket_meta(struct bucket_meta_info *meta)
{
	if (meta->name.allocated) {
		free(meta->name.value.buffer);
	}
	if (meta->auth_type.allocated) {
		free(meta->auth_type.value.buffer);
	}
	if (meta->bucket_type.allocated) {
		free(meta->bucket_type.value.buffer);
	}
	if (meta->flush_enabled.allocated) {
		free(meta->flush_enabled.value.buffer);
	}
	if (meta->parallel_compaction.allocated) {
		free(meta->parallel_compaction.value.buffer);
	}
	if (meta->proxy_port.allocated) {
		free(meta->proxy_port.value.buffer);
	}
	if (meta->ram_quota.allocated) {
		free(meta->ram_quota.value.buffer);
	}
	if (meta->replica_index.allocated) {
		free(meta->replica_index.value.buffer);
	}
	if (meta->replica_number.allocated) {
		free(meta->replica_number.value.buffer);
	}
	if (meta->sasl_password.allocated) {
		free(meta->sasl_password.value.buffer);
	}
}

static int append(char *buffer, int offset,
                  const char *key,
                  const struct field *field)
{
	if (field->present && field->value.len > 0) {
		int ii;
		int r = strlen(key);
		if (offset != 0) {
			buffer[offset++] = '&';
		}
		memcpy(buffer + offset, key, r);
		offset += r;
		buffer[offset++] = '=';

		for (ii = 0; ii < field->value.len; ++ii) {
			if (isalpha(field->value.buffer[ii]) || isdigit(field->value.buffer[ii])) {
				buffer[offset++] = field->value.buffer[ii];
			} else {
				sprintf(buffer + offset, "%%%02X", field->value.buffer[ii]);
				offset += 3;
			}
		}
	}

	return offset;
}

static int meta_to_url(char *buffer, const struct bucket_meta_info *meta)
{
	int offset = 0;
	offset = append(buffer, offset, "name", &meta->name);
	offset = append(buffer, offset, "authType", &meta->auth_type);
	offset = append(buffer, offset, "bucketType", &meta->bucket_type);
	offset = append(buffer, offset, "flushEnabled", &meta->flush_enabled);
	offset = append(buffer, offset, "parallelDBAndViewCompaction",
	                &meta->parallel_compaction);
	offset = append(buffer, offset, "proxyPort", &meta->proxy_port);
	offset = append(buffer, offset, "ramQuotaMB", &meta->ram_quota);
	offset = append(buffer, offset, "replicaIndex", &meta->replica_index);
	offset = append(buffer, offset, "replicaNumber", &meta->replica_number);
	offset = append(buffer, offset, "saslPassword", &meta->sasl_password);

	return offset;
}

static void update_field(struct field *f, const struct string *v)
{
	free(f->value.buffer);
	f->present = 1;
	f->allocated = 1;
	f->value.buffer = malloc(v->len + 1);
	memcpy(f->value.buffer, v->buffer, v->len);
	f->value.len = v->len;
	f->value.buffer[v->len] = '\0';
}

static int set_value(const struct string *key,
                     const struct string *value,
                     struct bucket_meta_info *meta
                     TSRMLS_DC)
{
	char k[64];
	char *c;
	if (key->len > (sizeof(k) - 1)) {
		zend_throw_exception(ccm_illegal_key_exception,
		                     "Invalid key specified", 0 TSRMLS_CC);
		return -1;
	}

	memcpy(k, key->buffer, key->len);
	k[key->len] = '\0';

	/**
	 * Allow 'friendlier' names (spaces are not always the most natural)
	 */
	for (c = k; *c; c++) {
		if (*c == '_') {
			*c = ' ';
		}
	}

	if (strcmp(k, "name") == 0) {
		update_field(&meta->name, value);
	} else if (strcmp(k, "type") == 0) {
		update_field(&meta->bucket_type, value);
	} else if (strcmp(k, "auth") == 0) {
		update_field(&meta->auth_type, value);
	} else if (strcmp(k, "enable flush") == 0) {
		update_field(&meta->flush_enabled, value);
	} else if (strcmp(k, "parallel compaction") == 0) {
		update_field(&meta->parallel_compaction, value);
	} else if (strcmp(k, "port") == 0) {
		update_field(&meta->proxy_port, value);
	} else if (strcmp(k, "quota") == 0) {
		update_field(&meta->ram_quota, value);
	} else if (strcmp(k, "index replicas") == 0) {
		update_field(&meta->replica_index, value);
	} else if (strcmp(k, "replicas") == 0) {
		update_field(&meta->replica_number, value);
	} else if (strcmp(k, "password") == 0) {
		update_field(&meta->sasl_password, value);
	} else {
		char message[80];
		sprintf(message, "Unknown key: %s", k);
		zend_throw_exception(ccm_illegal_key_exception, message, 0 TSRMLS_CC);
		return -1;
	}

	return 0;
}

static int extract_bucket_options(zval *options,
                                  struct bucket_meta_info *meta TSRMLS_DC)
{
	int rval = 0;
	for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(options));
	        zend_hash_has_more_elements(Z_ARRVAL_P(options)) == SUCCESS &&
	        rval == 0;
	        zend_hash_move_forward(Z_ARRVAL_P(options))) {

		struct string key;
		struct string value;
		ulong idx;
		int type;
		zval **ppzval = NULL;
		zval tmpcopy;

		type = zend_hash_get_current_key_ex(Z_ARRVAL_P(options),
		                                    &key.buffer, &key.len,
		                                    &idx, 0, NULL);

		if (type != HASH_KEY_IS_STRING || key.len == 0) {
			char message[80];
			sprintf(message, "Got non- (or empty) string value "
			        "for bucket parameters (%d)",
			        type);
			zend_throw_exception(ccm_exception, message, 0 TSRMLS_CC);
			return -1;
		}

		if (zend_hash_get_current_data(Z_ARRVAL_P(options),
		                               (void **)&ppzval) == FAILURE) {
			char message[80];
			sprintf(message, "Couldn't get value for %*s", key.len,
			        key.buffer);
			zend_throw_exception(ccm_exception, message, 0 TSRMLS_CC);
			return -1;
		}

		tmpcopy = **ppzval;
		zval_copy_ctor(&tmpcopy);
		convert_to_string(&tmpcopy);
		value.buffer = Z_STRVAL(tmpcopy);
		value.len = Z_STRLEN(tmpcopy);
		rval = set_value(&key, &value, meta TSRMLS_CC);
		zval_dtor(&tmpcopy);
	}

	return rval;
}

PHP_COUCHBASE_LOCAL
void ccm_create_bucket_impl(INTERNAL_FUNCTION_PARAMETERS)
{
	static const char path[] = "/pools/default/buckets";
	struct bucket_meta_info meta;
	zval *res, *options = NULL;
	char name[64];
	lcb_t instance;
	struct lcb_http_ctx ctx = { 0 };
	lcb_http_cmd_t cmd = { 0 };
	lcb_error_t rc;
	char *data;
	int dlen;

	init_bucket_meta(&meta);

	res = zend_read_property(couchbase_ce, getThis(),
	                         ZEND_STRL(COUCHBASE_PROPERTY_HANDLE), 1
	                         TSRMLS_CC);
	if (ZVAL_IS_NULL(res) || IS_RESOURCE != Z_TYPE_P(res)) {
		zend_throw_exception(ccm_exception, "unintilized couchbase",
		                     0 TSRMLS_CC);
		return;
	}

	meta.name.present = 1;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sa",
	                          &meta.name.value.buffer,
	                          &meta.name.value.len,
	                          &options) == FAILURE) {
		return;
	}

	if (meta.name.value.len >= sizeof(name)) {
		memcpy(name, meta.name.value.buffer, sizeof(name) - 1);
		name[sizeof(name) - 1] = '\0';
	} else {
		memcpy(name, meta.name.value.buffer, meta.name.value.len);
		name[meta.name.value.len] = '\0';
	}

	ZEND_FETCH_RESOURCE2(instance, lcb_t, &res, -1,
	                     PHP_COUCHBASE_CLUSTER_RESOURCE,
	                     le_couchbase_cluster, le_pcouchbase_cluster);


	if (extract_bucket_options(options, &meta TSRMLS_CC) != 0) {
		/* Exception already thrown */
		return;
	}

	data = calloc(4096, 1); // should be more than enough ;) (but fixme)
	dlen = meta_to_url(data, &meta);
	cmd.v.v0.path = path;
	cmd.v.v0.npath = strlen(path);
	cmd.v.v0.body = data;
	cmd.v.v0.nbody = dlen;
	cmd.v.v0.method = LCB_HTTP_METHOD_POST;
	cmd.v.v0.content_type = "application/x-www-form-urlencoded";

	rc = lcb_make_http_request(instance, &ctx,
	                           LCB_HTTP_TYPE_MANAGEMENT, &cmd, NULL);

	free(data);

	release_bucket_meta(&meta);

	if (rc != LCB_SUCCESS || ctx.error != LCB_SUCCESS) {
		char errmsg[512];
		if (rc == LCB_SUCCESS) {
			rc = ctx.error;
		}
		snprintf(errmsg, sizeof(errmsg), "Failed to create bucket \"%s\": %s",
		         name, lcb_strerror(instance, rc));
		zend_throw_exception(ccm_lcb_exception, errmsg, 0 TSRMLS_CC);
		free(ctx.payload);
		return ;
	}

	switch (ctx.status)  {
	case LCB_HTTP_STATUS_OK:
	case LCB_HTTP_STATUS_ACCEPTED:
		free(ctx.payload);
		RETURN_TRUE;

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
	free(ctx.payload);
}

PHP_COUCHBASE_LOCAL
void ccm_delete_bucket_impl(INTERNAL_FUNCTION_PARAMETERS)
{
	zval *res;
	char *name = NULL;
	int name_len = 0;
	lcb_t instance;
	struct lcb_http_ctx ctx = { 0 };
	lcb_http_cmd_t cmd = { 0 };
	lcb_error_t rc;
	char *path;
	int plen;

	res = zend_read_property(couchbase_ce, getThis(),
	                         ZEND_STRL(COUCHBASE_PROPERTY_HANDLE),
	                         1 TSRMLS_CC);
	if (ZVAL_IS_NULL(res) || IS_RESOURCE != Z_TYPE_P(res)) {
		zend_throw_exception(ccm_exception, "unintilized couchbase",
		                     0 TSRMLS_CC);
		return;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name,
	                          &name_len) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE2(instance, lcb_t, &res, -1,
	                     PHP_COUCHBASE_CLUSTER_RESOURCE,
	                     le_couchbase_cluster, le_pcouchbase_cluster);

	path = calloc(sizeof("/pools/default/buckets/") + name_len + 1, 1);
	plen = sprintf(path, "/pools/default/buckets/");
	memcpy(path + plen, name, name_len);
	plen += name_len;

	cmd.v.v0.path = path;
	cmd.v.v0.npath = plen;
	cmd.v.v0.method = LCB_HTTP_METHOD_DELETE;
	cmd.v.v0.content_type = "application/x-www-form-urlencoded";

	rc = lcb_make_http_request(instance, &ctx,
	                           LCB_HTTP_TYPE_MANAGEMENT, &cmd, NULL);
	free(path);

	if (rc != LCB_SUCCESS || ctx.error != LCB_SUCCESS) {
		char errmsg[512];
		if (rc == LCB_SUCCESS) {
			rc = ctx.error;
		}
		snprintf(errmsg, sizeof(errmsg), "Failed to remove bucket \"%s\": %s",
		         name, lcb_strerror(instance, rc));
		zend_throw_exception(ccm_lcb_exception, errmsg, 0 TSRMLS_CC);
		free(ctx.payload);
		return ;
	}

	switch (ctx.status) {
	case LCB_HTTP_STATUS_OK:
		free(ctx.payload);
		RETURN_TRUE;
		;

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

	free(ctx.payload);
}

PHP_COUCHBASE_LOCAL
void ccm_modify_bucket_impl(INTERNAL_FUNCTION_PARAMETERS)
{
	char *path;
	int plen;
	char name[64];
	struct bucket_meta_info meta;
	zval *res, *options = NULL;
	lcb_t instance;
	struct lcb_http_ctx ctx = { 0 };
	lcb_http_cmd_t cmd = { 0 };
	lcb_error_t rc;
	char *data;
	int dlen;

	init_bucket_meta(&meta);

	res = zend_read_property(couchbase_ce, getThis(),
	                         ZEND_STRL(COUCHBASE_PROPERTY_HANDLE), 1
	                         TSRMLS_CC);
	if (ZVAL_IS_NULL(res) || IS_RESOURCE != Z_TYPE_P(res)) {
		zend_throw_exception(ccm_exception, "unintilized couchbase",
		                     0 TSRMLS_CC);
		return;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sa",
	                          &meta.name.value.buffer,
	                          &meta.name.value.len,
	                          &options) == FAILURE) {
		return;
	}

	if (meta.name.value.len >= sizeof(name)) {
		memcpy(name, meta.name.value.buffer, sizeof(name) - 1);
		name[sizeof(name) - 1] = '\0';
	} else {
		memcpy(name, meta.name.value.buffer, meta.name.value.len);
		name[meta.name.value.len] = '\0';
	}


	ZEND_FETCH_RESOURCE2(instance, lcb_t, &res, -1,
	                     PHP_COUCHBASE_CLUSTER_RESOURCE,
	                     le_couchbase_cluster, le_pcouchbase_cluster);

	extract_bucket_options(options, &meta TSRMLS_CC);

	path = calloc(sizeof("/pools/default/buckets/") + meta.name.value.len + 1, 1);
	plen = sprintf(path, "/pools/default/buckets/");
	memcpy(path + plen, meta.name.value.buffer, meta.name.value.len);
	plen += meta.name.value.len;

	data = calloc(4096, 1); // should be more than enough ;) (but fixme)
	dlen = meta_to_url(data, &meta);

	cmd.v.v0.path = path;
	cmd.v.v0.npath = plen;
	cmd.v.v0.body = data;
	cmd.v.v0.nbody = dlen;
	cmd.v.v0.method = LCB_HTTP_METHOD_POST;

	cmd.v.v0.method = LCB_HTTP_METHOD_POST;
	cmd.v.v0.content_type = "application/x-www-form-urlencoded";

	rc = lcb_make_http_request(instance, &ctx,
	                           LCB_HTTP_TYPE_MANAGEMENT, &cmd, NULL);

	free(data);
	free(path);

	release_bucket_meta(&meta);

	if (rc != LCB_SUCCESS || ctx.error != LCB_SUCCESS) {
		char errmsg[512];
		if (rc == LCB_SUCCESS) {
			rc = ctx.error;
		}
		snprintf(errmsg, sizeof(errmsg), "Failed to modify bucket \"%s\": %s",
		         name, lcb_strerror(instance, rc));
		zend_throw_exception(ccm_lcb_exception, errmsg, 0 TSRMLS_CC);
		free(ctx.payload);
		return ;
	}

	switch (ctx.status) {
	case LCB_HTTP_STATUS_OK:
	case LCB_HTTP_STATUS_ACCEPTED:
		free(ctx.payload);
		RETURN_TRUE;
		;

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

	free(ctx.payload);
}

PHP_COUCHBASE_LOCAL
void ccm_get_bucket_info_impl(INTERNAL_FUNCTION_PARAMETERS)
{
	static const char uri[] = "/pools/default/buckets";
    char *allocuri = NULL;
    char *requesturi = (char*)uri;
	zval *res;
    char *name;
    int name_len;

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

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &name,
	                          &name_len) == FAILURE) {
		return;
	}

    ctx.use_emalloc = 1;
	ZEND_FETCH_RESOURCE2(instance, lcb_t, &res, -1,
	                     PHP_COUCHBASE_CLUSTER_RESOURCE,
	                     le_couchbase_cluster, le_pcouchbase_cluster);


    if (name != NULL) {
        allocuri = malloc(strlen(uri) + name_len + 2);
        if (allocuri == NULL) {
            zend_throw_exception(ccm_exception, "failed to allocate memory",
                                 0 TSRMLS_CC);
            return;
        }
        sprintf(allocuri, "%s/%*s", uri, name_len, name);
        requesturi = allocuri;
    }

	cmd.v.v0.path = requesturi;
	cmd.v.v0.npath = strlen(requesturi);
	cmd.v.v0.body = NULL;
	cmd.v.v0.nbody = 0;
	cmd.v.v0.method = LCB_HTTP_METHOD_GET;
	cmd.v.v0.content_type = "application/x-www-form-urlencoded";

	rc = lcb_make_http_request(instance, &ctx,
	                           LCB_HTTP_TYPE_MANAGEMENT, &cmd, NULL);

    free(allocuri);

	if (rc != LCB_SUCCESS || ctx.error != LCB_SUCCESS) {
		char errmsg[512];
		if (rc == LCB_SUCCESS) {
			rc = ctx.error;
		}
		snprintf(errmsg, sizeof(errmsg),
                 "Failed to get bucket information: %s",
		         lcb_strerror(instance, rc));
		zend_throw_exception(ccm_lcb_exception, errmsg, 0 TSRMLS_CC);
		efree(ctx.payload);
		return ;
	}

	switch (ctx.status)  {
	case LCB_HTTP_STATUS_OK:
	case LCB_HTTP_STATUS_ACCEPTED:
        RETURN_STRING(ctx.payload, 0);

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
