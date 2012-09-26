--TEST--
Arithmetic - IncrString
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Arithmetic", "testIncrString");
--EXPECT--
PHP_COUCHBASE_OK