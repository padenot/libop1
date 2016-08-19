all:
	g++ -O2  -std=c++14 op1-drum.cpp -lsndfile -L lib -Iinclude -o op1-drum
	g++ -O2  -std=c++14 op1-dump.cpp -o op1-dump

debug:
	g++ -g -O0 -fsanitize=address -std=c++14 op1-drum.cpp -lsndfile -L lib -Iinclude -o op1-drum
	g++ -g -O0 -fsanitize=address -std=c++14 op1-dump.cpp -o op1-dump
