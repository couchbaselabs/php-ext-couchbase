--TEST--
MutateBasic - AddOO

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("MutateBasic", "testAddOO");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("MutateBasic", "testAddOO");
--EXPECT--
PHP_COUCHBASE_OK