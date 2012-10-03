--TEST--
Store - AddWhenExists

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Store", "testAddWhenExists");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Store", "testAddWhenExists");
--EXPECT--
PHP_COUCHBASE_OK