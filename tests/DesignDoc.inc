<?php

require_once 'Common.inc';

class DesignDoc extends CouchbaseTestCommon {

    function testGetSetDeleteDesignDoc() {
        $h = $this->handle;
        $name = $this->mk_key();

        $d = '{"views":{"myview":{"map":"function (doc, meta) { emit(meta.id, NULL); }"}}}';
        $ret = couchbase_set_design_doc($h, $name, $d);
        $this->assertTrue($ret);

        # Get it back and ensure that it's the same..
        $ret = couchbase_get_design_doc($h, $name);
        $this->assertEquals($ret, $d);

        $ret = couchbase_delete_design_doc($h, $name);
        $this->assertTrue($ret);
   }

    function testGetSetDeleteDesignDocOO() {
        $cb = $this->oo;
        $name = $this->mk_key();

        $d = '{"views":{"myview":{"map":"function (doc, meta) { emit(meta.id, NULL); }"}}}';
        $ret = $cb->setDesignDoc($name, $d);
        $this->assertTrue($ret);

        # Get it back and ensure that it's the same..
        $ret = $cb->getDesignDoc($name);
        $this->assertEquals($ret, $d);

        $ret = $cb->deleteDesignDoc($name);
        $this->assertTrue($ret);
   }

   /* The implementation of the logic of the get/set/delete functions
    * is the same for the OO and the functional interface, so there
    * isn't any point of running all of the "error detection" tests
    * for both variants. Given that the OO variant throws exceptions
    * its more pleasant to work with
    */

    function testDeleteNonexistingDesignDoc() {
        $cb = $this->oo;
        $name = $this->mk_key();
        $ret = $cb->deleteDesignDoc($name);
        // This should fail, but it's not due to http://www.couchbase.com/issues/browse/MB-7408
        // $this->assertFalse($ret);
        // $this->assertFalse($ret);
    }

    function testGetNonexistingDesignDoc() {
        $cb = $this->oo;
        $name = $this->mk_key();
        $cb->deleteDesignDoc($name);
        $ddoc = $cb->getDesignDoc($name);
        $this->assertFalse($ddoc);
    }

   function testSetNoDocumentName() {
        try {
            $this->oo->setDesignDoc("", "");
            die("setDesignDoc didn't detect empty design document name");
        } catch (CouchbaseException $e) {

        }
   }

   function testSetNoDocumentData() {
        try {
            $name = $this->mk_key();
            $this->oo->setDesignDoc($name, "");
            die("setDesignDoc didn't detect empty design document");
        } catch (CouchbaseException $e) {

        }
   }

   function testGetNoDocumentName() {
        try {
            $this->oo->getDesignDoc("");
            die("getDesignDoc didn't detect empty design document name");
        } catch (CouchbaseException $e) {

        }
   }

   function testDeleteNoDocumentName() {
        try {
            $this->oo->deleteDesignDoc("");
            die("deleteDesignDoc didn't detect empty design document name");
        } catch (CouchbaseException $e) {

        }
   }

   function testSetInvalidJsonOO() {
       $cb = $this->oo;
       $name = $this->mk_key();

       try {
           $ret = $cb->setDesignDoc($name, "What's up gangsta!");
           die("setDesignDoc didn't detect bad JSON");
       } catch (CouchbaseServerException $e) {
           $this->assertContains("invalid UTF-8 JSON", $e->getMessage());
       }
   }
}
