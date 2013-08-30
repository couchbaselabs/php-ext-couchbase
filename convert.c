/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright 2012 Couchbase, Inc.                                       |
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

PHP_COUCHBASE_LOCAL
void pcbc_json_decode(zval *zv, char *data, int ndata, zend_bool assoc
					  TSRMLS_DC)
{
	php_json_decode(zv, data, ndata, assoc, 512 TSRMLS_CC);
}

PHP_COUCHBASE_LOCAL
void pcbc_json_encode(smart_str *buf, zval *value TSRMLS_DC)
{
	php_json_encode(buf, value, 0 TSRMLS_CC);
}

PHP_COUCHBASE_LOCAL
char *php_couchbase_zval_to_payload(zval *value, size_t *payload_len, unsigned int *flags, int serializer, int compressor TSRMLS_DC)
{
	char *payload = NULL;
	smart_str buf = {0};
	switch (Z_TYPE_P(value)) {
	case IS_STRING:
		smart_str_appendl(&buf, Z_STRVAL_P(value), Z_STRLEN_P(value));
		COUCHBASE_VAL_SET_TYPE(*flags, COUCHBASE_VAL_IS_STRING);
		COUCHBASE_SET_COMPRESSION(*flags, compressor);
		break;
	case IS_LONG:
	case IS_DOUBLE:
	case IS_BOOL: {
		zval value_copy;
		value_copy = *value;
		zval_copy_ctor(&value_copy);
		convert_to_string(&value_copy);
		smart_str_appendl(&buf, Z_STRVAL(value_copy), Z_STRLEN(value_copy));
		zval_dtor(&value_copy);
		switch (Z_TYPE_P(value)) {
		case IS_LONG:
			COUCHBASE_VAL_SET_TYPE(*flags, COUCHBASE_VAL_IS_LONG);
			break;
		case IS_DOUBLE:
			COUCHBASE_VAL_SET_TYPE(*flags, COUCHBASE_VAL_IS_DOUBLE);
			break;
		case IS_BOOL:
			COUCHBASE_VAL_SET_TYPE(*flags, COUCHBASE_VAL_IS_BOOL);
			break;
		}
		break;
	}
	default:
		COUCHBASE_SET_COMPRESSION(*flags, compressor);
		switch (serializer) {
		case COUCHBASE_SERIALIZER_JSON:
		case COUCHBASE_SERIALIZER_JSON_ARRAY:
			pcbc_json_encode(&buf, value TSRMLS_CC);
			COUCHBASE_VAL_SET_TYPE(*flags, COUCHBASE_VAL_IS_JSON);
			break;

#ifdef HAVE_IGBINARY
		case COUCHBASE_SERIALIZER_IGBINARY:
			if (igbinary_serialize((uint8_t **)&buf.c,
								   &buf.len,
								   value TSRMLS_CC) != 0) {
				smart_str_free(&buf);
				php_error_docref(NULL TSRMLS_CC, E_WARNING,
								 "Failed to serialize value with igbinary");
				return NULL;
			}
			COUCHBASE_VAL_SET_TYPE(*flags, COUCHBASE_VAL_IS_IGBINARY);
			break;
#endif

		default: {
			php_serialize_data_t var_hash;
			PHP_VAR_SERIALIZE_INIT(var_hash);
			php_var_serialize(&buf, &value, &var_hash TSRMLS_CC);
			PHP_VAR_SERIALIZE_DESTROY(var_hash);

			if (!buf.c) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "could not serialize value");
				smart_str_free(&buf);
				return NULL;
			}

			COUCHBASE_VAL_SET_TYPE(*flags, COUCHBASE_VAL_IS_SERIALIZED);
			break;
		}
		}
		break;
	}

	if ((COUCHBASE_GET_COMPRESSION(*flags)) && buf.len < COUCHBASE_G(compression_threshold)) {
		COUCHBASE_SET_COMPRESSION(*flags, COUCHBASE_COMPRESSION_NONE);
	}

	if (COUCHBASE_GET_COMPRESSION(*flags)) {
		/* status */
		zend_bool compress_status = 0;
		php_couchbase_comp cmpbuf = { NULL };
		/* Additional 5% for the data (LZ) */
		switch (compressor) {
		case COUCHBASE_COMPRESSION_FASTLZ:
			compress_status = php_couchbase_compress_fastlz(&buf, &cmpbuf);
			break;
		case COUCHBASE_COMPRESSION_ZLIB:
#ifdef HAVE_COMPRESSION_ZLIB
			compress_status = php_couchbase_compress_zlib(&buf, &cmpbuf);
#else
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "could not compress value, no zlib lib support");
			return NULL;
#endif
			break;
		default:
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "unknown compressor type: %d", compressor);
			return NULL;
		}

		if (!compress_status) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "could not compress value");
			cbcomp_free(&cmpbuf);
			smart_str_free(&buf);
			return NULL;
		}



		/* Check that we are above ratio */
		if (buf.len > (cmpbuf.compressed_len * COUCHBASE_G(compression_factor))) {
			cbcomp_deploy(&cmpbuf);
			*payload_len = cmpbuf.compressed_len;
			payload = cmpbuf.data;
			*flags |= COUCHBASE_COMPRESSION_MCISCOMPRESSED;

		} else {
			COUCHBASE_SET_COMPRESSION(*flags, COUCHBASE_COMPRESSION_NONE);
			*payload_len = buf.len;
			memcpy(payload, buf.c, buf.len);
			cbcomp_free(&cmpbuf);
			cmpbuf.data = NULL;
		}

	} else {
		*payload_len = buf.len;
		payload = estrndup(buf.c, buf.len);
	}

	smart_str_free(&buf);
	return payload;
}

PHP_COUCHBASE_LOCAL
int php_couchbase_zval_from_payload(zval *value, char *payload, size_t payload_len, unsigned int flags, int serializer, int ignoreflags TSRMLS_DC)
{
	int compressor;
	zend_bool payload_emalloc = 0;

	if (payload == NULL && payload_len > 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
						 "could not handle non-existing value of length %zu", payload_len);
		return 0;
	} else if (payload == NULL) {
		if ((flags & 127) == IS_BOOL) {
			ZVAL_FALSE(value);
		} else {
			ZVAL_EMPTY_STRING(value);
		}
		return 1;
	}

	if (ignoreflags) {
		ZVAL_STRINGL(value, payload, payload_len, 1);
		return 1;
	}

	if ((compressor = COUCHBASE_GET_COMPRESSION(flags))) {

		php_couchbase_decomp dcmp = { NULL };
		zend_bool decompress_status = 0;

		cbcomp_dcmp_init(payload, payload_len, &dcmp);

		switch (compressor) {
		case COUCHBASE_COMPRESSION_FASTLZ:
			decompress_status = php_couchbase_decompress_fastlz(&dcmp);
			break;
		case COUCHBASE_COMPRESSION_ZLIB:
#ifdef HAVE_COMPRESSION_ZLIB
			decompress_status = php_couchbase_decompress_zlib(&dcmp);
#else
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "could not decompress value, no zlib lib support");
			return 0;
#endif
			break;
		}

		if (!decompress_status) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "could not decompress value (bad content)");
			cbcomp_dcmp_free(&dcmp);
			return 0;
		}

		payload = dcmp.expanded;
		payload_len = dcmp.expanded_len;
		payload_emalloc = 1;
	}

	switch (COUCHBASE_VAL_GET_TYPE(flags)) {
	case COUCHBASE_VAL_IS_STRING:
		ZVAL_STRINGL(value, payload, payload_len, 1);
		break;

		//case 0: /* see http://www.couchbase.com/issues/browse/PCBC-30 */
	case COUCHBASE_VAL_IS_LONG: {
		long lval;
		char *buf = emalloc(payload_len + sizeof(char));
		memcpy(buf, payload, payload_len);
		buf[payload_len] = '\0';
		lval = strtol(buf, NULL, 10);
		efree(buf);
		ZVAL_LONG(value, lval);
		break;
	}

	case COUCHBASE_VAL_IS_DOUBLE: {
		double dval;
		char *buf = emalloc(payload_len + sizeof(char));
		memcpy(buf, payload, payload_len);
		buf[payload_len] = '\0';
		dval = zend_strtod(payload, NULL);
		efree(buf);
		ZVAL_DOUBLE(value, dval);
		break;
	}

	case COUCHBASE_VAL_IS_BOOL:
		ZVAL_BOOL(value, payload_len > 0 && payload[0] == '1');
		break;

	case COUCHBASE_VAL_IS_SERIALIZED: {
		const char *payload_tmp = payload;
		php_unserialize_data_t var_hash;

		PHP_VAR_UNSERIALIZE_INIT(var_hash);
		if (!php_var_unserialize(&value, (const unsigned char **)&payload_tmp, (const unsigned char *)payload_tmp + payload_len, &var_hash TSRMLS_CC)) {
			ZVAL_FALSE(value);
			PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "could not unserialize value");
			if (payload_emalloc) {
				efree(payload);
			}
			return 0;
		}
		PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
		break;
	}

	case COUCHBASE_VAL_IS_JSON:
		pcbc_json_decode(value, payload, payload_len,
						 (serializer == COUCHBASE_SERIALIZER_JSON_ARRAY)
						 TSRMLS_CC);
		break;

	case COUCHBASE_VAL_IS_IGBINARY:
#if HAVE_IGBINARY
		if (igbinary_unserialize((void *)payload,
								 payload_len, &value TSRMLS_CC)) {
			ZVAL_FALSE(value);
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
							 "Failed to deserialize with igbinary");
			return 1;
		}
#else
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
						 "Could not deserialize. The extension is built without support for igbinary");
		return 1;
#endif

	default:
		if (payload_emalloc) {
			efree(payload);
		}
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "unknown payload type");
		return 0;
	}

	if (payload_emalloc) {
		efree(payload);
	}

	return 1;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
