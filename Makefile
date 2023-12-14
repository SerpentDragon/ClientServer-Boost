all:
	g++ client.cpp -o client -std=c++23 -lboost_system -lboost_thread
	# g++ server.cpp -o server -std=c++23 -lboost_system -lboost_thread
