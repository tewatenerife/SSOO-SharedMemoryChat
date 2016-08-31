#ifndef CHATROOM_H
#define CHATROOM_H

#include <iostream>
#include <cstdio>
#include <cstring>
#include <errno.h>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <sys/types.h>

#include <execinfo.h>
#include <signal.h>

class ChatRoom
{
private:
    struct SharedMessage;
    SharedMessage* sharedMessage_;      // Buffer en memoria compartida para el intercambio de mensajes
    unsigned messageReceiveCounter_;    // Número de secuencia del último mensaje leído con receive()
    bool isSharedMemoryObjectOwner_;    // Indica si el objeto es el propietario del objeto shm
    std::string chatRoomId_;            // Identificador de la sala de chat
    std::string userName_;              // Nombre de usuario que se ha conectado
    bool stopRecvThread_;               // Flag para detener la ejecución del thread de receive()

public:
    ChatRoom();
    ~ChatRoom();
    int connectTo(const std::string& chatRoomId, const std::string& userName);
    void run();

private:
    void runSender();
    void runReceiver();
    void send(const std::string& message);
    void receive(std::string& message);

    // Ejecutar el comando indicado y enviar su salida estándar
    void execAndSend(const std::string& command);
};

#endif // CHATROOM_H
