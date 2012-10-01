--TEST--
Connection - ConnectNodeArray

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Connection", "testConnectNodeArray");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Connection", "testConnectNodeArray");
--EXPECT--
PHP_COUCHBASE_OK