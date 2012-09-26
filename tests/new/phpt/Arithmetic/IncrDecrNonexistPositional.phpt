--TEST--
Arithmetic - IncrDecrNonexistPositional
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Arithmetic", "testIncrDecrNonexistPositional");
--EXPECT--
PHP_COUCHBASE_OK