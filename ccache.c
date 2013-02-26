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
  | Author: Trond Norbye	   <trond.norbye@couchbase.com>				 |
  +----------------------------------------------------------------------+
*/
#include "internal.h"
#include <errno.h>

static int is_working(const char *path)
{
	char buffer[1024];
	FILE *fp;
	snprintf(buffer, sizeof(buffer), "%s/phpcache.test", path);
	fp = fopen(buffer, "w");
	if (fp != NULL) {
		fclose(fp);
		remove(buffer);
		return 1;
	}

	return 0;
}

int try_setup_cache_dir(const char *path, char **emsg)
{
	free(*emsg);
	*emsg = NULL;

#ifdef _WIN32
	if (!CreateDirectory(path, NULL)) {
		DWORD err = GetLastError();
		if (err != ERROR_ALREADY_EXISTS) {
			char errmsg[1024];
			LPVOID message;
			if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
							  FORMAT_MESSAGE_FROM_SYSTEM |
							  FORMAT_MESSAGE_IGNORE_INSERTS,
							  NULL, err, 0, (LPTSTR)&message,
							  0, NULL) != 0) {

				snprintf(errmsg, sizeof(errmsg),
						 "Failed to create cache directory \"%s\": \"%s\"."
						 "\nFix this and try again"
						 " extension", path, message);
				LocalFree(message);
			} else {
				snprintf(errmsg, sizeof(errmsg),
						 "Failed to create cache directory \"%s\".\nFix "
						 "this and try again",
						 path, strerror(errno));
			}
			*emsg = strdup(errmsg);
			return -1;
		}
	}

#else
	if (mkdir(path, S_IRWXU | S_IRGRP | S_IXGRP) == -1) {
		if (errno != EEXIST) {
			char errmsg[1024];
			snprintf(errmsg, sizeof(errmsg),
					 "Failed to create cache directory \"%s\": \"%s\".\nFix "
					 "this and try again",
					 path, strerror(errno));
			*emsg = strdup(errmsg);
			return -1;
		}
	}
#endif

	if (is_working(path) == 0) {
		char errmsg[1024];
		snprintf(errmsg, sizeof(errmsg),
				 "Missing write permission to \"%s\".\nFix "
				 "this and try to reload the Couchbase PHP extension",
				 path);
		*emsg = strdup(errmsg);
		return -1;
	}

	return 0;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
