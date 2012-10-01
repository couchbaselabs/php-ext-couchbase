--TEST--
MissCB - MissCb

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("MissCB", "testMissCb");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("MissCB", "testMissCb");
--EXPECT--
PHP_COUCHBASE_OK