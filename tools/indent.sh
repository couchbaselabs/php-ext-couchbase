#! /bin/sh
#
# This script unsures that the source code used in the PHP extension
# is uniform. The coding style used by the PHP community (see
# http://svn.php.net/viewvc/php/php-src/trunk/CODING_STANDARDS?view=markup )
# feels alien compared to the coding convention used within the rest of
# the Couchbase projects. I would assume that most people working on the
# Couchbase projects configured their editors to use SPACE instead of tab,
# so you could execute this script with an argument to make it replace
# all tabs with spaces.
#
# We are using the 1tbs (one true bracket style) to enforce brackets
# for single expressions as well.
#
# Please run this script without any arguments before pushing the
# code to gerrit.
#
style=force-tab
if [ x$1 != x ]
then
  style=spaces
fi

ROOT=`dirname $0`/..
astyle --mode=c \
       --quiet \
       --style=1tbs \
       --indent=${style}=4 \
       --indent-namespaces \
       --indent-col1-comments \
       --max-instatement-indent=78 \
       --pad-oper \
       --pad-header \
       --add-brackets \
       --unpad-paren \
       --align-pointer=name \
       `find $ROOT -name "*.[ch]" | grep -v fastlz`

