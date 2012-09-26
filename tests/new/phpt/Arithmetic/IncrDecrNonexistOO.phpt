--TEST--
Arithmetic - IncrDecrNonexistOO
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Arithmetic", "testIncrDecrNonexistOO");
--EXPECT--
PHP_COUCHBASE_OK