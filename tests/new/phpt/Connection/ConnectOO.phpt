--TEST--
Connection - ConnectOO
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Connection", "testConnectOO");
--EXPECT--
PHP_COUCHBASE_OK