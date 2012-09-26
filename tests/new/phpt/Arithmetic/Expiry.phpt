--TEST--
Arithmetic - Expiry
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Arithmetic", "testExpiry");
--EXPECT--
PHP_COUCHBASE_OK