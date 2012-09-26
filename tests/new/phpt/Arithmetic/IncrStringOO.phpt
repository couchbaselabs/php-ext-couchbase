--TEST--
Arithmetic - IncrStringOO
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Arithmetic", "testIncrStringOO");
--EXPECT--
PHP_COUCHBASE_OK