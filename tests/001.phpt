--TEST--
Check for couchbase presence
--SKIPIF--
<?php if (!extension_loaded("couchbase")) print "skip"; ?>
--FILE--
<?php
if (class_exists("Couchbase")) {
   echo "couchbase extension is available";
}
?>
--EXPECT--
couchbase extension is available
