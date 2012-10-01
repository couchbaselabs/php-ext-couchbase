<?php
require_once 'Common.inc';

class NegativeExpiry extends CouchbaseTestCommon {

    static $expiryRegexp = "/Expiry must not be negative/";

    function skipALL() {
        parent::skipALL();
        if (!function_exists("pcntl_fork")) {
            $this->markTestSkipped("need pcntl");
        }
    }

    /**
     * @test Test negative expiry
     * @pre Set a key with a negative expiry
     * @post Error is raised
     * @test_plans{2.9.3}
     * @bug PCBC-117
     */
    function testAdd() {
        $this->assertFatalRegexp(
            create_function('$oo', '$oo->add("key", "value", -1);'),
            array($this->getPersistOO()),
            self::$expiryRegexp
        );
    }

    function testSet() {
        $this->assertFatalRegexp(
            create_function('$oo','$oo->set("key", "value", -1);'),
            array($this->getPersistOO()),
            self::$expiryRegexp
        );
    }

    function testReplace() {
        $this->assertFatalRegexp(
            create_function('$oo', '$oo->replace("key", "value", -1);'),
            array($this->getPersistOO()),
            self::$expiryRegexp
        );
    }

    function testAppend() {
        $this->assertFatalRegexp(
            create_function('$oo', '$oo->append("key", "value", -1);'),
            array($this->getPersistOO()),
            self::$expiryRegexp
        );
    }

    function testSetMulti() {
        $kv = array("k1" => "v1", "k2" => "v2");
        $this->assertFatalRegexp(
            create_function('$oo, $kv', '$oo->setMulti($kv, -1);'),
            array($this->getPersistOO(), $kv),
            self::$expiryRegexp
        );
    }

    function testCas() {
        $this->assertFatalRegexp(
            create_function('$oo', '$oo->cas("123456", "key", "value", -1);'),
            array($this->getPersistOO()),
            self::$expiryRegexp
        );
    }

    function testIncrement() {
        $this->assertFatalRegexp(
            create_function('$oo', '$oo->increment("key", 1, false, -1);'),
            array($this->getPersistOO()),
            self::$expiryRegexp
        );
    }

    function testGetAndTouch() {
        $this->assertFatalRegexp(
            create_function('$oo', '$oo->get("key", NULL, $ignored, -1);'),
            array($this->getPersistOO()),
            self::$expiryRegexp
        );
    }

    function testMultiGetAndTouch() {
        $kv = array("k1" => "v1", "k2" => "v2");

        $this->getPersistOO()->setMulti($kv);

        $this->assertFatalRegexp(
            create_function('$oo,$kv',
                            'var_dump($oo->getMulti($kv, $casary, NULL, -1));'),

            array($this->getPersistOO(), $kv),
            self::$expiryRegexp
        );
    }

    function testTouch() {
        $this->assertFatalRegexp(
            create_function('$oo', '$oo->touch("key", -1);'),
            array($this->getPersistOO()),
            self::$expiryRegexp
        );
    }

    function testTouchMulti() {
        $kv = array("k1" => "v1", "k2" => "v2");
        $this->assertFatalRegexp(
            create_function('$oo, $kv', '$oo->touchMulti($kv, -1);'),
            array($this->getPersistOO(), $kv),
            self::$expiryRegexp
        );
    }
}
