--TEST--
ViewSimple - SingleUri
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("ViewSimple", "testSingleUri");
--EXPECT--
PHP_COUCHBASE_OK