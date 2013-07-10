--TEST--
GetReplica - GetReplicaStrategySelect

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("GetReplica", "testGetReplicaStrategySelect");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("GetReplica", "testGetReplicaStrategySelect");
--EXPECT--
PHP_COUCHBASE_OK