--TEST--
ServerStats - Stats
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("ServerStats", "testStats");
--EXPECT--
PHP_COUCHBASE_OK