--TEST--
Connection - ConnectUri

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Connection", "testConnectUri");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Connection", "testConnectUri");
--EXPECT--
PHP_COUCHBASE_OK