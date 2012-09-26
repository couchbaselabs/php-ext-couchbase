--TEST--
GetMulti - MgetOrdered
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("GetMulti", "testMgetOrdered");
--EXPECT--
PHP_COUCHBASE_OK