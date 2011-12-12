<?php
if (!extension_loaded("couchbase"))  {
    print("skip");
    return;
}

include "couchbase.inc";
if(!couchbase_connect(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET)) {
   print("server couldn't be connected");
   return;
}
