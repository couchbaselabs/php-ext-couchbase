--TEST--
Get - GetWithLock

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Get", "testGetWithLock");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Get", "testGetWithLock");
--EXPECT--
PHP_COUCHBASE_OK