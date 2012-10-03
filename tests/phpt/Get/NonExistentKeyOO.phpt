--TEST--
Get - NonExistentKeyOO

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Get", "testNonExistentKeyOO");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Get", "testNonExistentKeyOO");
--EXPECT--
PHP_COUCHBASE_OK