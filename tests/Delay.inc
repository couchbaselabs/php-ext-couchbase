<?php
require_once 'Common.inc';

$values = array();

function global_content_cb($myh, $val) {
    global $values;
    array_push($values, $val);
}

class Delay extends CouchbaseTestCommon {

    /**
     * In each of the functions below we retrieve a resultset (either at once
     * or incrementally), and place it inside the 'values' array. This common
     * test checks to see if:
     * 1) All keys in the values array are present within both the $casrets (the
     * array returned by set) and in the $keys array (which is the original data
     * source). If the request also gives us the CAS, then we ensure the CAS indeed
     * matches.
     */

    // Need to unmark this as a test, (we should alias this),
    // otherwise PHPUnit will try to run it..

    /**
     * @_test
     * Delay (Common)
     *
     * @pre
     * Receive an array of key-value pairs ($keys), an array of key-cas pairs
     * ($casvals), and a boolean of whether to verify the cas. Also an
     * externally defined global array $values exists, which is populated
     * by a get operation invoked by the caller. It contains an array of
     * arrays which contain the key, value, and optionally cas. i.e.
     * <tt>[{"key":"first_key"},{"key":"second_key"}]</tt>
     *
     * @post
     * None of the keys ($keys) are empty. Each element of the $values
     * array has its key and value set as non-empty. If the boolean flag
     * to check the cas is true, then also ensure that the cas in the element
     * matches the cas in <tt>$casvals[$key]</tt>
     */
    private function _xref_assert_common($casvals,
                                 $keys,
                                 $check_cas = false) {
        global $values;
        $this->assertCount(10, $values);

        foreach (array_values($values) as $val) {
            $k = $val['key'];
            $v = $val['value'];

            $this->assertNotEmpty($k);
            $this->assertNotEmpty($v);

            $this->assertArrayHasKey($k, $keys);
            $this->assertArrayHasKey($k, $casvals);

            $this->assertEquals($keys[$k], $v);

            if ($check_cas) {
                $c = $val['cas'];
                $this->assertNotEmpty($c);
                $this->assertEquals($casvals[$k], $c);
            }
        }
    }

    /**
     * @test Get Delayed Callback
     * @pre
     * Set 10 kv pairs using set(). Store the cas returns in $casvals.
     * Set up a callback which pushes each argument it receives into the
     * global $values array. Invoke get_delayed on the keys of the kv pair set.
     *
     * @post
     * @ref _xref_assert_common
     *
     * @remark
     * Variants ( anonymous @ref testAnonCb )
     */
    function testContentCb() {

        $h = $this->getPersistHandle();
        global $values;

        $keys = $this->makeKvPairs(10);

        $casvals = couchbase_set_multi($h, $keys);

        foreach ($keys as $k => $v) {
            $this->assertArrayHasKey($k, $casvals);
        }

        $this->assertInternalType('array', $casvals);
        $values = array();
        couchbase_get_delayed($h, array_keys($keys), false, "global_content_cb");
        $this->_xref_assert_common($casvals, $keys, false);

        $values = array();
        couchbase_get_delayed($h, array_keys($keys), true, "global_content_cb");
        $this->_xref_assert_common($casvals, $keys, true);

    }

    function testAnonCb() {
        global $values;
        $keys = $this->makeKvPairs(10);
        $h = $this->getPersistHandle();

        $values = array();
        $casvals = couchbase_set_multi($h, $keys);

        couchbase_get_delayed($h,
                              array_keys($keys),
                              false,
                              function($myh, $val) {
                                global $values;
                                array_push($values, $val);
                              });
        $this->_xref_assert_common($casvals, $keys, false);
    }

    /**
     * @test Get Delayed (Invalid)
     * @pre Create 10 kv pairs, set them with set, and invoke get_delayed with
     * a non-existing callback
     *
     * @post Error Message about invalid callback
     */
    function testInvalidCb() {
        global $values;
        $keys = $this->makeKvPairs(10);
        $h = $this->getPersistHandle();
        $values = array();
        $casvals = couchbase_set_multi($h, $keys);

        $msg = NULL;
        try {
            couchbase_get_delayed($h, array_keys($keys), false,
                                  "non-exist-callback");
        } catch (Exception $exc) {
            $msg = $exc->getMessage();
        }

        $this->assertNotNull($msg);
        $this->assertContains('invalid function', $msg);
    }

    # 020
    /**
     * @test Get Delayed (fetch all)
     *
     * @pre
     * Create 10 kv pairs, set them (keeping note of their cas values,
     * as in previous tests). Invoke get_delayed without any callback
     * parameter. Set the global $values array to the return value of
     * fetch_all()
     *
     * @post @ref _xref_assert_common
     */
    function testFetchAll() {
        $h = $this->getPersistHandle();
        $keys = $this->makeKvPairs(10);
        $casrets = couchbase_set_multi($h, $keys);
        $rv = couchbase_get_delayed($h, array_keys($keys), false);

        global $values;
        $values = couchbase_fetch_all($h);
        $this->_xref_assert_common($casrets, $keys, false);

        $rv = couchbase_fetch_all($h);
        $this->assertFalse($rv);

        $values = array();
        $rv = couchbase_get_delayed($h, array_keys($keys), true);
        $values = couchbase_fetch_all($h);
        $this->_xref_assert_common($casrets, $keys, true);
        $rv = couchbase_fetch_all($h);
        $this->assertFalse($rv);
    }

    # 021
    /**
     * @test Get Delayed (fetch one)
     * @pre
     * Like @ref testFetchAll but instead of setting the $values to the return
     * value of fetch_all, fetch_one is run in a loop (until it returns
     * false), and each iteration of fetch_one returns an item which is pushed
     * into the $values array
     *
     * @post @ref _xref_assert_common
     */
    function testFetchOne() {
        global $values;
        $h = $this->getPersistHandle();
        $keys = $this->makeKvPairs(10);

        $casrets = couchbase_set_multi($h, $keys);

        $rv = couchbase_get_delayed($h, array_keys($keys), false);
        $this->assertTrue($rv);

		if (!is_array($values)) { $values = array(); }
        while ($row = couchbase_fetch($h)) {
            array_push($values, $row);
        }

        $this->_xref_assert_common($casrets, $keys, false);

        $values = array();
        $rv = couchbase_get_delayed($h, array_keys($keys), true);
        $this->assertTrue($rv);

        while ($row = couchbase_fetch($h)) {
            array_push($values, $row);
        }

        $this->_xref_assert_common($casrets, $keys, true);
    }

}
