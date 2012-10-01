--TEST--
ViewSimple - NoRes

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("ViewSimple", "testNoRes");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("ViewSimple", "testNoRes");
--EXPECT--
PHP_COUCHBASE_OK