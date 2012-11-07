#ifndef PHP_COUCHBASE_H
#error "Include php_couchbase.h first"
#endif

#ifndef PCBC_HT_H_
#define PCBC_HT_H_

typedef struct {
	const char *key;
	int key_type;
	unsigned long key_len;
	int _allocated;
} pcbc_ht_key;

typedef struct {
	pcbc_ht_key *key_info;
	zval *data;
} pcbc_ht_entry;


/**
 * Creation/Destruction
 */
PHP_COUCHBASE_LOCAL
void pcbc_ht_key_create(const char *key, int len, pcbc_ht_key *ki);

PHP_COUCHBASE_LOCAL
void pcbc_ht_key_cleanup(pcbc_ht_key *info);

PHP_COUCHBASE_LOCAL
void pcbc_ht_key_free(pcbc_ht_key *info);

PHP_COUCHBASE_LOCAL
void pcbc_ht_entry_free(pcbc_ht_entry *pair);

PHP_COUCHBASE_LOCAL
void pcbc_ht_dispose(zval *assoc);

/**
 * Iteration
 */
PHP_COUCHBASE_LOCAL
void pcbc_ht_iter_init(zval *assoc);

PHP_COUCHBASE_LOCAL
int pcbc_ht_iter_remaining(zval *assoc);

PHP_COUCHBASE_LOCAL
void pcbc_ht_iter_next(zval *assoc);

PHP_COUCHBASE_LOCAL
pcbc_ht_key *pcbc_ht_iter_key(zval *assoc);

PHP_COUCHBASE_LOCAL
zval *pcbc_ht_iter_value(zval *assoc);

PHP_COUCHBASE_LOCAL
pcbc_ht_entry *pcbc_ht_iter_entry(zval *assoc);

/**
 * Misc
 */

PHP_COUCHBASE_LOCAL
unsigned long pcbc_ht_len(zval *assoc);

/**
 * Key/Value access lookup.
 * hk* functions search by a hashkey structure, while the non-prefixed versions
 * take a string and length.
 *
 * Suffixes (for storage) represent the type of object to be stored
 */

PHP_COUCHBASE_LOCAL
void pcbc_ht_del(zval *assoc, const char *key, unsigned int key_len);

PHP_COUCHBASE_LOCAL
zval *pcbc_ht_hkfind(zval *assoc, pcbc_ht_key *hk);

PHP_COUCHBASE_LOCAL
zval *pcbc_ht_find(zval *assoc, const char *key, int key_len);

PHP_COUCHBASE_LOCAL
zval *pcbc_ht_ifind(zval *assoc, unsigned long idx);

PHP_COUCHBASE_LOCAL
int pcbc_ht_exists(zval *assoc, const char *key, int key_len);

PHP_COUCHBASE_LOCAL
int pcbc_ht_iexists(zval *assoc, unsigned long idx);

PHP_COUCHBASE_LOCAL
void pcbc_ht_hkstores(zval *assoc, pcbc_ht_key *hk,
                      const char *value, int nvalue);

PHP_COUCHBASE_LOCAL
void pcbc_ht_stores(zval *assoc,
                    const char *key, int nkey, const char *value, int nvalue);

PHP_COUCHBASE_LOCAL
void pcbc_ht_hkstoreb(zval *assoc, pcbc_ht_key *hk, zend_bool value);

PHP_COUCHBASE_LOCAL
void pcbc_ht_storeb(zval *assoc, const char *key, int nkey, zend_bool value);

PHP_COUCHBASE_LOCAL
void pcbc_ht_hkstorez(zval *assoc, pcbc_ht_key *hk, zval *value);

#endif
