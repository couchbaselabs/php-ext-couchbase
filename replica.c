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

struct entry {
	lcb_error_t error;
	lcb_get_resp_t data;
	struct entry *next;
};

struct response {
	lcb_error_t error;
	int entries;
	struct entry *data;
};

static struct entry *clone(lcb_error_t error, const lcb_get_resp_t *r) {
	struct entry *ret = calloc(1, sizeof(struct entry));
	if (ret != NULL) {
		ret->error = error;
		ret->data = *r;
		ret->data.v.v0.key = malloc(ret->data.v.v0.nkey);
		ret->data.v.v0.bytes = malloc(ret->data.v.v0.nbytes);
		if (ret->data.v.v0.key == NULL || ret->data.v.v0.bytes == NULL) {
			free((void *)ret->data.v.v0.key);
			free((void *)ret->data.v.v0.bytes);
			free(ret);
			return NULL;
		}
		memcpy((void *)ret->data.v.v0.key, r->v.v0.key, ret->data.v.v0.nkey);
		memcpy((void *)ret->data.v.v0.bytes, r->v.v0.bytes, ret->data.v.v0.nbytes);
	}

	return ret;
}

static void get_replica_callback(lcb_t instance,
								 const void *cookie,
								 lcb_error_t error,
								 const lcb_get_resp_t *resp)
{
	struct response *r = (void *)cookie;
	struct entry *c = clone(error, resp);
	if (c) {
		r->entries++;
		c->next = r->data;
		r->data = c;
	}

	if (error != LCB_SUCCESS && error != LCB_KEY_ENOENT) {
		r->error = error;
	}
}

static lcb_get_replica_cmd_t *create_cmd(zval *k,
										 void **alloc,
										 const void *prefix,
										 size_t nprefix,
										 lcb_replica_t strategy,
										 int replica_index)
{
	lcb_get_replica_cmd_t *cmd = ecalloc(1, sizeof(lcb_get_replica_cmd_t));
	cmd->version = 1;
	cmd->v.v1.strategy = strategy;
	cmd->v.v1.index = replica_index;

	if (prefix == NULL && Z_TYPE_P(k) == IS_STRING) {
		/* Use as is */
		cmd->v.v1.key = Z_STRVAL_P(k);
		cmd->v.v1.nkey = Z_STRLEN_P(k);
	} else {
		/* We need to reallocate */
		char *buffer;

		if (Z_TYPE_P(k) == IS_STRING) {
			buffer = ecalloc(nprefix + Z_STRLEN_P(k) + 2, sizeof(char));
			memcpy(buffer, prefix, nprefix);
			buffer[nprefix] = '_';
			memcpy(buffer + nprefix + 1, Z_STRVAL_P(k), Z_STRLEN_P(k));
		} else {
			buffer = ecalloc(nprefix + 15, sizeof(char));
			if (prefix) {
				memcpy(buffer, prefix, nprefix);
				buffer[nprefix] = '_';
				sprintf(buffer + nprefix + 1, "%lu", k->value.lval);
			} else {
				sprintf(buffer, "%lu", k->value.lval);
			}

		}
		*alloc = buffer;
		cmd->v.v1.key = buffer;
		cmd->v.v1.nkey = strlen(buffer);
	}

	return cmd;
}

static zval *entry2array(INTERNAL_FUNCTION_PARAMETERS, struct entry *e, php_couchbase_res *res)
{
	zval *r;
	MAKE_STD_ZVAL(r);
	array_init(r);

	if (e->error == LCB_SUCCESS) {
		char cas[30];
		sprintf(cas, "%"PRIu64, e->data.v.v0.cas);
		add_assoc_string(r, "cas", cas, 1);

		if (e->data.v.v0.nbytes > 0) {
			zval *v;
			MAKE_STD_ZVAL(v);
			if (!php_couchbase_zval_from_payload(v,
												 (void *)e->data.v.v0.bytes,
												 e->data.v.v0.nbytes,
												 e->data.v.v0.flags,
												 res->serializer,
												 res->ignoreflags
												 TSRMLS_CC)) {
				add_assoc_string(r, "error", "failed to decode value", 1);
			} else {
				add_assoc_zval_ex(r, "value", 6, v);
			}
		}
	} else {
		add_assoc_string(r, "error", (void *)lcb_strerror(NULL, e->error), 1);
		add_assoc_long(r, "errorcode", (long)e->error);
	}
	return r;
}

PHP_COUCHBASE_LOCAL
void php_couchbase_get_replica_impl(INTERNAL_FUNCTION_PARAMETERS)
{
	zval *ids = NULL;
	zval *strategy_spec = NULL;
	lcb_replica_t strategy = LCB_REPLICA_FIRST;
	int replica_index = -1;
	int num_docs;
	int ii;
	void **key_allocs = NULL;
	php_couchbase_res *cb_res;
	lcb_t instance;
	lcb_error_t retval;
	lcb_get_replica_cmd_t **commands;
	lcb_get_callback old;
	struct response cookie;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z",
							  &ids, &strategy_spec) != SUCCESS) {
		return ;
	}

	PHP_COUCHBASE_GET_RESOURCE(cb_res, getThis(), PHP_COUCHBASE_ARG_F_OO);
	instance = cb_res->handle;

	if (strategy_spec != NULL) {
		if (Z_TYPE_P(strategy_spec) == IS_STRING) {
			if (Z_STRLEN_P(strategy_spec) == 21 &&
					memcmp(Z_STRVAL_P(strategy_spec), "COUCHBASE_REPLICA_ALL", 21) == 0) {
				strategy = LCB_REPLICA_ALL;
			} else if (Z_STRLEN_P(strategy_spec) == 23 &&
					   memcmp(Z_STRVAL_P(strategy_spec), "COUCHBASE_REPLICA_FIRST", 23) == 0) {
				strategy = LCB_REPLICA_FIRST;
			} else if (Z_STRLEN_P(strategy_spec) == 24 &&
					   memcmp(Z_STRVAL_P(strategy_spec), "COUCHBASE_REPLICA_SELECT", 24) == 0) {
				couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1,
									   cb_illegal_arguments_exception,
									   "Invalid value specified as strategy. No replica index specified");
				return;
			} else {
				couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1,
									   cb_illegal_arguments_exception,
									   "Invalid value specified as strategy.");
			}
		} else if (Z_TYPE_P(strategy_spec) == IS_LONG || Z_TYPE_P(strategy_spec) == IS_CONSTANT) {
			strategy = strategy_spec->value.lval;
			switch (strategy) {
			case LCB_REPLICA_FIRST:
			case LCB_REPLICA_ALL:
				break;
			case LCB_REPLICA_SELECT:
				couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1,
									   cb_illegal_arguments_exception,
									   "Invalid value specified as strategy. No replica index specified");
				return;

			default:
				couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1,
									   cb_illegal_arguments_exception,
									   "Invalid value specified as strategy");
				return;
			}
		} else if (Z_TYPE_P(strategy_spec) == IS_ARRAY) {
			HashTable *spec = Z_ARRVAL_P(strategy_spec);
			HashPosition position;
			zval **data;

			for (zend_hash_internal_pointer_reset_ex(spec, &position);
					zend_hash_get_current_data_ex(spec, (void **)&data, &position) == SUCCESS;
					zend_hash_move_forward_ex(spec, &position)) {

				char *key = NULL;
				uint klen = 0;
				ulong index;
				int type = zend_hash_get_current_key_ex(spec, &key, &klen,
														&index, 0, &position);
				if (type != HASH_KEY_IS_STRING) {
					couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1,
										   cb_illegal_arguments_exception,
										   "Invalid value specified as strategy./ Use: array(\"strategy\" => \"select\", \"index\" => 0)");
					return;
				}

				if (klen == 9 && memcmp(key, "strategy", klen) == 0) {
					if (Z_TYPE_PP(data) != IS_STRING) {
						couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1,
											   cb_illegal_arguments_exception,
											   "Strategy must be specified as a string./ Use: \"first\", \"all\" or \"select\")");
						return;
					}

					if (Z_STRLEN_PP(data) == 5 && memcmp(Z_STRVAL_PP(data),
														 "first", 5) == 0) {
						strategy = LCB_REPLICA_FIRST;
					} else if (Z_STRLEN_PP(data) == 3 && memcmp(Z_STRVAL_PP(data),
																"all", 3) == 0) {
						strategy = LCB_REPLICA_ALL;
					} else if (Z_STRLEN_PP(data) == 6 && memcmp(Z_STRVAL_PP(data),
																"select", 6) == 0) {
						strategy = LCB_REPLICA_SELECT;
					} else {
						couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1,
											   cb_illegal_arguments_exception,
											   "Unknown strategy specified. Use: \"first\", \"all\" or \"select\")");
						return;
					}
				} else if (klen == 6 && memcmp(key, "index", klen) == 0) {
					if (Z_TYPE_PP(data) != IS_LONG) {
						couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1,
											   cb_illegal_arguments_exception,
											   "Index must be specified as a number.");
						return;
					}

					replica_index = (int)(*data)->value.lval;
					if (replica_index < 0) {
						couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1,
											   cb_illegal_arguments_exception,
											   "Index must be specified as a positive number.");
						return;
					}
				} else {
					couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1,
										   cb_illegal_arguments_exception,
										   "Invalid option specified as strategy./ Use: array(\"strategy\" => \"select\", \"index\" => 0)");
					return;
				}
			}
		} else {
			couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1,
								   cb_illegal_arguments_exception,
								   "Invalid value specified as strategy");
			return;
		}
	}

	/* Done parsing the strategy! */
	if (Z_TYPE_P(ids) == IS_STRING || Z_TYPE_P(ids) == IS_LONG) {
		num_docs = 1;
	} else if (Z_TYPE_P(ids) == IS_ARRAY) {
		num_docs = zend_hash_num_elements(Z_ARRVAL_P(ids));
	} else {
		couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1,
							   cb_illegal_arguments_exception,
							   "Invalid datatype for document ids specified");
		return ;
	}

	commands = ecalloc(num_docs, sizeof(lcb_get_replica_cmd_t *));
	if (commands == NULL) {
		couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1,
							   cb_exception,
							   "Out of memory...");
		return ;
	}

	key_allocs = ecalloc(num_docs, sizeof(void *));
	if (key_allocs == NULL) {
		efree(commands);
		couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1,
							   cb_exception,
							   "Out of memory...");
		return ;
	}

	if (Z_TYPE_P(ids) == IS_ARRAY) {
		HashTable *spec = Z_ARRVAL_P(ids);
		HashPosition position;
		zval **data;
		int ii = 0;

		for (zend_hash_internal_pointer_reset_ex(spec, &position);
				zend_hash_get_current_data_ex(spec, (void **)&data, &position) == SUCCESS;
				zend_hash_move_forward_ex(spec, &position), ii++) {

			if (Z_TYPE_PP(data) == IS_STRING || Z_TYPE_PP(data) == IS_LONG) {
				commands[ii] = create_cmd(*data, &key_allocs[ii],
										  cb_res->prefix_key, cb_res->prefix_key_len,
										  strategy, replica_index);
			} else {
				couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1,
									   cb_illegal_arguments_exception,
									   "Invalid datatype for document ids specified");

				/* Time to release memory! */
				while (ii > 0) {
					--ii;
					efree(commands[ii]);
					efree(key_allocs[ii]);

				}
				efree(commands);
				efree(key_allocs);

				return ;
			}
		}
	} else {
		commands[0] = create_cmd(ids, &key_allocs[0],
								 cb_res->prefix_key, cb_res->prefix_key_len,
								 strategy, replica_index);
	}

	old = lcb_set_get_callback(instance, get_replica_callback);
	cookie.error = LCB_SUCCESS;
	cookie.entries = 0;
	cookie.data = NULL;

	lcb_behavior_set_syncmode(instance, LCB_SYNCHRONOUS);
	retval = lcb_get_replica(instance, &cookie, num_docs,
							 (const lcb_get_replica_cmd_t * const *)commands);
	lcb_behavior_set_syncmode(instance, LCB_ASYNCHRONOUS);
	lcb_set_get_callback(instance, old);

	if (retval == LCB_SUCCESS) {
		retval = cookie.error;
	}

	cb_res->rc = retval;
	if (retval == LCB_SUCCESS) {
		array_init(return_value);

		/* Time to build up the result array */
		if (strategy == LCB_REPLICA_ALL) {
			/* we may expect multipe values per key */
			struct entry *e = cookie.data;
			while (cookie.data) {
				e = cookie.data;
				cookie.data = e->next;
				if (e->data.v.v0.nkey > 0) {
					/* This object hasn't been procecced yet */
					zval *bl;
					char *k = (char *)e->data.v.v0.key;
					int nk = e->data.v.v0.nkey;
					struct entry *curr;
					char idx[10];
					int ii = 0;
					int len;

					if (cb_res->prefix_key) {
						k += cb_res->prefix_key_len + 1;
						nk -= cb_res->prefix_key_len - 1;
					}
					++nk;

					MAKE_STD_ZVAL(bl);
					array_init(bl);

					len = sprintf(idx, "%u", ii++);
					add_assoc_zval_ex(bl, idx, len + 1, entry2array(INTERNAL_FUNCTION_PARAM_PASSTHRU, e, cb_res));

					/* Do we have more entries? */
					for (curr = cookie.data; curr; curr = curr->next) {
						if (e->data.v.v0.nkey == curr->data.v.v0.nkey &&
								memcmp(e->data.v.v0.key, curr->data.v.v0.key,
									   e->data.v.v0.nkey) == 0) {
							len = sprintf(idx, "%u", ii++);
							add_assoc_zval_ex(bl, idx, len + 1,
											  entry2array(INTERNAL_FUNCTION_PARAM_PASSTHRU, curr, cb_res));
							curr->data.v.v0.nkey = 0;
						}
					}
					add_assoc_zval_ex(return_value, k, nk, bl);

				}
				free((void *)e->data.v.v0.key);
				free((void *)e->data.v.v0.bytes);
				free(e);
			}

		} else {
			/* We have a single value per key */
			struct entry *e = cookie.data;
			while (cookie.data) {
				char *k;
				int nk;
				zval *r;

				e = cookie.data;
				cookie.data = e->next;
				k = (char *)e->data.v.v0.key;
				nk = e->data.v.v0.nkey;

				if (cb_res->prefix_key) {
					k += cb_res->prefix_key_len + 1;
					nk -= cb_res->prefix_key_len - 1;
				}
				++nk;

				r = entry2array(INTERNAL_FUNCTION_PARAM_PASSTHRU, e, cb_res);
				add_assoc_zval_ex(return_value, k, nk, r);

				free((void *)e->data.v.v0.key);
				free((void *)e->data.v.v0.bytes);
				free(e);
			}
		}
	} else {
		couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1,
							   cb_lcb_exception,
							   "Failed to get replicas: %s",
							   lcb_strerror(instance, retval));
	}

	/* Time to release memory! */
	for (ii = 0; ii < num_docs; ++ii) {
		efree(commands[ii]);
		efree(key_allocs[ii]);
	}
	efree(commands);
	efree(key_allocs);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
