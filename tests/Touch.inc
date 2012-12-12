<?php

require_once 'Common.inc';

class Touch extends CouchbaseTestCommon {
    function testTouch() {
        $h = $this->handle;
        $key = $this->mk_key();
        couchbase_set($h, $key, "foobar");
        $cas = couchbase_touch($h, $key, 1);
        $this->assertTrue(is_string($cas));
    }

    function testMulti() {
        $h = $this->handle;
        $key1 = $this->mk_key();
        couchbase_set($h, $key1, "foobar");
        $key2 = $this->mk_key();
        couchbase_set($h, $key2, "foobar");
        $keys = array($key1, $key2);
        $arr = couchbase_touch_multi($h, $keys, 1);
        $this->assertTrue(is_array($arr));
        $this->assertTrue(is_string($arr[$key1]));
        $this->assertTrue(is_string($arr[$key2]));
    }

    function testTouchOO() {
        $oo = $this->oo;
        $key = $this->mk_key();
        $oo->set($key, "foobar");
        $cas = $oo->touch($key, 1);
        $this->assertTrue(is_string($cas));
    }

    function testTouchNoKeyOO() {
        try {
            $oo = $this->oo;
            $oo->touch("", 1);
            die("Touch should throw exception\n");
        } catch (CouchbaseException $e) {
        }
    }
    function testTouchMultiOO() {
        $oo = $this->oo;
        $key1 = $this->mk_key();
        $key2 = $this->mk_key();
        $oo->set($key1, "foobar");
        $oo->set($key2, "foobar");
        $keys = array($key1, $key2);
        $arr = $oo->touchMulti($keys, 1);
        $this->assertTrue(is_array($arr));
        $this->assertTrue(is_string($arr[$key1]));
        $this->assertTrue(is_string($arr[$key2]));
    }

    function testTouchMultiNoKeysOO() {
        try {
            $oo = $this->oo;
            $oo->touchMulti(array(), 1);
            die("Touch should throw exception\n");
        } catch (CouchbaseException $e) {
        }
    }
}
