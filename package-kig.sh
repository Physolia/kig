#! /bin/bash

# this is mostly a log for myself for remembering how to build the kig
# packages.

OLDPWD=$(pwd)

VERSION="0.3.1"
NAME="kig"

TEMPDIR="/tmp/$NAME-package-temp"
rm -rf $TEMPDIR
mkdir $TEMPDIR
cd $TEMPDIR

cd $TEMPDIR
~domi/src/cvs2dist \
	--name "$NAME" \
	--version "$VERSION" \
	--log="$TEMPDIR/log" \
	~/src/test/kdeedu kig

cd $OLDPWD

echo " !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! "
echo "Don't forget to change #MIN_CONFIG in "
echo "configure.in.in to "
echo "#MIN_CONFIG( 3.0 ) on packaging, so kig "
echo "won't depend on Qt 3.1 but 3.0."
echo "update: also use an admin dir and Makefile.cvs "
echo "from kde 3.0"
