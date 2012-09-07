--TEST--
Compression - CompressionFastLZ

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Compression", "testCompressionFastLZ");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Compression", "testCompressionFastLZ");
--EXPECT--
PHP_COUCHBASE_OK