--TEST--
MiscOptions - MiscOptions
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("MiscOptions", "testMiscOptions");
--EXPECT--
PHP_COUCHBASE_OK