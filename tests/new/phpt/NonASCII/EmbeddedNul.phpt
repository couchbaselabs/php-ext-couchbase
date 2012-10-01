--TEST--
NonASCII - EmbeddedNul

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("NonASCII", "testEmbeddedNul");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("NonASCII", "testEmbeddedNul");
--EXPECT--
PHP_COUCHBASE_OK