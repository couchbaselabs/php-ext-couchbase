--TEST--
Store - AddWhenExistsOO

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Store", "testAddWhenExistsOO");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Store", "testAddWhenExistsOO");
--EXPECT--
PHP_COUCHBASE_OK