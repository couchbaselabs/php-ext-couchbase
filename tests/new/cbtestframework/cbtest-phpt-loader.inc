<?php

require_once 'CBTestFramework.inc';
require_once dirname(__FILE__).'/../Common.inc';

function couchbase_phpt_runtest($clsname, $name) {
    require_once(dirname(__FILE__)."/../$clsname.inc");
    $reflector = new ReflectionClass($clsname);
    $obj = $reflector->newInstance();

    # If any assertions fail here, PHPUnitCompat will fail the test
    # and complain.
    $obj->runSingleTest($name);
    printf("PHP_COUCHBASE_OK\n");
}
