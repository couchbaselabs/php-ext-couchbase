--TEST--
Check for couchbase_view with query parameters
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

$contents = array(
     "foo" => '{"value":"fooval"}',
     "bar" => '{"value":"barval"}',
);

couchbase_set_multi($handle, $contents);

sleep(3);

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
            [rev] => %s
        )

    [doc] => Array
        (
            [meta] => Array
                (
                    [id] => bar
                    [rev] => %s
                    [expiration] => 0
                    [flags] => 0
                )

            [json] => Array
                (
                    [value] => barval
                )

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

    [doc] => Array
        (
            [meta] => Array
                (
                    [id] => foo
                    [rev] => %s
                    [expiration] => 0
                    [flags] => 0
                )

            [json] => Array
                (
                    [value] => fooval
                )

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

    [doc] => Array
        (
            [meta] => Array
                (
                    [id] => bar
                    [rev] => %s
                    [expiration] => 0
                    [flags] => 0
                )

            [json] => Array
                (
                    [value] => barval
                )

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

    [doc] => Array
        (
            [meta] => Array
                (
                    [id] => foo
                    [rev] => %s
                    [expiration] => 0
                    [flags] => 0
                )

            [json] => Array
                (
                    [value] => fooval
                )

        )

)
