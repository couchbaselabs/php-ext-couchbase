<?php
require_once 'Common.inc';

class IntegerArgs extends CouchbaseTestCommon {

    // 015

    /**
     * @test
     * Integer Conversion (Keys)
     *
     * @pre
     * Create an integer key 88888. (ensure it is removed first).
     * Get the key. Replace the key, Add the key, Delete the key,
     * Append to the key, set the key to "foo", and retrieve the key
     *
     * @post
     * Get => KEY_ENOENT, Replace => KEY_ENOENT, Add => Success,
     * Delete => Success, Append => NOT_STORED, Set => Success.
     * Final get on the string "88888" yields the value set
     *
     */
    function testIntKeyArgs() {
        $key = 888888;
        $oo = $this->oo;
        $oo->delete($key);

        $oo->get($key);
        $this->assertEquals(COUCHBASE_KEY_ENOENT,
                            $oo->getResultCode());

        $oo->replace($key, "foo");
        $this->assertEquals(COUCHBASE_KEY_ENOENT,
                            $oo->getResultCode());

        $oo->add($key, "foo");
        $this->assertEquals(COUCHBASE_SUCCESS,
                            $oo->getResultCode());

        $oo->delete($key);
        $this->assertEquals(COUCHBASE_SUCCESS,
                            $oo->getResultCode());

        $oo->append($key, "_suffix");
        $this->assertEquals(COUCHBASE_NOT_STORED,
                            $oo->getResultCode());

        $cas = $oo->set($key, "some_value");
        $this->assertNotEmpty($cas);
        $ret = $oo->get("$key");
        $this->assertEquals("some_value", $ret);

    }

    // 029
    /**
     * @test Integer Conversion (Values)
     *
     * @pre Create a key, and store the number 642349292 as the value.
     * Retrieve the key
     *
     * @post Set operation succeeds.
     * Get returns an integer type with the value 642349292
     */
    function testIntValueArgs() {
        $key = $this->mk_key();
        $oo = $this->oo;

        $oo->set($key, 642349292);
        $val = $oo->get($key);
        $this->assertInternalType('int', $val);
        $this->assertEquals(642349292, $val);

        # Smaller value? sure..
        $oo->delete($key);
        $oo->set($key, 5);
        $val = $oo->get($key);
        $this->assertInternalType('int', $val);
        $this->assertEquals(5, $val);
    }
}

?>
