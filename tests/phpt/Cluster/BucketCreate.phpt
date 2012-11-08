--TEST--
Cluster - BucketCreate

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Cluster", "testBucketCreate");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Cluster", "testBucketCreate");
--EXPECT--
PHP_COUCHBASE_OK