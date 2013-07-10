--TEST--
GetReplica - GetMultipleReplicasStrategyFirst

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("GetReplica", "testGetMultipleReplicasStrategyFirst");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("GetReplica", "testGetMultipleReplicasStrategyFirst");
--EXPECT--
PHP_COUCHBASE_OK