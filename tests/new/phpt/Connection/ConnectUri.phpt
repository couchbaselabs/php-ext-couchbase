--TEST--
Connection - ConnectUri
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Connection", "testConnectUri");
--EXPECT--
PHP_COUCHBASE_OK