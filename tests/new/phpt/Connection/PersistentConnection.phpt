--TEST--
Connection - PersistentConnection
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Connection", "testPersistentConnection");
--EXPECT--
PHP_COUCHBASE_OK