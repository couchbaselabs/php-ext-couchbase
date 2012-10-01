--TEST--
GetMulti - MgetPartial

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("GetMulti", "testMgetPartial");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("GetMulti", "testMgetPartial");
--EXPECT--
PHP_COUCHBASE_OK