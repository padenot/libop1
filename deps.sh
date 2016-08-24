#!/bin/sh

set -ex

if [ ! -d libsndfile ]
then
echo "Getting modified libsndfile sources..."
git clone https://github.com/padenot/libsndfile.git
fi

echo "Compiling modified libsndfile sources..."
cd libsndfile
./autogen.sh
./configure --prefix=`pwd`/../external
make -j install

echo "All done."
cd ../
