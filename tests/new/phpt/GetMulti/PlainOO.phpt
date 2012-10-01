--TEST--
GetMulti - PlainOO

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("GetMulti", "testPlainOO");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("GetMulti", "testPlainOO");
--EXPECT--
PHP_COUCHBASE_OK