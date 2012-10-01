--TEST--
MissCB - MissCbOO

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("MissCB", "testMissCbOO");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("MissCB", "testMissCbOO");
--EXPECT--
PHP_COUCHBASE_OK