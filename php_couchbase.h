/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright 2011 Couchbase, Inc.                                       |
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
  | Author: Xinchen Hui    <laruence@php.net>                            |
  +----------------------------------------------------------------------+
*/

/* $Id */

#ifndef PHP_COUCHBASE_H
#define PHP_COUCHBASE_H

extern zend_module_entry couchbase_module_entry;
#define phpext_couchbase_ptr &couchbase_module_entry

#ifdef PHP_WIN32
#    define PHP_COUCHBASE_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#    define PHP_COUCHBASE_API __attribute__ ((visibility("default")))
#else
#    define PHP_COUCHBASE_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#ifdef ZTS
#define COUCHBASE_G(v) TSRMG(couchbase_globals_id, zend_couchbase_globals *, v)
#else
#define COUCHBASE_G(v) (couchbase_globals.v)
#endif

#ifndef php_ignore_value
#define php_ignore_value(x) ((void) (x))
#endif

#if (PHP_MAJOR_VERSION == 5) && (PHP_MINFO_FUNCTION > 2)
#define COUCHBASE_ARG_PREFIX
#else
#define COUCHBASE_ARG_PREFIX static
#endif

#define PHP_COUCHBASE_VERSION     "0.0.1"
#define PHP_COUCHBASE_RESOURCE    "Couchbase"

enum memcached_serializer {
    SERIALIZER_PHP = 1,
    SERIALIZER_IGBINARY = 2,
    SERIALIZER_JSON = 3,
    SERIALIZER_JSON_ARRAY = 4,
};

#define COUCHBASE_SERIALIZER_PHP            0
#define COUCHBASE_SERIALIZER_JSON           1
#define COUCHBASE_SERIALIZER_JSON_ARRAY     2
#define COUCHBASE_SERIALIZER_DEFAULT        SERIALIZER_PHP
#define COUCHBASE_SERIALIZER_DEFAULT_NAME   "php"

#define COMPRESSION_TYPE_FASTLZ 0
#define COMPRESSION_TYPE_ZLIB   1

typedef struct _php_couchbase_res {
    libcouchbase_t handle;
    libcouchbase_io_opt_t *io;
    long seqno;
    unsigned char serializer;
    unsigned char compression_type;
    libcouchbase_error_t rc;
} php_couchbase_res;

typedef struct _php_couchbase_ctx {
    zval *rv;
    zval *cas;
    php_couchbase_res *res;
    void *extended_value;
} php_couchbase_ctx;

ZEND_BEGIN_MODULE_GLOBALS(couchbase)
    unsigned char serializer;
    unsigned char compression_type;
ZEND_END_MODULE_GLOBALS(couchbase)

PHP_GINIT_FUNCTION(couchbase);
PHP_MINIT_FUNCTION(couchbase);
PHP_MSHUTDOWN_FUNCTION(couchbase);
PHP_RINIT_FUNCTION(couchbase);
PHP_RSHUTDOWN_FUNCTION(couchbase);
PHP_MINFO_FUNCTION(couchbase);

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
PHP_FUNCTION(couchbase_delete);
PHP_FUNCTION(couchbase_get_stats);
PHP_FUNCTION(couchbase_flush);
PHP_FUNCTION(couchbase_increment);
PHP_FUNCTION(couchbase_decrement);
PHP_FUNCTION(couchbase_get_result_code);
PHP_FUNCTION(couchbase_version);

#endif    /* PHP_COUCHBASE_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet expandtab sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
