#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef PHP_WIN32
# include "win32/php_stdint.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/url.h"
#include "ext/standard/php_smart_str.h"
#include "ext/standard/php_var.h"
#ifdef HAVE_JSON_API
# include "ext/json/php_json.h"
#endif
#include "ext/standard/php_var.h"
#include <libcouchbase/couchbase.h>
#include "php_couchbase.h"
#ifdef HAVE_COMPRESSION_FASTLZ
# include "fastlz.c"
#endif
#ifdef HAVE_COMPRESSION_ZLIB
# include "zlib.h"
#endif


#include "Zend/zend_API.h"



void php_couchbase_get_servers_impl(INTERNAL_FUNCTION_PARAMETERS,
		int style)
{
	zval *zv_res;
	php_couchbase_res *res;
	const char * const * server_list = NULL;
	const char * const *cur_item;
	int wix = 0;
	if (style & PHP_COUCHBASE_ARG_F_OO) {
		PHP_COUCHBASE_GET_RESOURCE_OO(res);

	} else {
		if (zend_parse_parameters(
				ZEND_NUM_ARGS() TSRMLS_CC, "r", &zv_res) == FAILURE) {
			return;
		}
		PHP_COUCHBASE_GET_RESOURCE_FUNCTIONAL(res, zv_res);
	}

	server_list = lcb_get_server_list(res->handle);

	array_init(return_value);

	for (wix = 0, cur_item = server_list;
			*cur_item;
			cur_item++, wix++) {

		add_index_string(return_value, wix, *cur_item, 1);
	}
}


void php_couchbase_get_num_replicas_impl(INTERNAL_FUNCTION_PARAMETERS,
		int style)
{
	zval *zv_res;
	php_couchbase_res *res;

	if (style & PHP_COUCHBASE_ARG_F_OO) {
		PHP_COUCHBASE_GET_RESOURCE_OO(res);

	} else {
		if (zend_parse_parameters(
				ZEND_NUM_ARGS() TSRMLS_CC, "r", &zv_res) == FAILURE) {
			return;
		}

		PHP_COUCHBASE_GET_RESOURCE_FUNCTIONAL(res, zv_res);

	}

	ZVAL_LONG(return_value, lcb_get_num_replicas(res->handle));
}
