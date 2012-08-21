--TEST--
Check for couchbase_view
--SKIPIF--
<?php include "skipif.inc" ?>
--FILE--
<?php
include "couchbase.inc";
$handle = couchbase_connect(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);

sleep(3);

$result = couchbase_view($handle, "_all_docs", "");
foreach ($result["rows"] as $key => $value) {
    couchbase_delete($handle, $value["key"]);
}

sleep(1);

couchbase_set($handle, "foo", '{"value":"fooval"}');
couchbase_set($handle, "bar", '{"value":"barval"}');

sleep(3);

$result = couchbase_view($handle, "_all_docs", "");
foreach ($result["rows"] as $key => $value) {
    print_r($value);
}

$handle = new Couchbase(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);
$result = $handle->view("_all_docs", "");
foreach ($result["rows"] as $key => $value) {
    print_r($value);
}

?>
--EXPECTF--
Array
(
    [id] => bar
    [key] => bar
    [value] => Array
        (
            [rev] => %s
        )

)
Array
(
    [id] => foo
    [key] => foo
    [value] => Array
        (
            [rev] => %s
        )

)
Array
(
    [id] => bar
    [key] => bar
    [value] => Array
        (
            [rev] => %s
        )

)
Array
(
    [id] => foo
    [key] => foo
    [value] => Array
        (
            [rev] => %s
        )

)
