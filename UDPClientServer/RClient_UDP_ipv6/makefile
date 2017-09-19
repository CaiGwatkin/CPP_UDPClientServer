Rclient_UDP.exe		: 	Rclient_UDP.o 
	g++ -Wall -O2 Rclient_UDP.cpp -lws2_32 -o Rclient_UDP.exe 
			
Rclient_UDP.o		:	Rclient_UDP.cpp Rclient_UDP.h myrandomizer.h
	g++ -c -O2 -Wall Rclient_UDP.cpp
	
clean:
	del *.o
	del *.exe