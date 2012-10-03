<?php
require_once dirname(__FILE__).'/couchbase.inc';
function make_handle() {
    $handle = couchbase_connect(COUCHBASE_CONFIG_HOST,
                               COUCHBASE_CONFIG_USER,
                               COUCHBASE_CONFIG_PASSWD,
                               COUCHBASE_CONFIG_BUCKET);
    return $handle;
}

function make_handle_oo() {
    $oo = new Couchbase(COUCHBASE_CONFIG_HOST,
                        COUCHBASE_CONFIG_USER,
                        COUCHBASE_CONFIG_PASSWD,
                        COUCHBASE_CONFIG_BUCKET);
    return $oo;
}
