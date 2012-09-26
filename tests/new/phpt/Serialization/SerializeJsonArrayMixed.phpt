--TEST--
Serialization - SerializeJsonArrayMixed
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Serialization", "testSerializeJsonArrayMixed");
--EXPECT--
PHP_COUCHBASE_OK