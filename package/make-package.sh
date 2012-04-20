#!/bin/sh -ex
# creates a tar.gz file with a PHP extension
# depends on libcouchbase and PHP > 5.3.6 being installed
#   libcouchbase depends on libvbucket, we have OS packages
#   for all of them
#
# Usage ./package/make-package.sh
# Creates ./package/php-ext-couchbase.tar.gz

make clean
phpize
./configure
make

# make test (requires Couchbase Server running)
# make test

# package
cd package
  mkdir -p php-ext-couchbase
  cp ../CREDITS ../LICENSE README.md ../.libs/couchbase.so php-ext-couchbase
  tar czf php-ext-couchbase.tar.gz php-ext-couchbase
  rm -rf php-ext-couchbase
cd ..

# Done
echo "Done"
