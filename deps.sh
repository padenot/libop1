#!/bin/sh

set -e

if [ -n libsndfile ]
then
echo "Getting modified libsndfile sources..."
git clone https://github.com/padenot/libsndfile.git
fi

echo "Compiling modified libsndfile sources..."
cd libsndfile
./autogen.sh
./configure --prefix=`pwd`/../op1-drum
make -j install

echo "All done."
cd ../
