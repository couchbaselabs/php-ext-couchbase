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

/**
 * Compression stuff.
 * First, I'd like to note that I painfully had to go through and poke around
 * the php-memcached extension code from whence the compression headers and
 * semantics are derived
 *
 * The compression scheme is fairly simple, and is compatible with the older
 * memcached php clients.
 *
 * The first sizeof(uint32_t) bytes contain the uncompressed length, and the
 * compressed data follows.
 *
 * To make the rest of the code easier to follow, I've divided the compression
 * and decompression functions into these steps:
 *
 * 1) Allocate a buffer. This is the upper bound for allocation. This maintains
 * 		a private pointer to the base allocation, and exposes a 'data' field
 * 		which is an offset of sizeof(uint32_t) bytes into the allocated buffer.
 *
 * 		The 'space' is subsequently sizeof(uint32_t) bytes less than the actual
 * 		allocated length (but we don't care about this..)
 *
 * 2) Compress the input.. this is simple.. and involves populating the ->len
 * 		field of the structure with the compressed length
 *
 * 3) 'Deploy' the buffer. Encode the length as a size_t into the 'header'.
 *
 *
 */

#include "internal.h"

#define CBCOMPHDR_SANITY_MAX 0x40000000

static char *cbcomp_new(php_couchbase_comp *str,
                        size_t cmpbuf_len, size_t origlen)
{
	str->alloc = cmpbuf_len;
	str->_base = emalloc(str->alloc + sizeof(pcbc_payload_len_t));
	if (!str->_base) {
		return NULL; /* enomem */
	}

	str->data = str->_base + sizeof(pcbc_payload_len_t);
	*(pcbc_payload_len_t *)str->_base = origlen;
	return str->data;
}

PHP_COUCHBASE_LOCAL
void cbcomp_free(php_couchbase_comp *str)
{
	free(str->_base);
	str->data = NULL;
	str->_base = NULL;
}

/**
 * Called to 'deploy' the compressed buffer. This adjusts the base and the
 * length so that they will include the "header". Do not call this function
 * twice on the same comp
 */
PHP_COUCHBASE_LOCAL
void cbcomp_deploy(php_couchbase_comp *str)
{
	/* Expose the rest, now that we're done with compression */
	str->data = str->_base;
	str->compressed_len += sizeof(pcbc_payload_len_t);
}



/* headers which claim an uncompressed size above this figure are bad */
#define DECOMP_SANITY_LIMIT 0x40000000

/* make sure to initialize info to zero before passing here */
PHP_COUCHBASE_LOCAL
int cbcomp_dcmp_init(const char *data, size_t len,
                     php_couchbase_decomp *info)
{
	int ret = 0;
	/* make a sane value size, say, 1GB? */
	if (len < sizeof(pcbc_payload_len_t)) {
		return 0;
	}

	info->orig = data;
	info->orig_len = len;

	info->compressed = data + sizeof(pcbc_payload_len_t);
	info->compressed_len = len - sizeof(pcbc_payload_len_t);

	info->expanded_len = *(pcbc_payload_len_t *)data;
	info->expanded = NULL;

	return 1;
}

PHP_COUCHBASE_LOCAL
void cbcomp_dcmp_free(php_couchbase_decomp *info)
{
	if (info->expanded) {
		efree(info->expanded);
		info->expanded = NULL;
	}
}

#ifdef HAVE_COMPRESSION_ZLIB
PHP_COUCHBASE_LOCAL
int php_couchbase_compress_zlib(const smart_str *input,
                                php_couchbase_comp *output)
{
	/* compressBound tells us the maximum size of the buffer we'll need.. */
	cbcomp_new(output, compressBound(input->len), input->len);
	uLongf tmp_ulen = output->alloc;
	int status = compress(
	                 (Bytef *)output->data, &tmp_ulen,
	                 (Bytef *)input->c, input->len);

	if (status == Z_OK) {
		output->compressed_len = tmp_ulen;
		return 1;
	}

	return 0;
}

PHP_COUCHBASE_LOCAL
int php_couchbase_decompress_zlib(php_couchbase_decomp *info)
{
	int status;
	uLongf dlen;

	size_t n_alloc;

	/**
	 * in the event that the header is zero, make it default to 4096.
	 * This will automatically fall back to 'old-style' decompression.
	 */
	if (!info->expanded_len) {
		info->expanded_len = 4096;
	}

	info->expanded = emalloc(info->expanded_len);
	n_alloc = info->expanded_len;
	dlen = n_alloc;

	if (!info->expanded_len) {
		return 0;
	}

	status = Z_BUF_ERROR;
	/**
	 * sanity check, don't allocate over a GB, we should make this number
	 * smaller though
	 */

	if (info->expanded_len < CBCOMPHDR_SANITY_MAX) {
		status = uncompress((Bytef *)info->expanded, &dlen,
		                    (Bytef *)info->compressed, info->orig_len);
	}

	/* fall back to 'old-style' decompression */
	while (status == Z_BUF_ERROR) {

		n_alloc *= 2;
		info->expanded = erealloc(info->expanded, n_alloc);
		dlen = n_alloc;

		status = uncompress((Bytef *)info->expanded, &dlen,
		                    (Bytef *)info->orig, info->orig_len);
	}

	if (status != Z_OK) {
		efree(info->expanded);
		info->expanded = NULL;
	} else {
		info->expanded_len = dlen;
	}

	return status == Z_OK;
}
#endif /* HAVE_COMPRESSION_ZLIB */

#ifdef HAVE_COMPRESSION_FASTLZ
PHP_COUCHBASE_LOCAL
int php_couchbase_compress_fastlz(const smart_str *input,
                                  php_couchbase_comp *output)
{
	cbcomp_new(output,
	           (size_t)((input->len * 1.05) + 1),
	           input->len);

	return (output->compressed_len =
	            (fastlz_compress(input->c, input->len, output->data)));
}

PHP_COUCHBASE_LOCAL
int php_couchbase_decompress_fastlz(php_couchbase_decomp *info)
{
	if (info->expanded_len == 0 || info->expanded_len > CBCOMPHDR_SANITY_MAX) {
		return 0;
	}

	info->expanded = emalloc(info->expanded_len);
	if (!info->expanded) {
		return 0;
	}

	info->expanded_len = fastlz_decompress(info->compressed,
	                                       info->compressed_len, info->expanded, info->expanded_len);

	return info->expanded_len;
}
#endif
