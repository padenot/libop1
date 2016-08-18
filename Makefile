all:
	g++ -O2  -std=c++0x op1-drum.cpp -lsndfile -L lib -Iinclude -o op1-drum

debug:
	g++ -g -O0  -std=c++0x op1-drum.cpp -lsndfile -L lib -Iinclude -o op1-drum
