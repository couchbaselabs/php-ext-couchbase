--TEST--
Delay - AnonCb

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Delay", "testAnonCb");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Delay", "testAnonCb");
--EXPECT--
PHP_COUCHBASE_OK