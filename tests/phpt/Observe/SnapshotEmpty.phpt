--TEST--
Observe - SnapshotEmpty

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Observe", "testSnapshotEmpty");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Observe", "testSnapshotEmpty");
--EXPECT--
PHP_COUCHBASE_OK