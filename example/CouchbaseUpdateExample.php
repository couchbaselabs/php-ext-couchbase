<?php
/**
 * Simple reviews example demonstrating complete & partial updates
 * through CouchbaseUpdate, a client wrapper API
 * Intro to example - User reviews for a hotel are pushed into an Array in Document
 * with some review specific attributes such as stars,likes,hates.. which can take
 * valid JSON values. The example demos setting objects, pushing to arrays, updating
 * array elements..
 */
require_once 'CouchbaseUpdate.php';

class Hotel {
    public $name;
    public $reviews;
}

class Review {
    public $username;
    public $stars;
    public $review_critics;
    public $reviewed_hotels;
}

class ReviewCritic {
    public $thumbs_up;
    public $thumbs_down;
}

$cu = new CouchbaseUpdate();

/* retries count specified */
$cu->setMaxRetries(10);

$doc = new Hotel();
$doc->name = "Jane Doe's Hotel";
$doc->reviews = array();

/* delete the existing document */
$cu->delete($doc->name);

/* complete update creates/replaces the document */
$rv = $cu->update($doc->name, $doc);

/**
 * partial updates in an array with fields as keys and with following properties
 * path: path from the parent where root is doc's parent object
 * val: new value to be updated
 */
$r = new Review();
$r->username = "foo";
$r->stars = 5;
$r->review_critics = new ReviewCritic();
$rv = $cu->update($doc->name, array(
            0 => array(
                "path" => array("root", "reviews"),
                "val" => $r),
            "review_count" => array(
                "path" => array("root"),
                "val" => 1)
            ), true);

/* update individual field */
$rv = $cu->update($doc->name, array(
            "username" => array(
                "path"  => array("root","reviews",0),
                "val"   => "cool_foo"
                )), true);

/* update multiple fields*/
$r = new Review();
$r->username = "bar";
$r->stars = 5;
$rv = $cu->update($doc->name, array(
            1 => array(
                "path" => array("root", "reviews"),
                "val" => $r),
            "review_count" => array(
                "path" => array("root"),
                "val" => 2),
            "reviewed_hotels" => array(
                "path" => array("root", "reviews", 0),
                "val" => array($doc->name)),
            "thumbs_up" => array(
                "path" => array("root", "reviews", 0, "review_critics"),
                "val" => 1)
            ), true);

/* set NULL */
$rv = $cu->update($doc->name, array(
            "prices" => array(
                "path" => array("root"),
                "val" => NULL)
            ), true);

/* set boolean */
$rv = $cu->update($doc->name, array(
            "is_account_valid" => array(
                "path" => array("root", "reviews", 1),
                "val" => false)
            ), true);
$rv = $cu->update($doc->name, array(
            "is_account_valid" => array(
                "path" => array("root", "reviews", 0),
                "val" => true)
            ), true);

/* set object */
$rv = $cu->update($doc->name, array(
            "review_critics" => array(
                "path" => array("root", "reviews", 1),
                "val" => new ReviewCritic())
            ), true);

/* set array */
$rv = $cu->update($doc->name, array(
            "reviewed_hotels" => array(
                "path" => array("root", "reviews", 1),
                "val" => array($doc->name))
            ), true);

/* push to an array */
$rv = $cu->update($doc->name, array(
            1 => array(
                "path" => array("root", "reviews", 0, "reviewed_hotels"),
                "val" => "")
            ), true);

/* update an array element */
$rv = $cu->update($doc->name, array(
            1 => array(
                "path" => array("root", "reviews", 0, "reviewed_hotels"),
                "val" => "John Doe's hotel")
            ), true);

print_r(json_decode($cu->get($doc->name)));
?>
