--TEST--
NegativeExpiry - Set
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("NegativeExpiry", "testSet");
--EXPECT--
PHP_COUCHBASE_OK