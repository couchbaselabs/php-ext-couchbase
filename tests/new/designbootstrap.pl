#!/usr/bin/perl
use strict;
use warnings;
use LWP::UserAgent;
use HTTP::Request;
use Getopt::Long;
use JSON::XS qw(encode_json);
use Data::Dumper;

GetOptions(
    'u|username=s' => \my $Username,
    'p|password=s' => \my $Password,
    'h|host=s' => \my $Host,
    'd|design=s' => \my $DesignName,
    'b|bucket=s' => \my $Bucket,
    'V|viewname=s' => \my $ViewName,
    't|testid=s' => \my $TestID,
);

$Bucket ||= "default";

my $design = {
    _id => "_design/$DesignName",
    language => "javascript",
    views => {
        $ViewName => {
            map =>
                "function (doc) { if (doc.testid__ == '$TestID') { emit(doc.id) } }"
        }
    }
};

my $ua = LWP::UserAgent->new();
$ua->credentials($Host, "", $Username, $Password);
my $req = HTTP::Request->new('PUT',
                             "http://$Host:8092/$Bucket/_design/$DesignName");
$req->content(encode_json($design));
$req->header('content-type', 'application/json');
my $resp = $ua->request($req);
if (!$resp->is_success()) {
    print Dumper($resp);
}