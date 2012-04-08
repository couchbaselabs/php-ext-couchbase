--TEST--
Check for couchbase_view with query parameters
--SKIPIF--
<?php include "skipif.inc" ?>
--FILE--
<?php
include "couchbase.inc";
$handle = couchbase_connect(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);

$result = couchbase_view($handle, "_all_docs", "");
foreach ($result["rows"] as $key => $value) {
    couchbase_delete($handle, $value["key"]);
}

$contents = array(
     "foo" => '{"value":"fooval"}',
     "bar" => '{"value":"barval"}',
);

couchbase_set_multi($handle, $contents);
sleep(1);
$result = couchbase_view($handle, "_all_docs?include_docs=true", "");
foreach ($result["rows"] as $key => $value) {
    print_r($value);
}

$handle = new Couchbase(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);
$result = $handle->view("_all_docs?include_docs=true", "");
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
            [rev] => 1-%s
        )

    [doc] => Array
        (
            [_id] => bar
            [_rev] => 1-%s
            [$flags] => 0
            [$expiration] => 0
            [value] => barval
        )

)
Array
(
    [id] => foo
    [key] => foo
    [value] => Array
        (
            [rev] => 1-%s
        )

    [doc] => Array
        (
            [_id] => foo
            [_rev] => 1-%s
            [$flags] => 0
            [$expiration] => 0
            [value] => fooval
        )

)
Array
(
    [id] => bar
    [key] => bar
    [value] => Array
        (
            [rev] => 1-%s
        )

    [doc] => Array
        (
            [_id] => bar
            [_rev] => 1-%s
            [$flags] => 0
            [$expiration] => 0
            [value] => barval
        )

)
Array
(
    [id] => foo
    [key] => foo
    [value] => Array
        (
            [rev] => 1-%s
        )

    [doc] => Array
        (
            [_id] => foo
            [_rev] => 1-%s
            [$flags] => 0
            [$expiration] => 0
            [value] => fooval
        )

)
