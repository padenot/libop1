#!/bin/sh

set -ex

if [ ! -d libsndfile ]
then
echo "Getting modified libsndfile sources..."
git submodule init
git submodule update
fi

echo "Compiling modified libsndfile sources..."
cd libsndfile
./autogen.sh
./configure --prefix=`pwd`/../external
make -j install

echo "All done."
cd ../
