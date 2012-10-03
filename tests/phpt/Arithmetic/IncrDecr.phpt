--TEST--
Arithmetic - IncrDecr

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Arithmetic", "testIncrDecr");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Arithmetic", "testIncrDecr");
--EXPECT--
PHP_COUCHBASE_OK