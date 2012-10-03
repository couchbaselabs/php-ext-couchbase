--TEST--
Replace - ReplaceCas

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Replace", "testReplaceCas");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Replace", "testReplaceCas");
--EXPECT--
PHP_COUCHBASE_OK