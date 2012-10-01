--TEST--
NegativeExpiry - Increment

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("NegativeExpiry", "testIncrement");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("NegativeExpiry", "testIncrement");
--EXPECT--
PHP_COUCHBASE_OK