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

    function testPCBC_219() {
         $cb = $this->oo;
         $msg = "";
         try {
             $value = 50.00;
             $cb->set("pcbc-219", $value);
	     $this->assertEquals(50.00, floatval($cb->get("pcbc-219")));
	     $this->assertEquals(50.00, $cb->get("pcbc-219"));
         } catch (Exception $exp) {
             $msg = $exp->getMessage();
         }
         $this->assertEquals("", $msg);
    }

    function testPCBC_227() {
       $oo = $this->oo;
       $msg = "";


       try {
           couchbase_set($this->handle, "1234", "foo");
       } catch (Exception $exp) {
           $msg = $exp->getMessage();
       }
       $this->assertEquals("", $msg);

       try {
           couchbase_set($this->handle, 1234, "foo");
       } catch (Exception $exp) {
           $msg = $exp->getMessage();
       }
       $this->assertEquals("", $msg);

       try {
           couchbase_set_multi($this->handle, array("1234" =>  "foo"));
       } catch (Exception $exp) {
           $msg = $exp->getMessage();
       }
       $this->assertEquals("", $msg);

       try {
           couchbase_set_multi($this->handle, array(1234 =>  "foo"));
       } catch (Exception $exp) {
           $msg = $exp->getMessage();
       }
       $this->assertEquals("", $msg);

       try {
           $oo->set("1234", "foo");
       } catch (Exception $exp) {
           $msg = $exp->getMessage();
       }
       $this->assertEquals("", $msg);

       try {
           $oo->set(1234, "foo");
       } catch (Exception $exp) {
           $msg = $exp->getMessage();
       }
       $this->assertEquals("", $msg);

       try {
           $oo->setMulti(array("1234" =>  "foo"));
       } catch (Exception $exp) {
           $msg = $exp->getMessage();
       }
       $this->assertEquals("", $msg);

       try {
           $oo->setMulti(array(1234 =>  "foo"));
       } catch (Exception $exp) {
           $msg = $exp->getMessage();
       }
       $this->assertEquals("", $msg);
    }
}
