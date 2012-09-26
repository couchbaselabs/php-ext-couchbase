--TEST--
Replace - ReplaceOO
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Replace", "testReplaceOO");
--EXPECT--
PHP_COUCHBASE_OK