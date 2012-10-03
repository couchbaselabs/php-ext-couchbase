--TEST--
ViewSimple - MissingView

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("ViewSimple", "testMissingView");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("ViewSimple", "testMissingView");
--EXPECT--
PHP_COUCHBASE_OK