--TEST--
Store - SetSingleOO

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Store", "testSetSingleOO");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Store", "testSetSingleOO");
--EXPECT--
PHP_COUCHBASE_OK