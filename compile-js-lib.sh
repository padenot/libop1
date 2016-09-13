emcc --bind -std=c++11 -s EXPORTED_FUNCTIONS="`sh function-names.sh`" -Ivendor -Isrc -Iinclude -Iexternal/include  src/op1_drum_impl.cpp ../emout/lib/libsndfile.a -o libop1.js
