--TEST--
Get - GetMissingWithCallbackOO

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Get", "testGetMissingWithCallbackOO");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Get", "testGetMissingWithCallbackOO");
--EXPECT--
PHP_COUCHBASE_OK