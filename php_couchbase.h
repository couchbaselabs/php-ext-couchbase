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

/* $Id */

#ifndef PHP_COUCHBASE_H
#define PHP_COUCHBASE_H

extern zend_module_entry couchbase_module_entry;
#define phpext_couchbase_ptr &couchbase_module_entry
extern zend_class_entry *couchbase_ce;

#ifdef PHP_WIN32
#	 define PHP_COUCHBASE_API __declspec(dllexport)
#	 define strtoull _strtoui64
#elif defined(__GNUC__) && __GNUC__ >= 4
#	 define PHP_COUCHBASE_API __attribute__ ((visibility("default")))
#else
#	 define PHP_COUCHBASE_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#include <libcouchbase/couchbase.h>

#ifdef ZTS
#define COUCHBASE_G(v) TSRMG(couchbase_globals_id, zend_couchbase_globals *, v)
#else
#define COUCHBASE_G(v) (couchbase_globals.v)
#endif

#ifndef php_ignore_value
#define php_ignore_value(x) ((void) (x))
#endif

#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION > 2)
#define COUCHBASE_ARG_PREFIX
#else
#define COUCHBASE_ARG_PREFIX static
#endif

#define PHP_COUCHBASE_VERSION	  "1.0.0"
#define PHP_COUCHBASE_RESOURCE	  "Couchbase"
#define PHP_COUCHBASE_PERSISTENT_RESOURCE "Persistent Couchbase"
#define COUCHBASE_PROPERTY_HANDLE "_handle"

#define COUCHBASE_OPT_SERIALIZER			1
#define COUCHBASE_OPT_COMPRESSION			 2
#define COUCHBASE_OPT_PREFIX_KEY			3

#define COUCHBASE_SERIALIZER_PHP			0
#define COUCHBASE_SERIALIZER_DEFAULT		COUCHBASE_SERIALIZER_PHP
#define COUCHBASE_SERIALIZER_DEFAULT_NAME	"php"
#define COUCHBASE_SERIALIZER_JSON			1
#define COUCHBASE_SERIALIZER_JSON_ARRAY		2

#define COUCHBASE_IS_JSON					30 
#define COUCHBASE_IS_SERIALIZED				31
#define COUCHBASE_GET_TYPE(f)				((f) & 31)

#define COUCHBASE_COMPRESSION_MASK			 224 /* 7 << 5 */
#define COUCHBASE_COMPRESSION_NONE			 0
#define COUCHBASE_COMPRESSION_FASTLZ		 1
#define COUCHBASE_COMPRESSION_ZLIB			 2

#define COUCHBASE_GET_COMPRESSION(f)		 ((f) >> 5)
#define COUCHBASE_SET_COMPRESSION(f, c)		 ((f) = ((f) & ~COUCHBASE_COMPRESSION_MASK) | (c) << 5)

#ifndef Z_ADDREF_P 
#define Z_ADDREF_P	ZVAL_ADDREF
#endif


typedef struct _php_couchbase_res {
	libcouchbase_t handle;
	libcouchbase_io_opt_t *io;
	long seqno;
	char async;
	char serializer;
	char compressor;
	char *prefix_key;
	int prefix_key_len;
	libcouchbase_error_t rc; /* returned code */
} php_couchbase_res;

typedef struct _php_couchbase_ctx {
	zval *rv;
	zval *cas;
	php_couchbase_res *res;
	unsigned char flags;
	void *extended_value;
} php_couchbase_ctx;

ZEND_BEGIN_MODULE_GLOBALS(couchbase)
	char serializer_real;
	char *serializer;
	char compressor_real;
	char *compressor;
	long compression_threshold;
	double compression_factor;
ZEND_END_MODULE_GLOBALS(couchbase)

PHP_GINIT_FUNCTION(couchbase);
PHP_MINIT_FUNCTION(couchbase);
PHP_MSHUTDOWN_FUNCTION(couchbase);
PHP_RINIT_FUNCTION(couchbase);
PHP_RSHUTDOWN_FUNCTION(couchbase);
PHP_MINFO_FUNCTION(couchbase);

PHP_METHOD(couchbase, __construct);
PHP_METHOD(couchbase, add);
PHP_METHOD(couchbase, set);
PHP_METHOD(couchbase, setMulti);
PHP_METHOD(couchbase, replace);
PHP_METHOD(couchbase, prepend);
PHP_METHOD(couchbase, append);
PHP_METHOD(couchbase, cas);
PHP_METHOD(couchbase, get);
PHP_METHOD(couchbase, getMulti);
PHP_METHOD(couchbase, getDelayed);
PHP_METHOD(couchbase, fetch);
PHP_METHOD(couchbase, fetchAll);
PHP_METHOD(couchbase, delete);
PHP_METHOD(couchbase, getStats);
PHP_METHOD(couchbase, flush);
PHP_METHOD(couchbase, increment);
PHP_METHOD(couchbase, decrement);
PHP_METHOD(couchbase, getResultCode);
PHP_METHOD(couchbase, setOption);
PHP_METHOD(couchbase, getOption);
PHP_METHOD(couchbase, getVersion);

PHP_FUNCTION(couchbase_connect);
PHP_FUNCTION(couchbase_add);
PHP_FUNCTION(couchbase_set);
PHP_FUNCTION(couchbase_set_multi);
PHP_FUNCTION(couchbase_replace);
PHP_FUNCTION(couchbase_prepend);
PHP_FUNCTION(couchbase_append);
PHP_FUNCTION(couchbase_cas);
PHP_FUNCTION(couchbase_get);
PHP_FUNCTION(couchbase_get_multi);
PHP_FUNCTION(couchbase_get_delayed);
PHP_FUNCTION(couchbase_fetch);
PHP_FUNCTION(couchbase_fetch_all);
PHP_FUNCTION(couchbase_delete);
PHP_FUNCTION(couchbase_get_stats);
PHP_FUNCTION(couchbase_flush);
PHP_FUNCTION(couchbase_increment);
PHP_FUNCTION(couchbase_decrement);
PHP_FUNCTION(couchbase_get_result_code);
PHP_FUNCTION(couchbase_set_option);
PHP_FUNCTION(couchbase_get_option);
PHP_FUNCTION(couchbase_get_version);

#endif	  /* PHP_COUCHBASE_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet expandtab sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
