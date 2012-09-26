--TEST--
Serialization - SerializerOptions
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Serialization", "testSerializerOptions");
--EXPECT--
PHP_COUCHBASE_OK