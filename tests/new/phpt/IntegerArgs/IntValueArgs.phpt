--TEST--
IntegerArgs - IntValueArgs
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("IntegerArgs", "testIntValueArgs");
--EXPECT--
PHP_COUCHBASE_OK