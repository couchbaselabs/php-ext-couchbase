--TEST--
GetMulti - SetMulti

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("GetMulti", "testSetMulti");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("GetMulti", "testSetMulti");
--EXPECT--
PHP_COUCHBASE_OK