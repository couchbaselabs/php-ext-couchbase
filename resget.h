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
  | Author: Mark Nunberg	   <mnunberg@haskalah.org>					 |
  +----------------------------------------------------------------------+
*/

#ifndef PHP_COUCHBASE_H
#error "Include php_couchbase.h first"
#endif

#ifndef PCBC_RESGET_H_
#define PCBC_RESGET_H_

#include <stdarg.h>

/**
 * Flags for various calling styles with PHP entry points
 */
enum {
	/* no-op, but nice to have */
	PHP_COUCHBASE_ARG_F_FUNCTIONAL,

	/* this is a *multi variant */
	PHP_COUCHBASE_ARG_F_MULTI = 1 << 0,

	/* this is an oo variant */
	PHP_COUCHBASE_ARG_F_OO = 1 << 1,

	/* Specify that we expect an active async operation */
	PHP_COUCHBASE_ARG_F_ASYNC = 1 << 2,

	/* Specify that it's ok to not be connected */
	PHP_COUCHBASE_ARG_F_NOCONN = 1 << 3
};

/** Only check that we have a valid pointer. Ignore any other checks */
#define PHP_COUCHBASE_ARG_F_ONLYVALID \
	(PHP_COUCHBASE_ARG_F_NOCONN|PHP_COUCHBASE_ARG_F_ASYNC)

enum {
	/* all ok */
	PHP_COUCHBASE_RES_OK,

	/* caller should return immediately */
	PHP_COUCHBASE_RES_ERETURN,

	/* resource is invalid */
	PHP_COUCHBASE_RES_EINVAL,

	/* resource is not yet connected */
	PHP_COUCHBASE_RES_ENOTCONN,

	/* async operations in progress */
	PHP_COUCHBASE_RES_EBUSY,
};

/**
 * Extract the resource from something.
 * @param zvres (For functional only) - The zval which claims to be our resource
 * @param argflags the argument/style flags
 * @param A pointer to an error code (if we get an error)
 * @param pres a pointer to a php_couchbase_res object. To be filled when we have
 * a handle
 */
PHP_COUCHBASE_LOCAL
void php_couchbase_get_resource(INTERNAL_FUNCTION_PARAMETERS,
		zval *zvres,
		int argflags,
		int *ec,
		php_couchbase_res **pres);

/**
 * Returns true on success, false on error
 */
PHP_COUCHBASE_LOCAL
int php_couchbase_res_ok(int ec TSRMLS_DC);


/**
 * Gets a resource from a zval (or a getThis()). This macro must be called
 * from within a zend entry point (or another function which has
 * INTERNAL_FUNCTION_PARAMETERS in its prototype
 *
 * If an error is encountered, this macro will return from the calling code
 * (i.e. the caller will return).
 *
 * @param cbres a pointer to a php_couchbase_res*. Will be populated
 * @param zvres a pointer to a zval possibly containing the resource
 * @param flags the argument flags for the style.
 */
#define PHP_COUCHBASE_GET_RESOURCE(cbres, zvres, flags) \
{ \
	int pcbc__ec = -1; \
	php_couchbase_get_resource(\
			INTERNAL_FUNCTION_PARAM_PASSTHRU, \
			zvres, \
			flags,\
			&pcbc__ec, \
			&(cbres)\
	); \
	\
	if (pcbc__ec == PHP_COUCHBASE_RES_ERETURN) { \
		return; \
	} else if (!php_couchbase_res_ok(pcbc__ec TSRMLS_CC)) { \
		RETURN_FALSE; \
	} \
}

/**
 * Extracts the parameters and gets us a zval.
 *
 * If an error occurs in parsing the arguments or in extracting the couchbase
 * resource, then the caller will return.
 *
 * @param zvres a zval* which will be assigned to contain the resource for the object
 * @param resvar a php_couchbase_res * which will point to the object itself
 * @param flags the argument flags
 * @param fmt the argument format string
 * @param ... extra pointers (dependent on format string)
 */
#define PHP_COUCHBASE_GET_PARAMS_WITH_ZV(zvres, resvar, flags, fmt, ...) \
	{ \
		if ( (flags) & PHP_COUCHBASE_ARG_F_OO) { \
			if (zend_parse_parameters(\
					ZEND_NUM_ARGS() TSRMLS_CC, fmt, \
					## __VA_ARGS__) == FAILURE) { \
				return; \
			} \
			\
			zvres = getThis(); \
			PHP_COUCHBASE_GET_RESOURCE(resvar, zvres, flags); \
			\
		} else { \
			if (zend_parse_parameters(\
					ZEND_NUM_ARGS() TSRMLS_CC, \
					"r" fmt, &(zvres), ## __VA_ARGS__) == FAILURE) { \
				return; \
			} \
			PHP_COUCHBASE_GET_RESOURCE(resvar, zvres, flags); \
		} \
	}

/**
 * Wrapper around PHP_COUCHBASE_GET_PARAMS_WITH_ZV. Does not require to pass
 * a zval* for a resource.
 */
#define PHP_COUCHBASE_GET_PARAMS(resvar, flags, fmt, ...) \
	{ \
		zval *pcbc__zv__res__dummy PHP_COUCHBASE_UNUSED; \
		PHP_COUCHBASE_GET_PARAMS_WITH_ZV(\
				pcbc__zv__res__dummy, \
				resvar, flags, fmt, ## __VA_ARGS__); \
	}

#endif /* PCBC_RESGET_H_ */
