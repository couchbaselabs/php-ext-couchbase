<?php
require_once 'Common.inc';

class GetMulti extends CouchbaseTestCommon
{

    public $iteration_count = 10;
    /**
     * Make key-value pairs and return them as an array. The keys
     * are guaranteed to have been removed
     *
     * @param $count the amount of key value pairs to generate
     */
    function makeKvPairs($count = -1) {
        $ret = array();

        if ($count == -1) {
            $count = $this->iteration_count;
        }

        for ($ii = 0; $ii < $count; $ii++) {
            $k = $this->mk_key();
            $v = uniqid("couchbase_value_");
            $ret[$k] = $v;
        }
        return $ret;
    }

    /**
     * @test MultiGet (Common)
     *
     * Common test for multi-get operations
     *
     * @pre
     * Create 10 key-value pairs as an array @c $keys.
     * Set the values and place their CASes into an array @c $cas.
     * Retrieve the values using @c get(), storing the values into an
     * array @c $values, and the cas values into an array @c $cas1
     *
     * @post
     * Count of all four arrays are identical.
     * The @c $keys and @c $values array are identical.
     * The @c $cas and @c $cas1 arrays are identical
     *
     * @remark
     * Variants: OO
     *
     * @test_plans{3.1}
     */

    function _test_common_oo() {
        $oo = $this->oo;
        $keys = array();
        $cas = array();
        $keys = $this->makeKvPairs();

        foreach ($keys as $k => $v) {
            $cas[$k] = $oo->set($k, $v);
        }

        $values = $oo->getMulti(array_keys($keys), $cas1);


        sort($values);
        sort($keys);
        sort($cas1);
        sort($cas);

        $this->assertEquals(count($values), count($keys));
        $this->assertEquals(serialize($values), serialize($keys));
        $this->assertEquals(serialize($cas), serialize($cas1));

        $k = array_keys($keys);
        $this->assertEquals($values[$k[0]], $keys[$k[0]]);
    }

    /**
     * @see _test_common_oo
     */
    function _test_common() {
        $h = $this->handle;
        $keys = array();
        $cas = array();

        $keys = $this->makeKvPairs();

        foreach ($keys as $k => $v) {
            $cas[$k] = couchbase_set($h, $k, $v);
        }
        $values = couchbase_get_multi($h, array_keys($keys), $cas1);
        sort($values);
        sort($keys);
        sort($cas1);
        sort($cas);

        $this->assertEquals(count($values), count($keys));
        $this->assertEquals(serialize($values), serialize($keys));
        $this->assertEquals(serialize($cas), serialize($cas1));
    }

    # Replaces 007
    /**
     * @test
     * MultiGet (plain)
     *
     * @pre
     * Create a handle and set a key prefix. Run the @ref _test_common.
     * Remove the prefix, run the @ref _test_common again.
     *
     * @post common test succeeds
     *
     * @remark
     * Variants: OO
     *
     * @test_plans{4.3}
     */
    function testPlainOO() {
        $this->_test_common_oo();
        #prefixed:
        $this->oo->setOption(COUCHBASE_OPT_PREFIX_KEY, "foo_");
        $this->_test_common_oo();
        $this->oo->setOption(COUCHBASE_OPT_PREFIX_KEY, '');
    }

    function testPlain() {
        $this->_test_common();
    }

    # test setMulti on its own, replaces 017

    /**
     * @test
     * MultiSet
     *
     * @pre
     * Make 10 key value pairs as an array.
     * Pass this array to @c set_multi.
     * Store the returned cas values as @c $casrets
     *
     * @post
     * Returned CAS array contains all keys from the initial KV array.
     * All CAS values are value
     *
     * @bug PCBC-66
     * @bug PCBC-59
     *
     * @remark
     * Variants: OO
     *
     * @test_plans{4.1}
     */
    function testSetMulti() {
        $h = $this->handle;
        $values = $this->makeKvPairs();

        $casrets = couchbase_set_multi($h, $values, 1);
        $this->assertCount($this->iteration_count, $casrets);

        foreach ($casrets as $k => $cas) {
            $this->assertArrayHasKey($k, $values);
        }

        foreach ($values as $k => $val) {
            $this->assertArrayHasKey($k, $casrets);
            couchbase_delete($h, $k);
        }
    }

    function testSetMultiOO() {
        $oo = $this->oo;
        $values = $this->makeKvPairs();
        $casrets = $this->oo->setMulti($values, 1);

        $this->assertCount($this->iteration_count, $casrets);


        # Cross-checking the arrays, PCBC-66
        foreach ($casrets as $k => $cas) {
            $this->assertArrayHasKey($k, $values);
        }

        foreach ($values as $k => $val) {
            $this->assertArrayHasKey($k, $casrets);
            $oo->delete($k);
        }
    }

    # 032
    /**
     * @test MultiGet (Ordered)
     *
     * @pre
     * Create 10 key-value pairs as an array (@c $values ).
     * Store the key values, and save their cas values as an array @c $casrets.
     * Get the keys with the @c GET_PRESERVE_ORDER option specified.
     * Storing the returned values in @c $res and the returned CASes in @c $cas
     *
     * @post
     * casrets and @c $cas are identical. @c $res and @c $values are identical
     *
     * @bug PCBC-67
     *
     * @test_plans{4.3, 3.1}
     */
    function testMgetOrdered() {
        # ensure keys are in order, if requested..

        $oo = $this->getPersistOO();
        $values = $this->makeKvPairs();
        $casrets = $oo->setMulti($values, 1);
        asort($values);

        $keys = array_keys($values);
        $res = $oo->getMulti($keys, $cas, Couchbase::GET_PRESERVE_ORDER);
        $this->assertEquals(serialize($values), serialize($res));
    }


    private function _ne_key_for_value($k) {
        return "value_for_$k";
    }

    /**
     * @test Test partial successes on multi get
     *
     * @pre
     * Generate key value pairs, set half of them via @c set. Perform
     * a @c getMulti on all the keys. Store the @c $existing keys in a separate
     * array.
     *
     * @post
     *
     * All keys in the @c $existing array are accounted for in the @c getMulti
     * return (with their values set)
     */
    function testMgetPartial() {
        $oo = $this->getPersistOO();
        $nkeys = $this->iteration_count;

        $existing = array();
        $keys = array();

        for ($ii = 0; $ii < $nkeys; $ii++) {
            $k = $this->mk_key();
            if ($ii % 2) {
                $v = $this->_ne_key_for_value($k);
                $casret = $oo->set($k, $v);
                $existing[$k] = $casret;
            }
            array_push($keys, $k);
        }

        $cas = array();
        $values = $oo->getMulti($keys, $cas);
        $this->assertCount(count(array_keys($existing)), $values);

        foreach ($existing as $k => $v) {
            $this->assertArrayHasKey($k, $values);
            $this->assertEquals($this->_ne_key_for_value($k),
                                $values[$k]);
            $this->assertArrayHasKey($k, $cas);
            $this->assertEquals($existing[$k], $cas[$k]);
        }
    }

}
