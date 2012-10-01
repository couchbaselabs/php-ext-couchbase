--TEST--
NegativeExpiry - Replace
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("NegativeExpiry", "testReplace");
--EXPECT--
PHP_COUCHBASE_OK