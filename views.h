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

#ifndef PCBC_VIEWS_H_
#define PCBC_VIEWS_H_

typedef struct {
	char *str;
	size_t len;
	int allocated;
} pcbc_sso_buf;

typedef struct view_param_st view_param;

typedef int (*view_param_handler)(view_param* param,
		zval* input, pcbc_sso_buf *output, char **error TSRMLS_DC);


struct view_param_st {
	const char *param;
	view_param_handler handler;
};


PHP_COUCHBASE_LOCAL
void pcbc_sso_buf_cleanup(pcbc_sso_buf *buf);

PHP_COUCHBASE_LOCAL
view_param *pcbc_find_view_param(const char *vopt, size_t nvopt);

PHP_COUCHBASE_LOCAL
int pcbc_vopt_generic_param_handler(view_param *param,
		zval *input, pcbc_sso_buf *output, char **error TSRMLS_DC);

#endif /* PCBC_VIEWS_H_ */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
