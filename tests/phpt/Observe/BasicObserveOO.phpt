--TEST--
Observe - BasicObserveOO

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Observe", "testBasicObserveOO");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Observe", "testBasicObserveOO");
--EXPECT--
PHP_COUCHBASE_OK