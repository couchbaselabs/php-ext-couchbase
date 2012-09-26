--TEST--
MissCB - MissCb
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("MissCB", "testMissCb");
--EXPECT--
PHP_COUCHBASE_OK