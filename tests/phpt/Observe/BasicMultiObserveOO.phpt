--TEST--
Observe - BasicMultiObserveOO

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Observe", "testBasicMultiObserveOO");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Observe", "testBasicMultiObserveOO");
--EXPECT--
PHP_COUCHBASE_OK