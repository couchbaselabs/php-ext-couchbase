--TEST--
GetMulti - PlainOO
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("GetMulti", "testPlainOO");
--EXPECT--
PHP_COUCHBASE_OK