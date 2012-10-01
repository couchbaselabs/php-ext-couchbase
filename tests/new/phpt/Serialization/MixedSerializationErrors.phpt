--TEST--
Serialization - MixedSerializationErrors

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Serialization", "testMixedSerializationErrors");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Serialization", "testMixedSerializationErrors");
--EXPECT--
PHP_COUCHBASE_OK