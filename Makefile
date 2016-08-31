all: ssoo-shmchat

ssoo-shmchat: main.o chatroom.o
	g++ -o ssoo-shmchat main.o chatroom.o -lrt

main.o: main.cpp chatroom.h
	g++ -c -std=gnu++11 -pthread main.cpp -lrt
	
chatroom.o: chatroom.cpp chatroom.h
	g++ -c -std=gnu++11 -pthread chatroom.cpp -lrt
	
clean:
	rm -f ssoo-shmchat *.o
	rm /dev/shm/teguayco
