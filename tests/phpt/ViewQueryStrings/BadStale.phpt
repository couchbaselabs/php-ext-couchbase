--TEST--
ViewQueryStrings - BadStale

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("ViewQueryStrings", "testBadStale");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("ViewQueryStrings", "testBadStale");
--EXPECT--
PHP_COUCHBASE_OK