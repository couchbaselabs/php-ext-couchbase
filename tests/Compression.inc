<?php
require_once 'Common.inc';

class Compression extends CouchbaseTestCommon {
    # Ideally we'd want to see the value actually compressed server-side, but
    # since there's no option to disable *de*-compression, there's nothing we
    # can do here.

    # 027 (doesn't really do anything there)..

    /**
     * @test Compression flags and options
     * @pre
     * Create a new handle. Check the compression setting.
     * @post
     * Compression is not set
     *
     * @pre Set the compression to Zlib. Check the compression setting
     * @post Compression setting is now @c COUCHBASE_COMPRESSION_ZLIB
     */
    function testCompressionFlags() {
        if (!extension_loaded('zlib')) {
            $this->markTestSkipped("Need Zlib extension");
        }
        $h = make_handle();
        $curcomp = couchbase_get_option($h, COUCHBASE_OPT_COMPRESSION);
        $this->assertEquals(COUCHBASE::COMPRESSION_NONE, $curcomp);

        couchbase_set_option($h, COUCHBASE_OPT_COMPRESSION,
                             COUCHBASE::COMPRESSION_ZLIB);
        $curcomp = couchbase_get_option($h, COUCHBASE_OPT_COMPRESSION);
        $this->assertEquals(COUCHBASE_COMPRESSION_ZLIB, $curcomp);

    }

    /**
     * @test Compression Functionality
     * @pre
     * Create a new handle  @c $h and set it to Zlib compression. Modify the
     * @c ini setting to make the compression threshold to 10 bytes.
     *
     * Create another handle @c $hnc and tell it to ignore decompression/compression
     * flags.
     *
     * @pre
     * Store a long value (5kb) to the server with the handle @c $h.
     * Retrieve it with the @c $hnc handle.
     *
     * @post Fetch it with the @c $hnc handle. The value should be equal to the
     * original (because we autoinflate no matter if compression is enabled
     * or not.
     *
     * @post Fetch it with the @c $h handle. The value should be equal to the
     * original
     *
     * @bug PCBC-111
     *
     * @test_plans{6.1, 6.3}
     */
    function testCompression() {
        if (!extension_loaded('zlib')) {
            $this->markTestSkipped("Need Zlib extension");
        }

        if (!$this->atLeastVersion(array(1,1))) {
            $this->markTestSkipped("Compression broken on older versions");
        }

        $h = make_handle();
        $hnc = make_handle();
        couchbase_set_option($h, COUCHBASE_OPT_COMPRESSION,
                             COUCHBASE_COMPRESSION_ZLIB);

        /* couchbase_set_option($hnc, COUCHBASE_OPT_IGNOREFLAGS, 1); */

        $inikey = "couchbase.compression_threshold";
        $this->assertGreaterThan(0, (int)ini_get($inikey));
        $this->assertTrue((bool)ini_alter($inikey,"10"));
        $this->assertEquals(10, (int)ini_get($inikey));

        # Set a compressed value
        $v = str_repeat("long_value", 500);
        $ol = strlen($v);

        $key = $this->mk_key();

        $rv = couchbase_set($h, $key, $v);
        $this->assertNotEmpty($rv);

        $ret = couchbase_get($hnc, $key);
        $this->assertNotNull($ret);
        $this->assertEquals($ol, strlen($ret));
        $this->assertEquals($ret, $v);

        $ret = couchbase_get($h, $key);
        $this->assertEquals($ret, $v);
    }
}
