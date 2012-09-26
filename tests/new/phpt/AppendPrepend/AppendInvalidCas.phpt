--TEST--
AppendPrepend - AppendInvalidCas
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("AppendPrepend", "testAppendInvalidCas");
--EXPECT--
PHP_COUCHBASE_OK