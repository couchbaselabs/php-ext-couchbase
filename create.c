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
void php_couchbase_create_impl(INTERNAL_FUNCTION_PARAMETERS, int oo) /* {{{ */
{
	char *user = NULL, *passwd = NULL, *bucket = NULL;
	int user_len = 0, passwd_len = 0, bucket_len = 0;
	zend_bool persistent = 0;
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
		if (!parse_host(
					Z_STRVAL_P(zvhosts), Z_STRLEN_P(zvhosts), &cparams TSRMLS_CC)) {
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

				php_error_docref(NULL TSRMLS_CC, E_WARNING,
								 "Couldn't get string from node lists");
				free_connparams(&cparams);
				RETURN_FALSE;
			}
			if (!parse_host(Z_STRVAL_PP(curzv),
							Z_STRLEN_PP(curzv),
							&cparams TSRMLS_CC)) {
				free_connparams(&cparams);
				RETURN_FALSE;
			}
		}

	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
						 "Hosts is neither a string nor an array");
		free_connparams(&cparams);
		RETURN_FALSE;
	}

	make_params(&cparams);


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
			free_connparams(&cparams);
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to create libcouchbase instance");
			RETURN_FALSE;
		}


		retval = lcb_connect(handle);

		if (LCB_SUCCESS != retval) {
			php_error(E_WARNING, "Failed to connect libcouchbase to the server: %s", lcb_strerror(handle, retval));
		}

		php_couchbase_setup_callbacks(handle);

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
				free_connparams(&cparams);
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
		free_connparams(&cparams);
		RETURN_FALSE;
	}

	free_connparams(&cparams);
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
