--TEST--
Regression - PCBC_251_oo

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Regression", "testPCBC_251_oo");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Regression", "testPCBC_251_oo");
--EXPECT--
PHP_COUCHBASE_OK