<?php

require_once 'ViewBase.inc';
class ViewSimple extends CouchbaseViewTestCommon {
    /**
     * @test Simple View Execution
     * @pre Create an object with the special @c testid__=php entry. Store it
     * via a @c set operation. Request a view (which returns these results
     * with the special field) including the @c stale parameter as @c false.
     *
     * @post Return value contains an entry called @c 'rows'.
     *
     * @pre Iterate over each of the results in the @c 'rows' entry. Delete
     * each entry (using @c delete )
     *
     * @post Get the original key. It should no longer exist
     *
     * @post Try the view again (now that we've delete all its entries). There
     * should be no results?
     *
     * @test_plans{10.3}
     *
     */
    function testBasicView() {
        $oo = $this->getPersistOO();

        $ary = array(
            "testid__" => "php"
        );

        $key = $this->mk_key();

        $oo->set($key, json_encode($ary));

        $rows = $oo->view(COUCHBASE_TEST_DESIGN_DOC_NAME,
                          COUCHBASE_TEST_VIEW_NAME,
                          array("stale" => "false",
                                "foo" => "bar"));

        $this->assertArrayHasKey('rows', $rows);
        $this->assertArrayHasKey('total_rows', $rows);
        $rows = $rows['rows'];
        foreach ($rows as $row) {
            $id = $row['id'];
            $ret = $oo->get($id);
            $this->assertNotNull($ret);
            $rv = $this->getPersistOO()->delete($id);
        }

        $this->assertNull($oo->get($key));
        $this->assertEquals(COUCHBASE_KEY_ENOENT, $oo->getResultCode());

        $rows = $oo->view(COUCHBASE_TEST_DESIGN_DOC_NAME,
                          COUCHBASE_TEST_VIEW_NAME,
                          array("stale" => "false"));
    }

    function testSingleUri() {
        $uri = "/_design/" . COUCHBASE_TEST_DESIGN_DOC_NAME .
                "/_view/" . COUCHBASE_TEST_VIEW_NAME . "?".
                "stale=false&foo=bar";
        $h = $this->getPersistHandle();
        $rows = couchbase_view($h, $uri);
        $this->assertArrayHasKey('rows', $rows);
    }

    function testNoRes() {
        $this->markTestIncomplete("...");
    }


    /**
     * @test
     * @pre Request a missing view
     * @post We get the 'normal' Couch error hash in the result
     * @test_plans{10.4}
     */
    function testMissingView() {
        $h = $this->getPersistHandle();

        $res = couchbase_view($h, 'non-exist-design',
                              'non-exist-view',
                              array(),
                              $return_error = true);

        $this->assertInternalType('array', $res);
        $this->assertArrayHasKey('error', $res);
        $this->assertEquals('not_found', $res['error']);

        # Test with the default arguments. Should raise an exception:
        $msg = NULL;
        try {
            $res = couchbase_view($h, 'non-exist-design', 'non-exist-view');
        } catch (Exception $exc) {
            $msg = $exc->getMessage();
        }
        $this->assertNotNull($msg);
        $this->assertContains('not_found', $msg);
    }
}
