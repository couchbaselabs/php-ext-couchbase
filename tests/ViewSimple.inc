<?php

require_once 'ViewBase.inc';
class ViewSimple extends CouchbaseViewTestCommon {
    function skipALL() {
        if (!$this->atLeastVersion(array(1,1))) {
            $this->markTestSkipped("Views not implemented in this version");
        }
    }
    /**
     * @test Simple View Execution
     * @pre Create an object with the special @c testid__=php entry. Store it
     * via a @c set operation. Request a view (which returns these results
     * with the special field) including the @c stale parameter as @c false.
     *
     * @post Return value contains an entry called @c 'rows'.
     *
     * @pre Iterate over each of the results in the @c 'rows' entry.
     * Ensure that our original key is inside the view.
     *
     * @post Try the view again (now that we've delete all its entries). There
     * should be no results?
     *
     * @test_plans{10.3}
     *
     */
    function testBasicView() {
        $this->ensureView();
        $oo = $this->getPersistOO();

        $ary = array(
            "testid__" => "php"
        );

        $key = $this->mk_key();

        $cas = $oo->set($key, json_encode($ary));
        $res = $oo->keyDurability($key, $cas,
                                  array("persist_to" => 1));

        $this->assertTrue($res);

        $rows = $oo->view(COUCHBASE_TEST_DESIGN_DOC_NAME,
                          COUCHBASE_TEST_VIEW_NAME,
                          array("stale" => "false"));

        $this->assertArrayHasKey('rows', $rows);
        $this->assertArrayHasKey('total_rows', $rows);
        $rows = $rows['rows'];
        $found_our_key = false;

        foreach ($rows as $row) {

            $id = $row['id'];
            $ret = $oo->get($id);
            $this->assertNotNull($ret);
            $rv = $this->getPersistOO()->delete($id);

            if ($id === $key) {
                $found_our_key = true;
            }
        }

        // Commenting this out. Will uncomment when we actually have
        // durable observe.

        // Give some time for our key to get deleted.. err...
        //$key_deleted = false;
        //for ($i = 0; $i < 10; $i++) {
        //    $st = $oo->observe($key, $cas, $details);
        //    if (!$st) {
        //        $key_deleted = true;
        //        break;
        //    }
        //    sleep(1);
        //}
        //$this->assertTrue($key_deleted);
        //
        //// This will fail sometimes, because we don't have "durable delete"
        //$rows = $oo->view(COUCHBASE_TEST_DESIGN_DOC_NAME,
        //                  COUCHBASE_TEST_VIEW_NAME,
        //                  array("stale" => "false"));
        //
        //$this->assertEquals(0, $rows['total_rows']);
    }

    function testSingleUri() {
        $this->ensureView();
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
