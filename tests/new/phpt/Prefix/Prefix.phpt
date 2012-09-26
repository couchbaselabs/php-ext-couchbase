--TEST--
Prefix - Prefix
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Prefix", "testPrefix");
--EXPECT--
PHP_COUCHBASE_OK