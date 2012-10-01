--TEST--
Delay - FetchAll

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Delay", "testFetchAll");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Delay", "testFetchAll");
--EXPECT--
PHP_COUCHBASE_OK