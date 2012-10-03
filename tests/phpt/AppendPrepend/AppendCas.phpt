--TEST--
AppendPrepend - AppendCas

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("AppendPrepend", "testAppendCas");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("AppendPrepend", "testAppendCas");
--EXPECT--
PHP_COUCHBASE_OK