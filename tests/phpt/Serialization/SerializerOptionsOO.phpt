--TEST--
Serialization - SerializerOptionsOO

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Serialization", "testSerializerOptionsOO");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Serialization", "testSerializerOptionsOO");
--EXPECT--
PHP_COUCHBASE_OK