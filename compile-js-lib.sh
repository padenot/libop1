emcc --bind -std=c++11 -s EXPORTED_FUNCTIONS="`sh function-names.sh`" -I external/include  op1_drum_impl.cpp ../emout/lib/libsndfile.a -o libop1.js
