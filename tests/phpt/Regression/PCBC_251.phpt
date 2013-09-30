--TEST--
Regression - PCBC_251

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Regression", "testPCBC_251");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Regression", "testPCBC_251");
--EXPECT--
PHP_COUCHBASE_OK