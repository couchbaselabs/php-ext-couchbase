#!/usr/bin/php

<?php

$OUTPUT = "phpt";

$lines = file("TEST_CLASSES");

class CBTestFramework { }

system("rm -rf $OUTPUT");
mkdir($OUTPUT);

foreach ($lines as $line) {
    $line = chop($line);
    require_once($line . ".inc");
    $clsname = $line;
    $reflector = new ReflectionClass($clsname);
    $methods = $reflector->getMethods();

    $clspath = "$OUTPUT/$clsname";
    mkdir($clspath);

    foreach ($methods as $meth) {
        $name = $meth->getName();
        $matches = array();
        if (! preg_match("/^test(.+)/", $name, $matches)) {
            continue;
        }

        $test_base = $matches[1];

        $fname = "$clspath/$test_base.phpt";
        $fp = fopen($fname, "w");


        // Boilerplate:

        fprintf($fp, <<<EOF
--TEST--
$clsname - $test_base
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("$clsname", "$name");
--EXPECT--
PHP_COUCHBASE_OK
EOF
);
    }
}
