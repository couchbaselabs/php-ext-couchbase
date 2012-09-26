--TEST--
Replace - ReplaceCas
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Replace", "testReplaceCas");
--EXPECT--
PHP_COUCHBASE_OK