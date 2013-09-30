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

    function testPCBC_251() {
       $handle = $this->handle;
       couchbase_set_option($handle, Couchbase::OPT_PREFIX_KEY, "PCBC-251");
       couchbase_delete($handle, "counter");
       couchbase_add($handle, "counter", 2);
       $this->assertEquals(1, couchbase_decrement($handle, "counter"));
       $this->assertEquals(0, couchbase_decrement($handle, "counter"));
       $lock = 0;
       $this->assertEquals(0, couchbase_get_and_lock($handle, "counter", $lock, 1));
       $this->assertTrue(couchbase_unlock($handle, "counter", $lock));
       # decr 0 -> 0
       $this->assertEquals(0, couchbase_decrement($handle, "counter"));
       couchbase_set_option($handle, Couchbase::OPT_PREFIX_KEY, "");
    }

    function testPCBC_251_oo() {
       $oo = $this->oo;
       $oo->setOption(Couchbase::OPT_PREFIX_KEY, "PCBC-251");
       $oo->delete("counter");
       $oo->add("counter", 2);
       $this->assertEquals(1, $oo->decrement("counter"));
       $this->assertEquals(0, $oo->decrement("counter"));
       $lock = 0;
       $this->assertEquals(0, $oo->getAndLock("counter", $lock, 1));
       $this->assertTrue($oo->unlock("counter", $lock));

       # decr 0 -> 0
       $this->assertEquals(0, $oo->decrement("counter"));
       $oo->setOption(Couchbase::OPT_PREFIX_KEY, "");
    }

}
