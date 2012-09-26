#!/usr/bin/perl
use strict;
use warnings;
use File::Path qw(rmtree mkpath);
use File::Basename qw(basename);

my $cbext_dir = $ENV{EXTDIR} or die "EXTDIR must be set!";
my $sys_extdir = qx(php -r "echo ini_get('extension_dir');");
my $ext = $cbext_dir . "/modules/couchbase.so";
my $tmp_extdir = "_extdir_tmp";

my $ini =  <<EOF;
extension_dir=$tmp_extdir;
extension=couchbase.so
EOF


rmtree($tmp_extdir);
mkpath($tmp_extdir);

my @orig_exts = glob("$sys_extdir/*");

foreach my $e (@orig_exts) {
    if ($e =~ /couchbase\.so$/) {
        next;
    }
    symlink($e, "$tmp_extdir/".basename($e));
}

symlink($ext, "$tmp_extdir/couchbase.so");

my $tmpini = "$tmp_extdir/ini_tmp";
open my $fh, ">", $tmpini or die "$tmpini: $!";

print $fh $ini;

close $fh;

$ENV{PHPRC} = $tmpini;
$ENV{PCBC_PHPT} = 1;

my @cmd = ("php", @ARGV);
if ($ENV{DEBUGGER}) {
    unshift @cmd, split(' ', $ENV{DEBUGGER});
}

exec @cmd;
