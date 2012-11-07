/* Various utility functions, e.g. wrappers around zend_hash_* functions that
 * are annoying to use in their native forms.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "internal.h"

/**
 * Checks that we're passed an array. If not, it's a fatal error as there's
 * likely something wrong in our code.
 */
#ifdef WIN32
#define __func__ __FUNCTION__
#endif

#define ISARRAY_SANITY(zv) \
	if (IS_ARRAY != Z_TYPE_P(zv)) { \
		php_error(E_ERROR, "%s given non-array zval in php "\
				"couchbase extension", __func__); \
	}

#define HK_STRING(hk, s, i) \
	s = (hk)->key; i = (hk)->key_len + 1;


PHP_COUCHBASE_LOCAL
void pcbc_ht_key_create(const char *key, int len, pcbc_ht_key *ki)
{
	memset(ki, 0, sizeof(*ki));
	assert(len);

	if (len == -1) {

		len = strlen(key);
		assert(len);

		ki->_allocated = 0;
		ki->key_len = len;
		ki->key = key;

		return;

	}

	ki->key = emalloc(len + 1);
	memcpy((char*)ki->key, key, len);
	((char*)ki->key)[len] = '\0';
	ki->key_len = len;
	ki->_allocated = 1;
}

PHP_COUCHBASE_LOCAL
void pcbc_ht_key_cleanup(pcbc_ht_key *info)
{
	if (info->key != NULL && info->_allocated) {
		efree((void*)info->key);
	}
}

PHP_COUCHBASE_LOCAL
void pcbc_ht_key_free(pcbc_ht_key *info)
{
	if (!info) {
		return;
	}
	pcbc_ht_key_cleanup(info);
	efree(info);
}

PHP_COUCHBASE_LOCAL
void pcbc_ht_entry_free(pcbc_ht_entry *pair)
{
	pcbc_ht_key_free(pair->key_info);
	efree(pair);
}

PHP_COUCHBASE_LOCAL
pcbc_ht_key* pcbc_ht_iter_key(zval *assoc)
{
	char *curr_key = NULL;
	unsigned int curr_key_len;
	int curr_key_type;
	unsigned long curr_idx;
	pcbc_ht_key *curr_key_info = NULL;
	ISARRAY_SANITY(assoc);

	curr_key_type = zend_hash_get_current_key_ex(Z_ARRVAL_P(assoc),
			(char**)&curr_key, &curr_key_len, &curr_idx, 1, NULL);
	
	switch(curr_key_type) {

		case HASH_KEY_IS_LONG:
			curr_key_len = curr_idx;
			break;

		case HASH_KEY_IS_STRING:
			/**
			 * zend gives us nul-terminated key strings,
			 * and for consistency's sake we don't want that
			 */
			curr_key_len--;
			break;
		default: /* something went badly wrong */
			return NULL;
	}

	curr_key_info = (pcbc_ht_key*)emalloc(sizeof(pcbc_ht_key));
	if (curr_key_info == NULL) {
		php_error(E_ERROR, "Unable to allocate memory for a key info struct!");
	}

	curr_key_info->key = curr_key;
	curr_key_info->key_type = curr_key_type;
	curr_key_info->key_len = curr_key_len;
	curr_key_info->_allocated = 1;

	return curr_key_info;
}

PHP_COUCHBASE_LOCAL
zval* pcbc_ht_iter_value(zval *assoc)
{
	zval **curr_data = NULL;
	ISARRAY_SANITY(assoc);
	zend_hash_get_current_data(Z_ARRVAL_P(assoc), (void**)&curr_data);
	return *curr_data;
}

PHP_COUCHBASE_LOCAL
pcbc_ht_entry* pcbc_ht_iter_entry(zval *assoc)
{
	pcbc_ht_entry *curr_entry;
	ISARRAY_SANITY(assoc);

	curr_entry = (pcbc_ht_entry*)emalloc(sizeof(pcbc_ht_entry));
	if (curr_entry == NULL) {
		php_error(E_ERROR,
				"Unable to allocate memory for a key-value pair struct!");
	}

	curr_entry->key_info = pcbc_ht_iter_key(assoc);
	curr_entry->data = pcbc_ht_iter_value(assoc);

	return curr_entry;
}

PHP_COUCHBASE_LOCAL
unsigned long pcbc_ht_len(zval *assoc)
{
	ISARRAY_SANITY(assoc);
	return zend_hash_num_elements(Z_ARRVAL_P(assoc));
}

PHP_COUCHBASE_LOCAL
void pcbc_ht_iter_init(zval *assoc)
{
	ISARRAY_SANITY(assoc);
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(assoc));
}

PHP_COUCHBASE_LOCAL
int pcbc_ht_iter_remaining(zval *assoc)
{
	ISARRAY_SANITY(assoc);
	return ((zend_hash_has_more_elements(Z_ARRVAL_P(assoc)) == SUCCESS)
			? 1 : 0);
}

PHP_COUCHBASE_LOCAL
void pcbc_ht_iter_next(zval *assoc)
{
	ISARRAY_SANITY(assoc);
	zend_hash_move_forward(Z_ARRVAL_P(assoc));
}

PHP_COUCHBASE_LOCAL
void pcbc_ht_del(zval *assoc, const char *key, unsigned int key_len)
{
	pcbc_ht_key hk = { NULL };
	ISARRAY_SANITY(assoc);
	pcbc_ht_key_create(key, key_len, &hk);

	zend_hash_del(Z_ARRVAL_P(assoc), hk.key, hk.key_len + 1);

	pcbc_ht_key_cleanup(&hk);
}

PHP_COUCHBASE_LOCAL
zval* pcbc_ht_hkfind(zval *assoc, pcbc_ht_key *hk)
{
	zval **ppdata = NULL;
	const char *key; int nkey;

	ISARRAY_SANITY(assoc);
	HK_STRING(hk, key, nkey);
	assert(nkey);

	if (zend_hash_find(Z_ARRVAL_P(assoc),
			key, nkey, (void**)&ppdata) != SUCCESS) {
		return NULL;
	}

	return *ppdata;
}

PHP_COUCHBASE_LOCAL
zval* pcbc_ht_find(zval *assoc, const char *key, int key_len)
{
	zval *ret;
	pcbc_ht_key hk = { NULL };
	pcbc_ht_key_create(key, key_len, &hk);
	ret = pcbc_ht_hkfind(assoc, &hk);
	pcbc_ht_key_cleanup(&hk);
	return ret;
}

PHP_COUCHBASE_LOCAL
zval* pcbc_ht_ifind(zval *assoc, unsigned long idx)
{
	zval *data;
	ISARRAY_SANITY(assoc);

	if (!zend_hash_index_exists(Z_ARRVAL_P(assoc), idx)) {
		return NULL;	
	} 

	if (zend_hash_index_find(Z_ARRVAL_P(assoc),
			idx, (void **)&data) != SUCCESS) {
		return NULL;	
	}

	return data;
}

PHP_COUCHBASE_LOCAL
int pcbc_ht_exists(zval *assoc, const char *key, int key_len)
{
	pcbc_ht_key hk = { NULL };
	const char *zh_key;
	int zh_nkey;
	int ret;

	ISARRAY_SANITY(assoc);


	pcbc_ht_key_create(key, key_len, &hk);

	HK_STRING(&hk, zh_key, zh_nkey);
	ret = zend_hash_exists(Z_ARRVAL_P(assoc), zh_key, zh_nkey);

	pcbc_ht_key_cleanup(&hk);

	return ret;
}

PHP_COUCHBASE_LOCAL
int pcbc_ht_iexists(zval *assoc, unsigned long idx)
{
	if (IS_ARRAY != Z_TYPE_P(assoc)) {
		php_error(E_RECOVERABLE_ERROR,
				"assoc_idx_exists given non-array zval, in couchbase php-ext");
	}

	return ((zend_hash_index_exists(Z_ARRVAL_P(assoc), idx)) ? 1 : 0);
}

PHP_COUCHBASE_LOCAL
void pcbc_ht_hkstores(zval *assoc, pcbc_ht_key *hk,
		const char *value, int nvalue)
{
	const char *zh_key;
	int zh_nkey;
	
	ISARRAY_SANITY(assoc);

	if (nvalue == -1) {
		nvalue = strlen(value);
	}

	HK_STRING(hk, zh_key, zh_nkey);

	/* value is duplicated, so it's sane to cast to char */
	add_assoc_stringl_ex(assoc, zh_key, zh_nkey, (char*)value, nvalue, 1);
}

PHP_COUCHBASE_LOCAL
void pcbc_ht_stores(zval *assoc,
		const char *key, int nkey, const char *value, int nvalue)
{
	pcbc_ht_key hk;
	pcbc_ht_key_create(key, nkey, &hk);
	pcbc_ht_hkstores(assoc, &hk, value, nvalue);
	pcbc_ht_key_cleanup(&hk);
}

PHP_COUCHBASE_LOCAL
void pcbc_ht_hkstoreb(zval *assoc, pcbc_ht_key *hk, zend_bool value)
{
	const char *zh_key;
	int zh_nkey;

	ISARRAY_SANITY(assoc);
	HK_STRING(hk, zh_key, zh_nkey);
	add_assoc_bool_ex(assoc, zh_key, zh_nkey, value);
}

PHP_COUCHBASE_LOCAL
void pcbc_ht_storeb(zval *assoc, const char *key, int nkey, zend_bool value)
{
	pcbc_ht_key hk;
	pcbc_ht_key_create(key, nkey, &hk);
	pcbc_ht_hkstoreb(assoc, &hk, value);
	pcbc_ht_key_cleanup(&hk);
}

PHP_COUCHBASE_LOCAL
void pcbc_ht_hkstorez(zval *assoc, pcbc_ht_key *hk, zval *value)
{
	const char *zh_key;
	int zh_nkey;
	
	ISARRAY_SANITY(assoc);
	HK_STRING(hk, zh_key, zh_nkey);
	add_assoc_zval_ex(assoc, zh_key, zh_nkey, value);
}

PHP_COUCHBASE_LOCAL
void pcbc_ht_dispose(zval *assoc)
{
	if (IS_ARRAY != Z_TYPE_P(assoc)) {
		php_error(E_RECOVERABLE_ERROR,
				"assoc_destroy given non-array zval, in couchbase php-ext");
	}

	zval_dtor(assoc);
}


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
