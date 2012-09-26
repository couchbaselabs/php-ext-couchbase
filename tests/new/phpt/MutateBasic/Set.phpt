--TEST--
MutateBasic - Set
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("MutateBasic", "testSet");
--EXPECT--
PHP_COUCHBASE_OK