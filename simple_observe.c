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
  | Author: Trond Norbye	   <trond.norbye@gmail.org>					 |
  +----------------------------------------------------------------------+
*/
#include "internal.h"

struct observe_cookie {
	struct observe_entry *entries;
	int num;
	lcb_error_t error;
};

static void simple_observe_callback(lcb_t instance,
									const void *cookie,
									lcb_error_t error,
									const lcb_observe_resp_t *resp)
{
	struct observe_cookie *c = (struct observe_cookie *)cookie;
	struct observe_entry *ent = NULL;
	int ii;

	if (resp->version != 0) {
		c->error = LCB_EINVAL;
	}

	if (c->error != LCB_SUCCESS) {
		/*
		 * if we've gotten any incorrect version responses we'll just
		 * ditch all of them
		 */
		return ;
	}

	/* Is this the terminating packet? */
	if (resp->v.v0.nkey == 0) {
		return;
	}

	/* let's find the correct entry */
	for (ii = 0; ii < c->num; ++ii) {
		if ((c->entries[ii].nkey == resp->v.v0.nkey) &&
				memcmp(c->entries[ii].key, resp->v.v0.key, resp->v.v0.nkey) == 0) {
			ent = &c->entries[ii];
		}
	}

	if (ent == NULL) {
		/* This shouldn't happen.. the server sent a key we didn't request! */
		c->error = LCB_ERROR;
		return;
	}

	ent->err = error;
	if (error == LCB_SUCCESS) {
		if (ent->cas != resp->v.v0.cas) {
			/*
			 *It's only mutated if it is from the master (if not it may
			 * be data from the previous version of the key
			 */
			if (resp->v.v0.from_master) {
				ent->mutated = 1;
			}
		} else {
			/* This is the correct object */
			if (resp->v.v0.from_master == 0) {
				if ((resp->v.v0.status & LCB_OBSERVE_NOT_FOUND) == 0) {
					ent->replicated++;
				}
			}

			if (resp->v.v0.status & LCB_OBSERVE_PERSISTED) {
				ent->persisted++;
			}
		}
	}
}

static int should_add(struct observe_entry *entries,
					  long persist_to,
					  long replicate_to)
{
	int ret = 0;
	if (entries->mutated == 0) {
		if (entries->persisted < persist_to) {
			ret++;
		}
		if (entries->replicated < replicate_to) {
			ret++;
		}

		if (ret) {
			/* reset the counters */
			entries->persisted = 0;
			entries->replicated = 0;
		}
	}

	return ret;
}

PHP_COUCHBASE_LOCAL
lcb_error_t simple_observe(lcb_t instance,
						   struct observe_entry *entries,
						   int nentries,
						   long persist_to,
						   long replicate_to)
{
	lcb_error_t err;

	lcb_observe_cmd_t *cmds = ecalloc(nentries, sizeof(lcb_observe_cmd_t));
	lcb_observe_cmd_t **commands = ecalloc(nentries, sizeof(lcb_observe_cmd_t *));
	int ii;
	lcb_observe_callback org = lcb_set_observe_callback(instance,
														simple_observe_callback);
	struct observe_cookie cookie;
	int xx;
	int done = 0;
	int numtries = 0;
	int interval = INI_INT(PCBC_INIENT_OBS_INTERVAL);
	int maxretry = INI_INT(PCBC_INIENT_OBS_TIMEOUT) / interval;


	cookie.entries = entries;
	cookie.num = nentries;
	cookie.error = LCB_SUCCESS;

	lcb_behavior_set_syncmode(instance, LCB_SYNCHRONOUS);

	do {
		/* set up the commands */
		for (xx = 0, ii = 0; ii < nentries; ++ii) {
			cmds[ii].v.v0.key = entries[ii].key;
			cmds[ii].v.v0.nkey = entries[ii].nkey;
			if (should_add(&entries[ii], persist_to, replicate_to)) {
				commands[xx++] = cmds + ii;
			}
		}

		if (xx > 0) {
			if (numtries > 0) {
				usleep(interval);
			}
			++numtries;
			err = lcb_observe(instance, &cookie, xx,
							  (const lcb_observe_cmd_t * const *)commands);
		} else {
			done = 1;
		}
	} while (!done && numtries < maxretry);

	efree(cmds);
	efree(commands);

	lcb_behavior_set_syncmode(instance, LCB_ASYNCHRONOUS);
	lcb_set_observe_callback(instance, org);

	if (!done) {
		return LCB_ETIMEDOUT;
	}

	return (err == LCB_SUCCESS) ? cookie.error : err;
}

PHP_COUCHBASE_LOCAL
int validate_simple_observe_clause(lcb_t instance,
								   int persist,
								   int replicas TSRMLS_DC)
{
	int num_replicas = lcb_get_num_replicas(instance);
	int num_nodes = lcb_get_num_nodes(instance);

	if ((persist > (num_replicas + 1)) || (replicas > num_replicas)) {
		zend_throw_exception(cb_not_enough_nodes_exception,
							 "Not enough replicas to fulfill the request",
							 0 TSRMLS_CC);
		return -1;
	}

	if ((persist > num_nodes) || ((replicas + 1) > num_nodes)) {
		zend_throw_exception(cb_not_enough_nodes_exception,
							 "Not enough nodes to fulfill the request",
							 0 TSRMLS_CC);
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
