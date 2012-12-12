<?php

require_once 'Common.inc';

define('COUCHBASE_TEST_DESIGN_DOC_NAME', 'php-couchbase-view-tests');
define('COUCHBASE_TEST_VIEW_NAME', 'php-test-view');

class CouchbaseViewTestCommon extends CouchbaseTestCommon {

    static function setUpBeforeClass() {
        parent::setUpBeforeClass();
    }

    function insertTestId(&$ary) {
        $ary['testid__'] = 'php';
    }

    function makeDocId($ary) {
        $this->insertTestId($ary);
        $ary['id'] = $this->mk_key();
    }

    function ensureView() {
        $oo = $this->oo;
        $ddoc = $oo->getDesignDoc(COUCHBASE_TEST_DESIGN_DOC_NAME);
        if (!$ddoc) {
            $key = "php";
            $func = "function (doc, meta) { if (doc.testid__ == '" . $key . "') { emit(meta.id, NULL); }}";
            $ddoc = json_encode('{"views":{"' . COUCHBASE_TEST_VIEW_NAME . '":{"map":"' . $func . '"}}}');
            $ret = $oo->setDesignDoc(COUCHBASE_TEST_DESIGN_DOC_NAME,
                                     json_decode($ddoc));
            $this->assertTrue($ret);
        }
    }

    function getStdViewname() {
        return COUCHBASE_TEST_DESIGN_DOC_NAME . "/" . COUCHBASE_TEST_VIEW_NAME;
    }
}

?>
