--TEST--
Store - SetMultiOO

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Store", "testSetMultiOO");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Store", "testSetMultiOO");
--EXPECT--
PHP_COUCHBASE_OK