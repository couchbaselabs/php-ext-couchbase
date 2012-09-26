<?php

require_once 'ViewBase.php';
require_once 'CouchbaseView.php';

class ViewObject extends CouchbaseViewTestCommon {
    function testMe() {

        $arrval = array("Hello" => "world");
        $this->insertTestId($arrval);
        $k = $this->mk_key();

        $v = new CouchbaseView(
            COUCHBASE_TEST_DESIGN_DOC_NAME,
            COUCHBASE_TEST_VIEW_NAME);

        $v->allowStale(false);

        $h = $this->getPersistHandle();

        couchbase_set($h, $k, json_encode($arrval));
        $v->execute($h);
        $rows = $v->fetchAll();
        print_r($rows);
    }
}
