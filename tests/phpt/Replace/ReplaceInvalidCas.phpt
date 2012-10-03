--TEST--
Replace - ReplaceInvalidCas

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Replace", "testReplaceInvalidCas");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Replace", "testReplaceInvalidCas");
--EXPECT--
PHP_COUCHBASE_OK