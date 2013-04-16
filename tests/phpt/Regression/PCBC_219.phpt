--TEST--
Regression - PCBC_219

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Regression", "testPCBC_219");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Regression", "testPCBC_219");
--EXPECT--
PHP_COUCHBASE_OK