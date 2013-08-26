/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright 2013 Couchbase, Inc.                                       |
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
*/
#include "internal.h"


struct nodeinfo_st;

struct nodeinfo_st {
	struct nodeinfo_st *next;
	char *host;
	php_url *url;
};

struct connparams_st {
	struct nodeinfo_st *nodes;
	struct nodeinfo_st *tail;

	char *host_string;
	char *bucket;
	char *username;
	char *password;
};

static int hostcmp(const void *a, const void *b)
{
	return strcmp(*(char * const *)a, *(char * const *)b);
}

static char *sort_hosts(const char *hosts)
{
	int num = 1;
	char **ptrs;
	char *copy = strdup(hosts);
	char *ret = strdup(hosts);
	char *curr = copy;
	int ii;

	if (copy == NULL || ret == NULL) {
		free(copy);
		free(ret);
		return NULL;
	}

	while ((curr = strchr(curr, ';')) != NULL) {
		*curr = '\0';
		num++;
		curr++;
	}

	if (num == 1) {
		/* single entry */
		free(copy);
		return ret;
	}

	if ((ptrs = calloc(num, sizeof(char *))) == NULL) {
		free(copy);
		free(ret);
		return NULL;
	}

	curr = copy;
	for (ii = 0; ii < num; ++ii) {
		ptrs[ii] = curr;
		if (ii != (num - 1)) {
			curr = strchr(curr + 1, '\0') + 1;
		}
	}

	qsort(ptrs, num, sizeof(char *), hostcmp);

	curr = ret;
	for (ii = 0; ii < num; ++ii) {
		strcpy(curr, ptrs[ii]);
		curr += strlen(ptrs[ii]);
		if (ii != (num - 1)) {
			*curr = ';';
			curr++;
		}
	}

	free(copy);
	free(ptrs);

	return ret;
}

static int parse_host(const char *host,
					  size_t host_len, struct connparams_st *cparams TSRMLS_DC)
{
	php_url *url = NULL;
	struct nodeinfo_st *curnode;
	curnode = ecalloc(1, sizeof(*curnode));

	if (!cparams->tail) {
		cparams->nodes = cparams->tail = curnode;
	} else {
		cparams->tail->next = curnode;
		cparams->tail = curnode;
	}


	if (strncasecmp(host, "http://", sizeof("http://") - 1) != 0
			&& strncasecmp(host, "https://", sizeof("https://") - 1) != 0) {
		/* simple host string */
		curnode->host = ecalloc(1, host_len + 1);
		memcpy(curnode->host, host, host_len);
		curnode->host[host_len] = '\0';
		return 1;
	}


	if (!(url = php_url_parse_ex(host, host_len))) {
		return 0;
	}

	if (!url->host) {
		php_url_free(url);
		return 0;
	}

	curnode->url = url;
	return 1;
}

static int _append_host_port(char *oldstr, char **newstr,
							 const char *host, unsigned short port)
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

static void make_params(struct connparams_st *cparams)
{
	struct nodeinfo_st *ni;
	char *curstr = NULL;

	for (ni = cparams->nodes; ni; ni = ni->next) {

		char *newstr = NULL;

		if (ni->url) {

			_append_host_port(curstr, &newstr, ni->url->host, ni->url->port);

			if (cparams->username == NULL) {
				cparams->username = ni->url->user;
			}

			if (cparams->password == NULL) {
				cparams->password = ni->url->pass;
			}

			if (cparams->bucket == NULL && ni->url->path != NULL &&
					ni->url->path[0] == '/') {

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

static void free_connparams(struct connparams_st *cparams)
{
	struct nodeinfo_st *ni = cparams->nodes;
	while (ni) {

		struct nodeinfo_st *next = ni->next;
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
void php_couchbase_create_impl(INTERNAL_FUNCTION_PARAMETERS, int oo)
{
	char *user = NULL, *passwd = NULL, *bucket = NULL;
	int user_len = 0, passwd_len = 0, bucket_len = 0;
	zend_bool persistent = COUCHBASE_G(persistent);
	zval *zvhosts = NULL;
	struct connparams_st cparams = { NULL };

	lcb_t handle;
	lcb_error_t retval;
	php_couchbase_res *couchbase_res;
	char *hashed_key;
	uint hashed_key_len = 0;


	memset(&cparams, 0, sizeof(cparams));

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|zsssb",
							  &zvhosts,
							  &user, &user_len,
							  &passwd, &passwd_len,
							  &bucket, &bucket_len,
							  &persistent) == FAILURE) {
		return;
	}

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
		parse_host(host_localhost, sizeof(host_localhost) - 1,
				   &cparams TSRMLS_CC);

	} else if (Z_TYPE_P(zvhosts) == IS_STRING) {
		if (!parse_host(Z_STRVAL_P(zvhosts), Z_STRLEN_P(zvhosts),
						&cparams TSRMLS_CC)) {
			free_connparams(&cparams);
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
				free_connparams(&cparams);
				couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
									   cb_exception,
									   "Couldn't get string from node lists");
				return;
			}
			if (!parse_host(Z_STRVAL_PP(curzv),
							Z_STRLEN_PP(curzv),
							&cparams TSRMLS_CC)) {
				free_connparams(&cparams);
				couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
									   cb_exception,
									   "malformed host URL");
				return;
			}
		}
	} else {
		free_connparams(&cparams);
		couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
							   cb_exception,
							   "Hosts is neither a string nor an array");
		return;
	}

	make_params(&cparams);


	if (persistent) {
		zend_rsrc_list_entry *le;
		char *sorted_hosts = sort_hosts(cparams.host_string);
		if (sorted_hosts == NULL) {
			couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
								   cb_exception,
								   "Failed to allocate memory");
			return;
		}

		hashed_key_len = spprintf(&hashed_key, 0, "couchbase_%s_%s_%s_%s",
								  sorted_hosts, user, passwd, bucket);
		free(sorted_hosts);

		if (zend_hash_find(&EG(persistent_list), hashed_key,
						   hashed_key_len + 1, (void **) &le) == FAILURE) {
			goto create_new_link;
		}
		couchbase_res = le->ptr;
		couchbase_res->seqno = 0;
		couchbase_res->async = 0;
		couchbase_res->serializer = COUCHBASE_G(serializer_real);
		couchbase_res->compressor = COUCHBASE_G(compressor_real);
		couchbase_res->ignoreflags = 0;
		couchbase_res->rc = LCB_SUCCESS;
		efree(hashed_key);
	} else {
		struct lcb_cached_config_st cached;
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

		if (strlen(COUCHBASE_G(config_cache)) == 0) {
			if (lcb_create(&handle, &create_options) != LCB_SUCCESS) {
				free_connparams(&cparams);
				couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
									   cb_lcb_exception,
									   "Failed to create libcouchbase instance");
				return;
			}

            if (COUCHBASE_G(skip_config_errors_on_connect)) {
                int enable = 1;
                if (lcb_cntl(handle, LCB_CNTL_SET,
                             LCB_CNTL_SKIP_CONFIGURATION_ERRORS_ON_CONNECT,
                             &enable) != LCB_SUCCESS) {
                    free_connparams(&cparams);
                    lcb_destroy(handle);
                    couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
                                           cb_lcb_exception,
                                           "Failed to enable skip_config_errors_on_connect");
                }
            }
		} else {
			lcb_error_t err;
			char cachefile[1024];

			if (COUCHBASE_G(config_cache_error) != NULL) {
				if (try_setup_cache_dir(COUCHBASE_G(config_cache),
										&COUCHBASE_G(config_cache_error)) != 0) {
					free_connparams(&cparams);
					couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
										   cb_exception,
										   COUCHBASE_G(config_cache_error));
				}
			}

			memset(&cached, 0, sizeof(cached));
			memcpy(&cached.createopt, &create_options, sizeof(create_options));
			cached.cachefile = cachefile;
			snprintf(cachefile, sizeof(cachefile), "%s/%s.cache",
					 COUCHBASE_G(config_cache),
					 create_options.v.v0.bucket);

			err = lcb_create_compat(LCB_CACHED_CONFIG, &cached,
									&handle, NULL);
			if (err != LCB_SUCCESS) {
				free_connparams(&cparams);
				if (err == LCB_NOT_SUPPORTED) {
					couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
										   cb_not_supported_exception,
										   "Configuration cache is not supported in the installed version of libcouchbase");
				} else {
					couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
										   cb_lcb_exception,
										   "Failed to create libcouchbase instance");
				}
				return;
			}
		}

		php_couchbase_setup_callbacks(handle);

		couchbase_res = calloc(1, sizeof(php_couchbase_res));
		couchbase_res->handle = handle;
		if (cparams.bucket) {
			couchbase_res->bucket = strdup(cparams.bucket);
		} else {
			couchbase_res->bucket = strdup("default");
		}
		couchbase_res->seqno = -1; /* tell error callback stop event loop when error occurred */
		couchbase_res->async = 0;
		couchbase_res->serializer = COUCHBASE_G(serializer_real);
		couchbase_res->compressor = COUCHBASE_G(compressor_real);
		couchbase_res->ignoreflags = 0;

		lcb_set_cookie(handle, couchbase_res);

		lcb_behavior_set_syncmode(handle, LCB_SYNCHRONOUS);
		if ((retval = lcb_connect(handle)) != LCB_SUCCESS) {
			if (couchbase_res->errinfo != NULL) {
				couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
									   cb_lcb_exception,
									   "Failed to connect libcouchbase to the server: %s (%s)",
									   lcb_strerror(handle, retval), couchbase_res->errinfo);
			} else {
				couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
									   cb_lcb_exception,
									   "Failed to connect libcouchbase to the server: %s",
									   lcb_strerror(handle, retval));
			}
			lcb_destroy(handle);
			free(couchbase_res->bucket);
			free(couchbase_res);
			free_connparams(&cparams);

			return ;
		}

		lcb_behavior_set_syncmode(handle, LCB_ASYNCHRONOUS);
		couchbase_res->seqno = 0;
		couchbase_res->is_connected = 1;
		if (persistent) {
			zend_rsrc_list_entry le;
			Z_TYPE(le) = le_pcouchbase;
			le.ptr = couchbase_res;
			if (zend_hash_update(&EG(persistent_list), hashed_key,
								 hashed_key_len + 1, (void *) &le,
								 sizeof(zend_rsrc_list_entry),
								 NULL) == FAILURE) {
				free_connparams(&cparams);
				couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
									   cb_exception,
									   "Failed to store persistent link");
			}
			efree(hashed_key);
		}
	}

	ZEND_REGISTER_RESOURCE(return_value, couchbase_res,
						   persistent ? le_pcouchbase : le_couchbase);
	if (oo) {
		zval *self = getThis();
		zend_update_property(couchbase_ce, self,
							 ZEND_STRL(COUCHBASE_PROPERTY_HANDLE),
							 return_value TSRMLS_CC);
	} else if (!couchbase_res->is_connected) { /* !oo && !connected */
		free_connparams(&cparams);
		RETURN_FALSE;
	}

	free_connparams(&cparams);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
