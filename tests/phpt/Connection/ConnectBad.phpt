--TEST--
Connection - ConnectBad

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Connection", "testConnectBad");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Connection", "testConnectBad");
--EXPECT--
PHP_COUCHBASE_OK