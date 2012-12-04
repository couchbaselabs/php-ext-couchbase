<?php
require_once 'Common.inc';

class NegativeExpiry extends CouchbaseTestCommon {

    static $expiryRegexp = "/^Expiry must not be negative.+/";

    function skipALL() {
        parent::skipALL();
        if (!function_exists("pcntl_fork")) {
            $this->markTestSkipped("need pcntl");
        }
    }

    function _v11_feature() {
        if (!$this->atLeastVersion(array(1,1))) {
            $this->markTestSkipped("Need >= v1.1");
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
        $this->assertExceptionRegexp(
            create_function('$oo', '$oo->add("key", "value", -1);'),
            array($this->getPersistOO()),
            self::$expiryRegexp
        );
    }

    function testSet() {
        $this->assertExceptionRegexp(
            create_function('$oo','$oo->set("key", "value", -1);'),
            array($this->getPersistOO()),
            self::$expiryRegexp
        );
    }

    function testReplace() {
        $this->assertExceptionRegexp(
            create_function('$oo', '$oo->replace("key", "value", -1);'),
            array($this->getPersistOO()),
            self::$expiryRegexp
        );
    }

    function testAppend() {
        $this->assertExceptionRegexp(
            create_function('$oo', '$oo->append("key", "value", -1);'),
            array($this->getPersistOO()),
            self::$expiryRegexp
        );
    }

    function testSetMulti() {
        $kv = array("k1" => "v1", "k2" => "v2");
        $this->assertExceptionRegexp(
            create_function('$oo, $kv', '$oo->setMulti($kv, -1);'),
            array($this->getPersistOO(), $kv),
            self::$expiryRegexp
        );
    }

    function testCas() {
        $this->assertExceptionRegexp(
            create_function('$oo', '$oo->cas("123456", "key", "value", -1);'),
            array($this->getPersistOO()),
            self::$expiryRegexp
        );
    }

    function testIncrement() {
        $this->assertExceptionRegexp(
            create_function('$oo', '$oo->increment("key", 1, false, -1);'),
            array($this->getPersistOO()),
            self::$expiryRegexp
        );
    }

    function testGetAndTouch() {
        $this->skipGetAndTouch();
        $this->assertExceptionRegexp(
            create_function('$oo', '$oo->get("key",$ignored, -1);'),
            array($this->getPersistOO()),
            self::$expiryRegexp
        );
    }
    function skipGetAndTouch() { $this->_v11_feature(); }

    function testMultiGetAndTouch() {
        $this->skipMultiGetAndTouch();

        $kv = array("k1" => "v1", "k2" => "v2");

        $this->getPersistOO()->setMulti($kv);

        $this->assertExceptionRegexp(
            create_function('$oo,$kv',
                            'var_dump($oo->getMulti($kv, $casary, -1));'),

            array($this->getPersistOO(), $kv),
            self::$expiryRegexp
        );
    }
    function skipMultiGetAndTouch() { $this->_v11_feature(); }

    function testTouch() {
        $this->skipTouch();

        $this->assertExceptionRegexp(
            create_function('$oo', '$oo->touch("key", -1);'),
            array($this->getPersistOO()),
            self::$expiryRegexp
        );
    }
    function skipTouch() { $this->_v11_feature(); }

    function testTouchMulti() {
        $this->skipTouchMulti();
        
        $kv = array("k1" => "v1", "k2" => "v2");
        $this->assertExceptionRegexp(
            create_function('$oo, $kv', '$oo->touchMulti($kv, -1);'),
            array($this->getPersistOO(), $kv),
            self::$expiryRegexp
        );
    }
    function skipTouchMulti() { $this->_v11_feature(); }
}
