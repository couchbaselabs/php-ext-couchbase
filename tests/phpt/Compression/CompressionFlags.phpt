--TEST--
Compression - CompressionFlags

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Compression", "testCompressionFlags");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Compression", "testCompressionFlags");
--EXPECT--
PHP_COUCHBASE_OK