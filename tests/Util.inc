<?php
require_once dirname(__FILE__).'/couchbase.inc';
function make_handle($persistent = true) {
    $handle = couchbase_connect(COUCHBASE_CONFIG_HOST,
                               COUCHBASE_CONFIG_USER,
                               COUCHBASE_CONFIG_PASSWD,
                               COUCHBASE_CONFIG_BUCKET,
                               $persistent);
    return $handle;
}

function make_handle_oo($persistent = true) {
    $oo = new Couchbase(COUCHBASE_CONFIG_HOST,
                        COUCHBASE_CONFIG_USER,
                        COUCHBASE_CONFIG_PASSWD,
                        COUCHBASE_CONFIG_BUCKET,
                        $persistent);
    return $oo;
}
