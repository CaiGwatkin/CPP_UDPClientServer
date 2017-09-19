Rserver_udp.exe		: 	Rserver_UDP.o 
	g++ -Wall -O2 Rserver_UDP.cpp -lws2_32 -o Rserver_UDP.exe 
			
Rserver_udp.o		:	Rserver_UDP.cpp myrandomizer.h
	g++ -c -O2 -Wall Rserver_UDP.cpp
	
clean:
	del *.o
	del *.exe