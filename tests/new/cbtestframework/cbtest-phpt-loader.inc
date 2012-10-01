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

function couchbase_phpt_skipif($clsname, $name) {
    require_once(dirname(__FILE__)."/../$clsname.inc");
    $reflector = new ReflectionClass($clsname);
    $obj = $reflector->newInstance();
    $skip_cls = "skipALL";
    $matchlist = array();

    $bname = preg_match("/^test(.+)/", $name, $matchlist);
    $skip_meth = "skip" . $matchlist[1];

    $is_skipped;

    $meths = array($skip_cls, $skip_meth);

    foreach ($meths as $meth) {
        if ($reflector->hasMethod($meth)) {
            try {
                $is_skipped = $obj->$meth();
            } catch (CBTestFramework_TestSkipException $ex) {
                die("skip " . $ex->getMessage());
            }
        }
    }
}
