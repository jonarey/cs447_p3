all:
	g++ -o controller control.cpp;
	g++ -o server server.cpp;
	g++ -o receiver data.cpp;
clean:
	rm controller server receiver;
