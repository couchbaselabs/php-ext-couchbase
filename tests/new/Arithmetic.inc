<?php

/* This file replaces 010 and 022 */

require_once 'Common.inc';
class Arithmetic extends CouchbaseTestCommon
{
    /**
     * @test
     * Simple Increment and Decrement
     *
     * @pre
     * Set a key to the string value "2", fetch it with GET. Increment it
     * by one using increment(). Get, get the key using get(). Decrement the
     * Key, Get the key again
     *
     * @post
     * Return value is an integer type, and value is now 3.
     * Get returns the string "3". Decrement returns an integer 2, get
     * returns a string "2"
     *
     * @test_plans{5.3, 5.4}
    */
    function testIncrDecrOO() {
        $oo = $this->getPersistOO();
        $key = $this->mk_key();
        $value = "2";

        $oo->add($key, $value);
        $rv = $oo->increment($key);
        $this->assertInternalType('int', $rv,
                                  "Incr return is numeric");
        $this->assertEquals(3, $rv);

        $rv = $oo->get($key);
        $this->assertInternalType('string', $rv,
                                  "Get return is string");
        $this->assertEquals('3', $rv);

        $rv = $oo->decrement($key);
        $this->assertInternalType('int', $rv,
                                  "Decr return is numeric");
        $this->assertEquals(2, $rv);

        $rv = $oo->get($key);
        $this->assertInternalType('string', $rv);
        $this->assertEquals('2', $rv);
    }

    function testIncrDecr() {
        $h = $this->getPersistHandle();
        $key = $this->mk_key();
        $value = "2";
        couchbase_add($h, $key, $value);
        $rv = couchbase_increment($h, $key);
        $this->assertInternalType('int', $rv);
        $this->assertEquals(3, $rv);

        $rv = couchbase_get($h, $key);
        $this->assertInternalType('string', $rv);
        $this->assertEquals('3', $rv);

        $rv = couchbase_decrement($h, $key);
        $this->assertInternalType('int', $rv);
        $this->assertEquals(2, $rv);

        $rv = couchbase_get($h, $key);
        $this->assertInternalType('string', $rv);
        $this->assertEquals('2', $rv);
    }


    /**
     * @test
     * Increment a string value
     *
     * @pre
     * Set a key to the value "string", and attempt to increment it
     *
     * @post
     * Error message indicating stored value is not numeric
     * @test_plans{5.4}
     */
    function testIncrStringOO() {
        $key = $this->mk_key();
        $oo = $this->getPersistOO();
        $oo->set($key, "String");

        $msg = NULL;
        try {
            $rv = $oo->increment($key);
        } catch (Exception $exc) {
            $msg = $exc->getMessage();
        }

        $this->assertNotNull($msg, "Got exception for incrementing a string");
        $this->assertContains("Not a number", $msg);
    }

    function testIncrString() {
        $key = $this->mk_key();
        $h = $this->getPersistHandle();
        couchbase_set($h, $key, 'String');

        $msg = NULL;
        try {
            $rv = couchbase_increment($h, $key);
        } catch (Exception $exc) {
            $msg = $exc->getMessage();
        }
        $this->assertNotNull($msg);
        $this->assertContains('Not a number', $msg);
    }

    /**
     * @test
     * Arithmetic (Non-Exist, Initial)
     *
     * @pre
     * Increment a key with an initial value of 2
     *
     * @post
     * Return value is 2.
     *
     * @post
     * GET on value yields the string @c '2'
     *
     * @test
     * Arithmetic (Non-Exist, No Initial)
     *
     * @pre
     * Decrement a key by 2, with no initial value
     *
     * @post
     * error message indicating ENOENT
     * @test_plans{5.1, 5.2, 5.7}
     */
    function testIncrDecrNonexistOO() {
        $key = $this->mk_key();
        $oo = $this->getPersistOO();

        $rv = $oo->increment($key,
                             $offset = 1,
                             $create = true,
                             $expire = NULL,
                             $initial_value = 2);

        $this->assertEquals(2, $rv,
                            "Value is set to the default rather than the offset");
        $rv = $oo->get($key);
        $this->assertEquals('2', $rv);

        $oo->delete($key);

        $rv = $oo->decrement($key, 2);
        $this->assertFalse($rv);
    }

    function testIncrDecrNonexist() {
        $key = $this->mk_key();
        $h = $this->getPersistHandle();
        $rv = couchbase_increment($h,
                                  $key,
                                  $offset = 1,
                                  $create = true,
                                  $expire = NULL,
                                  $initial_value = 2);
        $this->assertEquals(2, $rv);

        $rv = couchbase_get($h, $key);
        $this->assertEquals('2', $rv);

        couchbase_delete($h, $key);

        $msg = NULL;
        $rv = couchbase_decrement($h, $key, 2);
        $this->assertFalse($rv);
    }

    function testIncrDecrNonexistPositionalOO() {
        $key = $this->mk_key();
        $oo = $this->getPersistOO();
        $rv = $oo->increment($key, 20, 1, 0, 2);
        $this->assertEquals(2, $rv, "Set to initial value (incr)");

        $oo->delete($key);
        $rv = $oo->decrement($key, 20, 1, 0, 2);
        $this->assertEquals(2, $rv, "Set to initial value (Decr)");
    }

    function testIncrDecrNonexistPositional() {
        $key = $this->mk_key();
        $h = $this->getPersistHandle();
        $rv = couchbase_increment($h, $key, 20, 1, 0, 2);
        $this->assertEquals(2, $rv);

        couchbase_delete($h, $key);

        $rv = couchbase_decrement($h, $key, 20, 1, 0, 2);
        $this->assertEquals(2, $rv);
    }

    /**
     * @test Incr/Decr with custom offsets
     *
     * @pre Set a key to arithmetic 0, Call @c increment with an offset of @c 666
     * @post Return value from increment is @c 666
     *
     * @pre Decrement the key using the offset @c 333
     * @post Key is now @c 333 (return value)
     *
     * @pre Get the key via @c get
     * @post Key is now @c '333'
     *
     * @test_plans{5.5}
     */
    function testCustomOffset() {
        $key = $this->mk_key();
        $oo = $this->getPersistOO();

        $rv = $oo->increment($key, 0, true, NULL, 0);
        $this->assertEquals(0, $rv);

        $rv = $oo->increment($key, 666);
        $this->assertEquals($rv, 666);

        $rv = $oo->decrement($key, 333);
        $this->assertEquals(333, $rv);

        $this->assertEquals('333', $oo->get($key));
    }

    /**
     * @test Arithmetic with expiration
     * @pre Set a key to the value 100 (via arithmetic) with an expiry of 1.
     * @post get the key, should be @c '100'
     *
     * @pre Wait two seconds, then get the key again
     * @post Return value should be @c NULL
     *
     * @group slow
     */
    function testExpiry() {
        $k = $this->mk_key();
        $oo = $this->getPersistOO();

        $rv = $oo->increment($k,
                             $offset = 0,
                             $create = true,
                             $expire = 1,
                             $initial_value = 100);

        $this->assertEquals(100, $rv);

        $this->assertEquals('100', $oo->get($k));
        sleep(2);
        $this->assertNull($oo->get($k));
        $this->assertEquals(COUCHBASE_KEY_ENOENT,
                            $oo->getResultCode());
    }

}

?>
