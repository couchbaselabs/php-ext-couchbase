--TEST--
Delay - ContentCb
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Delay", "testContentCb");
--EXPECT--
PHP_COUCHBASE_OK