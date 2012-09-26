--TEST--
Serialization - SerializeFileError
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Serialization", "testSerializeFileError");
--EXPECT--
PHP_COUCHBASE_OK