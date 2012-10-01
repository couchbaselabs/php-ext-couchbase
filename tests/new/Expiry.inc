<?php
require_once 'Common.inc';
class Expiry extends CouchbaseTestCommon {

// The basic one replaces 009, the rest are new.


    /**
     * @test Expiration
     * @pre Set a key, with an expiry of 1. Get the key. Sleep two seconds,
     * get it again.
     *
     * @post First get should succeed, second should fail with @c ENOENT
     *
     * @test_plans{2.9.1}
     *
     * @group slow
     */
    function testExpirySetOO() {
        $oo = $this->oo;
        $key = $this->mk_key();

        $oo->add($key, "foo", 1);
        $this->assertEquals("foo", $oo->get($key));
        sleep(2);
        $this->assertNull($oo->get($key));
    }


    /**
     * @test Expiration (zero expiry)
     * @pre Store a kv with an expiry of @c 0. Wait two seconds, and retrieve
     * the value
     * @post Retrieval succeeds
     *
     * @test_plans{2.9.2}
     *
     * @group slow
     */
    function testExpirySetZeroOO() {
        $oo = $this->oo;
        $key = $this->mk_key();
        $oo->add($key, "foo", 0);
        sleep(2);
        $this->assertEquals("foo", $oo->get($key));
    }

    /**
     * @test Touch
     *
     * This is like @ref testExpirySetOO
     *
     * @todo Not implemented in the client
     * @warning This feature is not implemented in the client
     *
     * @group slow
     */
    function testExpiryTouch() {
        $this->markTestSkipped("Touch not implemented");
        $oo = $this->oo;
        $key = $this->mk_key();
        $oo->set($key, "foo");
        $oo->touch($key, 1);

        $this->assertEquals("foo",
                            $oo->get($key),
                            "Key exists");
        sleep(2);
        $this->assertNull($o->get($key));
    }

    /**
     * @test Touch (zero expiry)
     * This is like @ref testExpirySetZeroOO
     * @group slow
     */
    function textExpiryTouchZero() {
        $this->markTestSkipped("Touch not implemented");
        $oo = $this->oo;
        $key = $this->mk_key();
        $oo->set($key, "foo");
        $oo->touch($key, 0);
        # what happens here?
        sleep(2);
        $this->assertEquals("foo", $oo->get($key));
    }

    /**
     * test Expiry (arithmetic)
     * @todo Document this function
     * @todo Implement this!
     */
    function testArithmeticExpiry() {
        $this->markTestIncomplete("Waiting for docs");
    }
}
