--TEST--
Delay - InvalidCb

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Delay", "testInvalidCb");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Delay", "testInvalidCb");
--EXPECT--
PHP_COUCHBASE_OK