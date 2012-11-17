--TEST--
ViewQueryStrings - StaleParams

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("ViewQueryStrings", "testStaleParams");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("ViewQueryStrings", "testStaleParams");
--EXPECT--
PHP_COUCHBASE_OK