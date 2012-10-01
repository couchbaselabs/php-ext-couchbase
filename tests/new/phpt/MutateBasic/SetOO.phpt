--TEST--
MutateBasic - SetOO

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("MutateBasic", "testSetOO");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("MutateBasic", "testSetOO");
--EXPECT--
PHP_COUCHBASE_OK