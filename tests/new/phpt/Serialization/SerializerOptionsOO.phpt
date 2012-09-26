--TEST--
Serialization - SerializerOptionsOO
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Serialization", "testSerializerOptionsOO");
--EXPECT--
PHP_COUCHBASE_OK