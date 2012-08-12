--TEST--
Multi-Node Array
--SKIPIF--
<?php include "skipif.inc" ?>
--INI--

--FILE--
<?php

include "couchbase.inc";
$hosts = array(
    "non-existent-host",
    "another-bogus-host",
    COUCHBASE_CONFIG_HOST
);

$cb = new Couchbase($hosts,
    COUCHBASE_CONFIG_USER,
    COUCHBASE_CONFIG_PASSWD,
    COUCHBASE_CONFIG_BUCKET);
print_r($cb);
?>

--EXPECTF--
Couchbase Object
(
    [%s] => Resource id #%d
)
