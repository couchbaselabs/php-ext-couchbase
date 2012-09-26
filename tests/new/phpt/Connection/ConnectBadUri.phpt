--TEST--
Connection - ConnectBadUri
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Connection", "testConnectBadUri");
--EXPECT--
PHP_COUCHBASE_OK