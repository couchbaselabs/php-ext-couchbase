--TEST--
Regression - PCBC_191

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Regression", "testPCBC_191");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Regression", "testPCBC_191");
--EXPECT--
PHP_COUCHBASE_OK