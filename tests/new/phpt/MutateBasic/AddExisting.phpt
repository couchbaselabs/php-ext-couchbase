--TEST--
MutateBasic - AddExisting

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("MutateBasic", "testAddExisting");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("MutateBasic", "testAddExisting");
--EXPECT--
PHP_COUCHBASE_OK