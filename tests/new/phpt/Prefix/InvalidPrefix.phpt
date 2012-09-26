--TEST--
Prefix - InvalidPrefix
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Prefix", "testInvalidPrefix");
--EXPECT--
PHP_COUCHBASE_OK