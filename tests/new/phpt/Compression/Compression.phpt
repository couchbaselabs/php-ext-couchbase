--TEST--
Compression - Compression
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Compression", "testCompression");
--EXPECT--
PHP_COUCHBASE_OK