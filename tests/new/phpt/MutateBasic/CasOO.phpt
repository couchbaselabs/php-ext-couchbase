--TEST--
MutateBasic - CasOO
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("MutateBasic", "testCasOO");
--EXPECT--
PHP_COUCHBASE_OK