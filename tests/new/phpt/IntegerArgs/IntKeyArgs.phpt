--TEST--
IntegerArgs - IntKeyArgs
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("IntegerArgs", "testIntKeyArgs");
--EXPECT--
PHP_COUCHBASE_OK