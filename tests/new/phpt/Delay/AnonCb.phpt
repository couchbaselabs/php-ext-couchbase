--TEST--
Delay - AnonCb
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Delay", "testAnonCb");
--EXPECT--
PHP_COUCHBASE_OK