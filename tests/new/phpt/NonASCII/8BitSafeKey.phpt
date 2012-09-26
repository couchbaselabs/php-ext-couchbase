--TEST--
NonASCII - 8BitSafeKey
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("NonASCII", "test8BitSafeKey");
--EXPECT--
PHP_COUCHBASE_OK