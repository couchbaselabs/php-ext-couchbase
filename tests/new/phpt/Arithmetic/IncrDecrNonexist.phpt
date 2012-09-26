--TEST--
Arithmetic - IncrDecrNonexist
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Arithmetic", "testIncrDecrNonexist");
--EXPECT--
PHP_COUCHBASE_OK