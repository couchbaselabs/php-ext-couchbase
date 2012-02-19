--TEST--
Check for couchbase_connect
--SKIPIF--
<?php include "skipif.inc" ?>
--FILE--
<?php
include "couchbase.inc";
$url  = "htTp://" . COUCHBASE_CONFIG_USER . ':' . COUCHBASE_CONFIG_PASSWD . '@' . COUCHBASE_CONFIG_HOST . '/' . COUCHBASE_CONFIG_BUCKET;
$handle = couchbase_connect($url);
var_dump($handle);

$url  = "hTtps://" . COUCHBASE_CONFIG_USER . ':' . COUCHBASE_CONFIG_PASSWD . '@' . COUCHBASE_CONFIG_HOST . '/' . COUCHBASE_CONFIG_BUCKET;
$handle = new Couchbase($url);
print_r($handle);
?>
--EXPECTF--
resource(%d) of type (Couchbase)
Couchbase Object
(
    [_handle:Couchbase:private] => Resource id #%d
)
