--TEST--
GetReplica - GetMultipleReplicasStrategySelect

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("GetReplica", "testGetMultipleReplicasStrategySelect");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("GetReplica", "testGetMultipleReplicasStrategySelect");
--EXPECT--
PHP_COUCHBASE_OK