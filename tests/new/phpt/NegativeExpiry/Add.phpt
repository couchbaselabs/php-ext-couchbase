--TEST--
NegativeExpiry - Add
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("NegativeExpiry", "testAdd");
--EXPECT--
PHP_COUCHBASE_OK