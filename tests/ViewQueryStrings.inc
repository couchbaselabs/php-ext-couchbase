<?php

require_once 'ViewBase.inc';
class ViewQueryStrings extends CouchbaseViewTestCommon {
    function skipALL() {
        if (!$this->atLeastVersion(array(1,1))) {
            $this->markTestSkipped("Views not implemented in this version");
        }
    }

    var $dummy_view = "dummy_view";
    var $dummy_ddoc = "dummy_ddoc";

    private function _gen_exp_base($design, $view) {
        return sprintf("/_design/%s/_view/%s?",
                       $this->dummy_ddoc, $this->dummy_view);
    }

    private function _wrap_qgen($opts) {
        $rv = $this->getPersistOO()->viewGenQuery(
            $this->dummy_ddoc,
            $this->dummy_view,
            $opts
        );
        return $rv;
    }

    private function _assert_vopteq($exp, $name, $value) {
        $s = $this->_wrap_qgen(array($name => $value));
        $this->assertEquals(
            $this->_gen_exp_base($this->dummy_ddoc, $this->dummy_view) . $exp,
            $s
        );
    }

    /**
     * @test stale parameters for view queries
     *
     * @pre Test stale with various boolean options which should be coerced,
     * and with an 'update_after' options.
     *
     * @post all restultant strings shall appear normal
     */
    function testStaleParams() {
        $this->_assert_vopteq("stale=ok", "stale", true);
        $this->_assert_vopteq("stale=false", "stale", false);
        $this->_assert_vopteq("stale=update_after", "stale", "update_after");
        $this->_assert_vopteq("stale=ok", "stale", 1);
        $this->_assert_vopteq("stale=false", "stale", 0);
        $this->_assert_vopteq("stale=false", "stale", "false");
    }

    /**
     * @test Stale with non-sensical arguments
     * @post Throws an error
     */
    function testBadStale() {
        $this->assertExceptionRegexp(
            create_function(
                '$oo',
                '$oo->viewGenQuery("d", "v", array("stale"=>"blahblah"));'
            ),
            array($this->getPersistOO()),
            "/Problem with value/"
        );
    }

    /**
     * @test Unrecognized view parameters
     * @pre test with various unrecognized view options
     * @post They all fail with an error
     */
    function testUnrecognizedParam() {
        $this->assertExceptionRegexp(
            create_function(
                '$oo',
                '$oo->viewGenQuery("d", "v", array("frobble" => "gobble"));'
            ),
            array($this->getPersistOO()),
            "/unrecognized/i"
        );
    }

    /**
     * @test Test various options that take booleans.
     *
     * @pre Feed a bunch of conversible variables so that they may be converted
     * into booleans.
     *
     * @post The query string returns successfully
     *
     * @pre Test with nonsensical strings that cannot be sanely evaluated as
     * boolean
     *
     * @post nonsensical values fail with errors
     */
    function testMiscBooleans() {
        $pnames = array(
            "descending",
            "reduce",
            "inclusive_end",
            "full_set",
            "group",
        );

        foreach ($pnames as $name) {
            # With false:
            $exp = "$name=false";
            foreach (array(NULL, 0, false, "false") as $fv) {
                $this->_assert_vopteq($exp, $name, $fv);
            }

            $exp = "$name=true";
            foreach (array(1, true, "true") as $tv) {
                $this->_assert_vopteq($exp, $name, $tv);
            }

            $functxt = '$oo->viewGenQuery("d", "v", array("%s" => "frobble"));';
            $functxt = sprintf($functxt, $name);

            $this->assertExceptionRegexp(
                create_function('$oo', $functxt),
                array($this->getPersistOO()),
                "/Problem with value/"
            );
        }
    }

    /**
     * @test Test various keywords which accept a numeric argument
     *
     * @pre Test that numeric strings are conversible
     * @post Numeric strings are converted properly
     *
     * @pre Test with non-numeric strings
     * @post Such parameters result in errors
     */
    function testMiscNumeric() {
        $pnames = array(
            "group_level",
            "skip",
            "limit"
        );

        foreach ($pnames as $name) {
            $exp = "$name=42";
            foreach (array(42, "42") as $iv) {
                $this->_assert_vopteq($exp, $name, $iv);
            }

            $functxt = '$oo->viewGenQuery("d", "v", array("%s"=>$badval));';
            $functxt = sprintf($functxt, $name);

            foreach (array(TRUE, FALSE, "gobble") as $bv) {
                $msg = sprintf("name=%s. Param=%s",
                            $name, print_r($bv, true));

                $fn = create_function('$oo, $badval', $functxt);
                $this->assertExceptionRegexp(
                    $fn,
                    array($this->getPersistOO(), $bv),
                    "/Problem with value/",
                    $msg
                );
            }
        }
    }

    /**
     * @test Values which should be in JSON-encoded format
     *
     * @pre Check various parameters which accept a JSON value. Pass complex
     * objects and strings
     *
     * @post The values are properly encoded into json and are then uri-encoded
     */
    function testEncodeStringToJson() {
        $pnames = array(
            "endkey",
            "key",
            "startkey"
        );

        $values = array(
            "dummy",
            42,
            null,
            true,
            false,
            array("chicken", "broth"),
            array("soup" => "chicken")
        );

        foreach ($pnames as $name) {
            foreach ($values as $jv) {
                $exp = sprintf('%s=%s', $name,
                               urlencode(json_encode($jv)));

                $this->_assert_vopteq($exp, $name, $jv);
            }
        }
    }

    /**
     * @test Pass-Through option.
     * @pre Set the OPT_VOPTS_PASSTHROUGH on a handle
     * @post Setting arbitrary view options should now work
     */
    function testPassThrough() {
        $oo = make_handle_oo();
        $oo->setOption(COUCHBASE::OPT_VOPTS_PASSTHROUGH, 1);
        $query = $oo->viewGenQuery("d", "v", array("foo" => "bar"));
        $this->assertRegExp('/foo=bar$/', $query);
        # No error here..
    }

}
