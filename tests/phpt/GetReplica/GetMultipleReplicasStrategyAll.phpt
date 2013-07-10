--TEST--
GetReplica - GetMultipleReplicasStrategyAll

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("GetReplica", "testGetMultipleReplicasStrategyAll");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("GetReplica", "testGetMultipleReplicasStrategyAll");
--EXPECT--
PHP_COUCHBASE_OK