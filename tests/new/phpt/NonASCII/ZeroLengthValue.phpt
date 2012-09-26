--TEST--
NonASCII - ZeroLengthValue
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("NonASCII", "testZeroLengthValue");
--EXPECT--
PHP_COUCHBASE_OK