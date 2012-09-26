<?php
require_once "Common.inc";


class NonASCII extends CouchbaseTestCommon {

    /**
     * @test
     * @pre Set a key and values both containing embedded hebrew literals
     * @post operation succeeds. Retrieved value is the same as the original
     * @test_plans{2.1.1, 2.1.2, 3.3, 3.4, 2.10.1}
     */
    function test8BitSafeKey() {
        $k = $this->mk_key();
        $k = "שלום".$k;
        $v = "קוראים לי מרדכי";
        $oo = $this->getPersistOO();

        $rv = $oo->set($k, $v);

        $this->assertNotEmpty($rv,
                              "Can store non-ascii keys");

        $ret = $oo->get($k);
        $this->assertNotEmpty($ret,
                              "Can retrieve non-ascii key");
        $this->assertEquals($v, $ret, "Got back our value");
    }

    function testEmbeddedNul() {
        $k = $this->mk_key();
        $k = "blah\0$k";
        $v = $k;

        $oo = $this->getPersistOO();
        $rv = $oo->set($k, $v);

        $this->assertNotEmpty($rv);
        $ret = $oo->get($k);
        $this->assertNotEmpty($ret);
        $this->assertEquals($v, $ret);
    }

    /**
     * @test
     * @pre set a key and value. both key and value have embedded NULs
     * @post operation succeeds. Retrieved value is empty and matches original
     * value
     *
     * @test_plans{2.1.4, 3.6}
     */
    function testZeroLengthValue() {
        $k = $this->mk_key();
        $v = "";
        $oo = $this->getPersistOO();
        $rv = $oo->set($k, $v);
        $this->assertNotEmpty($rv);

        $ret = $oo->get($k);
        $this->assertEmpty($ret);
    }
}

?>
