--TEST--
MutateBasic - Add

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("MutateBasic", "testAdd");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("MutateBasic", "testAdd");
--EXPECT--
PHP_COUCHBASE_OK