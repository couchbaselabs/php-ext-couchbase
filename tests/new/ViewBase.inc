<?php

require_once 'Common.inc';

define('COUCHBASE_TEST_DESIGN_DOC_NAME', 'php-couchbase-view-tests');
define('COUCHBASE_TEST_VIEW_NAME', 'php-test-view');

class CouchbaseViewTestCommon extends CouchbaseTestCommon {

    static function setUpBeforeClass() {
        parent::setUpBeforeClass();
        CouchbaseViewTestCommon::initDesign();
    }

    function insertTestId(&$ary) {
        $ary['testid__'] = 'php';
    }

    function makeDocId($ary) {
        $this->insertTestId($ary);
        $ary['id'] = $this->mk_key();
    }

    static function initDesign() {
        $script = dirname(__FILE__) . "/designbootstrap.pl";

        $cmd = $script;
        if (COUCHBASE_CONFIG_USER) {
            $cmd .= sprintf(" -u %s -p %s",
                            COUCHBASE_CONFIG_USER,
                            COUCHBASE_CONFIG_PASSWD);
        }
        if (COUCHBASE_CONFIG_BUCKET) {
            $cmd .= sprintf(" -b %s", COUCHBASE_CONFIG_BUCKET);
        }

        $cmd .= sprintf(" -h %s", COUCHBASE_CONFIG_HOST);

        $cmd .= sprintf(' -d %s -V %s',
                        COUCHBASE_TEST_DESIGN_DOC_NAME,
                        COUCHBASE_TEST_VIEW_NAME);

        $cmd .= " -t php";

        system($cmd);
    }

    function getStdViewname() {
        return COUCHBASE_TEST_DESIGN_DOC_NAME . "/php-test-view";
    }
}

?>
