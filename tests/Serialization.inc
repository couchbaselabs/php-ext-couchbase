<?php
require_once 'Common.inc';
class Serialization extends CouchbaseTestCommon {


    /**
     * @_test (private, compare standard types)
     *
     * @pre Two objects, one is the original and one is a deserialized
     * return value.
     *
     * This function stores the given item in the cluster and retrives it.
     *
     * @param $h the handle
     * @param $k the key
     * @param $v a complex object
     * @param $tname the type name
     *
     * @post
     * The stored and retrieved items are identical
     *
     */
    function _cmp_stdtypes($h, $k, $v, $tname) {
        couchbase_delete($h, $k);
        $rv = couchbase_add($h, $k, $v);
        $this->assertNotEmpty($rv,
                              "Can serialize " . $tname);
        $ret = couchbase_get($h, $k);
        $this->assertEquals($v, $ret, "Can deserialize " . $tname);
    }

    /**
     * @test Basic Serialization
     * @pre
     * Test various input types, the types array, mixed-array, sequential array,
     * empty array, NULL, TRUE, and FALSE are tested with @ref _cmp_stdtypes.
     *
     * An actual class instance is tested as well, but we cannot compare
     * equality, only that they are of the same class
     *
     * @post all objects are equal as in the preconditon
     *
     * @test_plans{9.1}
     */

    function testSerializeBasic() {
        $h = $this->getPersistHandle();
        $value = array(1,2,3);
        $key = $this->mk_key();

        $this->_cmp_stdtypes($h, $key, array(1,2,3),
                             "Array");

        couchbase_delete($h, $key);
        $value = new stdClass();
        $rv = couchbase_add($h, $key, $value);
        $this->assertNotEmpty($rv,
                              "Can serialize class objects");
        $ret = couchbase_get($h, $key);
        $this->assertInstanceOf('stdClass', $ret,
                                "Can deserialize class objects");


        $this->_cmp_stdtypes($h, $key, array("dummy"),
                             "Single element array");

        $this->_cmp_stdtypes($h, $key, NULL, "NULL");

        $this->_cmp_stdtypes($h, $key, TRUE, "TRUE");

        $this->_cmp_stdtypes($h, $key,
                             array(1,2,3, "Test" => "bar", array("dummmy")),
                             "mixed array");
    }

    function testSerializeFileError() {
        $this->markTestSkipped("weird-object checking not yet implemented");
        $key = $this->mk_key();
        $fp = fopen(__FILE__, "r");
        $msg = NULL;
        try {
            $rv = couchbase_set($h, $key, $fp);
            $ret = couchbase_get($h, $key);
        } catch (Exception $exc) {
            $msg = $exc->getMessage();
        }
        $this->assertNotNull($msg);
        $this->assertContains('cannot serialize', $fp);
    }

    # 024
    /**
     * @test
     * Test JSON Array Serialization
     *
     * @pre Set the handle's serialization to @c COUCHBASE_SERIALIZER_JSON_ARRAY
     * and store a mixed array. Retrieve it
     *
     * @post
     * @ref _cmp_stdtypes
     *
     * @todo This test was inherited. It makes little sense to me
     */
    function testSerializeJsonArrayMixed() {
        if (!extension_loaded('json') ||
            !defined('COUCHBASE_SERIALIZER_JSON_ARRAY')) {
            $this->markTestSkipped("No JSON support");
        }

        $h = make_handle();
        couchbase_set_option($h,
                             COUCHBASE_OPT_SERIALIZER,
                             COUCHBASE_SERIALIZER_JSON_ARRAY);
        $key = $this->mk_key();

        $value = array(1,2,3, "Test" => "bar", array("dummmy"));
        $this->_cmp_stdtypes($h, $key, $value, "mixed array");
    }

    # 024
    /**
     * @test JSON Object Serialization
     * @pre @ref testSerializeJsonArrayMixed
     * @post expect @c stdClass instance
     * @todo this makes little sense as well
     */
    function testSerializeJsonObjectMixed() {

        if (!extension_loaded('json') ||
            !defined('COUCHBASE_SERIALIZER_JSON')) {
            $this->markTestSkipped("No JSON support");
        }

        $h = make_handle();
        couchbase_set_option($h,
                             COUCHBASE_OPT_SERIALIZER,
                             COUCHBASE_SERIALIZER_JSON);

        $key = $this->mk_key();
        $value = array(1,2,3, "Test" => "bar", array("dummmy"));
        $rv = couchbase_add($h, $key, $value);
        $this->assertNotEmpty($rv);
        $ret = couchbase_get($h, $key);
        $this->assertInstanceOf('stdClass', $ret,
                                "Return value is an object");
    }


    /**
     * @test Test Mixed Serializers
     *
     * @pre Create to handles, $h_php which is configured with SERIALIZER_PHP,
     * and $h_json which is configured with SERIALIZER_JSON.
     * Store an array(1,2,3) with $h_json, and retrieve it with $h_php.
     * Do the reverse
     *
     * @post
     * Values are all identical
     *
     * @remark
     * Variants: OO
     *
     * @test_plans{9.3}
     */

    function testMixedSerializationErrors() {
        $h_php = make_handle();
        $h_json = make_handle();
        $value = array(1,2,3);
        $key = $this->mk_key();

        couchbase_set_option($h_json,
                             COUCHBASE_OPT_SERIALIZER,
                             COUCHBASE_SERIALIZER_JSON);

        couchbase_set_option($h_php,
                             COUCHBASE_OPT_SERIALIZER,
                             COUCHBASE_SERIALIZER_PHP);

        $rv = couchbase_add($h_json, $key, $value);
        $this->assertNotEmpty($rv, "Can serialize JSON");

        $ret = couchbase_get($h_php, $key);

        $this->assertEquals($ret, $value,
                            "Can use different flags for JSON and native PHP");

        couchbase_delete($h_json, $key);
        $rv = couchbase_add($h_php, $key, $value);
        $this->assertNotEmpty($rv);
        $ret = couchbase_get($h_json, $key);
        $this->assertEquals($value, $ret);
    }

    function testSerializeAppend() {
        $this->markTestIncomplete("Not implemented");
    }

    # 024
    /**
     * @test Serialization Options
     *
     * @pre Create a handle. Set the serializer to PHP. Check the serialization
     * value. Set the serializer to an invalid value
     *
     * @post Changing the serializer to a valid serializer succeeds. Changing
     * to an invalid serializer raising an error indicating such
     *
     * @remark
     * Variants: OO
     */
    function testSerializerOptions() {
        $h = make_handle();
        couchbase_set_option($h, COUCHBASE_OPT_SERIALIZER,
                             COUCHBASE_SERIALIZER_PHP);
        $cur = couchbase_get_option($h, COUCHBASE_OPT_SERIALIZER);
        $this->assertEquals(COUCHBASE_SERIALIZER_PHP, $cur);

        $msg = NULL;
        try {
            couchbase_set_option($h, COUCHBASE_OPT_SERIALIZER, 1111);
        } catch (Exception $exc) {
            $msg = $exc->getMessage();
        }
        $this->assertNotNull($msg);
        $this->assertContains('unsupported serializer', $msg);
    }

    function testSerializerOptionsOO() {
        $oo = make_handle_oo();
        $rv = $oo->setOption(COUCHBASE::OPT_SERIALIZER,
                             COUCHBASE::SERIALIZER_PHP);
        $this->assertTrue((bool)$rv);

        $cur = $oo->getOption(COUCHBASE::OPT_SERIALIZER);
        $this->assertEquals(COUCHBASE::SERIALIZER_PHP, $cur);

        $msg = NULL;
        try {
            $oo->setOption(COUCHBASE::OPT_SERIALIZER, 1111);
        } catch (Exception $exc) {
            $msg = $exc->getMessage();
        }
        $this->assertNotNull($msg);
        $this->assertContains('unsupported serializer', $msg);
    }
}
