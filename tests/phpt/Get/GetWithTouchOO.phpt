--TEST--
Get - GetWithTouchOO

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Get", "testGetWithTouchOO");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Get", "testGetWithTouchOO");
--EXPECT--
PHP_COUCHBASE_OK