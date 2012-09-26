--TEST--
GetMulti - SetMultiOO
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("GetMulti", "testSetMultiOO");
--EXPECT--
PHP_COUCHBASE_OK