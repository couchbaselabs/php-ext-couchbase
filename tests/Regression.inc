<?php
require_once 'Common.inc';

class Regression extends CouchbaseTestCommon {

    function testPCBC_191() {
        $cb = $this->oo;
        $docs = array();
        for ($x = 1; $x <= 1000; $x++) {//$x = 1-1000
            $docs[]['TEST_DOC::' . $x] = '{"a" : 1}';
        }

        $msg = "not thrown";
        try {
	   $cb->setMulti($docs);
        } catch (CouchbaseIllegalKeyException $e) {
           $msg = $e->getMessage();
        } catch (Exception $exp) {
           $msg = $exp->getMessage();
        }
        $this->assertEquals("Invalid key specified (not a string)",
                            $msg);
    }

    function testPCBC_218() {
        $cb = $this->oo;
        $msg = "not thrown";
        try {
           $cb->cas("yay", "foo", "bar", 0);
        } catch (Exception $exp) {
           $msg = $exp->getMessage();
        }

        $this->assertEquals("Illegal CAS specified", $msg);
    }

}
