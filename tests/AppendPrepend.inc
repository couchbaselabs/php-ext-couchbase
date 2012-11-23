<?php
require_once 'Common.inc';
class AppendPrepend extends CouchbaseTestCommon {
        /* 014 */

    /**
     * @test Append and Prepend
     * @pre Set a key @c foo, prepend the prefix @c prefix_, append
     * the suffix @c suffix_.
     *
     * @post Get the key. The key should now be @c prefix_foo_suffix
     *
     * @remark
     * Variant: OO
     *
     * @test_plans{2.6}
     */
    function testAppendPrependOO() {
        $key = $this->mk_key();
        $value = "foo";
        $oo = $this->oo;
        $oo->add($key, $value);
        $oo->prepend($key, "prefix_");
        $oo->append($key, "_suffix");
        $this->assertEquals("prefix_$value" . "_suffix",
                            $oo->get($key));


        # Ensure that it works if we try to wait for persistence
        $key = $this->mk_key();
        $value = "foo";
        $oo = $this->oo;
        $cas = $oo->add($key, $value);
        $cas = $oo->prepend($key, "prefix_", 0, $cas, 1);
        $cas = $oo->append($key, "_suffix", 0, $cas, 1);
        $this->assertEquals("prefix_$value" . "_suffix",
                            $oo->get($key));
    }

    function testAppendPrepend() {
        $key = $this->mk_key();
        $value = "foo";
        $h = $this->handle;
        couchbase_add($h, $key, $value);
        couchbase_prepend($h, $key, "prefix_");
        couchbase_append($h, $key, "_suffix");

        $this->assertEquals("prefix_$value" . "_suffix",
                            couchbase_get($h, $key));
    }

    /**
     * @test append to non-existing key
     * @pre append to a non-exist key
     * @post fails (false return), result code is @c NOT_STORED
     * @test_plans{2.7}
     */
    function testAppendNonExist() {
        $oo = $this->getPersistOO();
        $k = $this->mk_key();
        $rv = $oo->append($k, "this should never show");
        $this->assertFalse($rv);
        $this->assertEquals(COUCHBASE_NOT_STORED, $oo->getResultCode());
    }

    function testAppendCas() {
        $oo = $this->getPersistOO();
        $k = $this->mk_key();
        $v = "initial value";

        $cas = $oo->set($k, $v);
        $this->assertNotEmpty($cas);


        $suffix = "_appended";

        $rv = $oo->append($k, "_appended", 0, $cas);
        $this->assertNotEmpty($rv);

        $ret = $oo->get($k);
        $this->assertEquals($v.$suffix, $ret);
    }

    function testAppendInvalidCas() {
        $oo = $this->getPersistOO();
        $k = $this->mk_key();
        $v = "initial_value";
        $rv = $oo->set($k, $v);
        $this->assertNotEmpty($rv);

        $rv = $oo->append($k, "_suffix", 0, 1234566);
        $this->assertFalse($rv);
        $this->assertEquals(COUCHBASE_KEY_EEXISTS, $oo->getResultCode());
        $ret = $oo->get($k);
        $this->assertEquals($v, $ret);
    }
}
