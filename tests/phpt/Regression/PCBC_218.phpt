--TEST--
Regression - PCBC_218

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Regression", "testPCBC_218");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Regression", "testPCBC_218");
--EXPECT--
PHP_COUCHBASE_OK