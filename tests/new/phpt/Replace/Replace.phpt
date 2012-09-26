--TEST--
Replace - Replace
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Replace", "testReplace");
--EXPECT--
PHP_COUCHBASE_OK