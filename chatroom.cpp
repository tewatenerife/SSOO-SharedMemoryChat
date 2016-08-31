#include <condition_variable>
#include <mutex>
#include <thread>
#include <pthread.h>
#include <string>

#include "chatroom.h"

#define MSG_BYTE_SIZE 1000000
#define MODE S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH

struct ChatRoom::SharedMessage
{
	char message[MSG_BYTE_SIZE];
	unsigned int message_len;
	unsigned int n_message;
    char name_sender[80];

	std::mutex mtx;
	std::condition_variable cv;

    SharedMessage();
};

/* 
 *	Se inicializan los miembros de la struct ChatRoom::SharedMessage
 */
ChatRoom::SharedMessage::SharedMessage()
{
	message_len = 0;
	n_message = 0;
}

ChatRoom::ChatRoom(): 
	sharedMessage_(nullptr),
    messageReceiveCounter_(0),      
    isSharedMemoryObjectOwner_(false),
    stopRecvThread_(false)
{}

ChatRoom::~ChatRoom()
{
    if (sharedMessage_ != nullptr)
    {
    	if (munmap(sharedMessage_, sizeof(sharedMessage_)) < 0)
    	{
    		perror("munmap() error");
    	}

    	sharedMessage_ = nullptr;
    }

    if (isSharedMemoryObjectOwner_) 
    {
    	if (shm_unlink(chatRoomId_.c_str()) < 0) 
    	{
    		perror("shm_unlink() error");
    	}
    }
}

/*
 *	Conecta con la sala cuyo identificador es chatRoomId.
 *	Devuelve 0 en caso de éxito y un valor negativo en caso contrario.
 */
int ChatRoom::connectTo(const std::string& chatRoomId, const std::string& userName)
{
	int fd;
	chatRoomId_ = chatRoomId;
    userName_ = userName;

	if ((fd = shm_open(chatRoomId.c_str(), O_EXCL | O_CREAT | O_RDWR, MODE)) < 0)
	{
		fd = shm_open(chatRoomId.c_str(), O_RDWR, MODE);
		isSharedMemoryObjectOwner_ = false;
        std::cout << userName_ << " has joined the chat #(" << chatRoomId << ")" << std::endl;
	} 

	else
	{
		isSharedMemoryObjectOwner_ = true;
        std::cout << userName_ << " opened the chat #(" << chatRoomId << ")" << std::endl;
    }

	if (isSharedMemoryObjectOwner_)
    {
		if (ftruncate(fd, sizeof(ChatRoom::SharedMessage)) < 0)
        {
			perror("ftruncate() error");
			return -3;
		}
	}

	sharedMessage_ = (ChatRoom::SharedMessage *) mmap(NULL, 
					sizeof(ChatRoom::SharedMessage), 
					PROT_READ | PROT_WRITE, 
					MAP_SHARED, 
					fd,
                    0);

	if (!sharedMessage_ || sharedMessage_ == nullptr)
	{
		std::cerr << "failed while mapping" << std::endl;
		return -4;

    }

	if (isSharedMemoryObjectOwner_)
	{
		new(sharedMessage_) ChatRoom::SharedMessage;
	}

    return 0;
}

void ChatRoom::run()
{
    std::thread snd_th(&ChatRoom::runSender, this);
    std::thread rcv_th(&ChatRoom::runReceiver, this);

    if ( sharedMessage_ == nullptr )
        return;

    snd_th.join();
    stopRecvThread_ = true;
    sharedMessage_->cv.notify_all();
    rcv_th.join();
}

void ChatRoom::runSender()
{
	std::string msg;

	while (true)
    {
		std::getline(std::cin, msg);
        if (strcmp(msg.c_str(), ":quit") == 0 || std::cin.eof())
			break;
        else if (msg[0] == '!')
            execAndSend(msg.substr(1));
		else
			send(msg);
	}
}

void ChatRoom::send(const std::string& message)
{
    sharedMessage_->mtx.lock();
    sharedMessage_->message_len = message.length();
    message.copy(sharedMessage_->message, message.length(), 0);
    sharedMessage_->n_message++;
    strcpy(sharedMessage_->name_sender, userName_.c_str());
    sharedMessage_->cv.notify_all();
    sharedMessage_->mtx.unlock();
}

void ChatRoom::runReceiver()
{
    std::string msg;
    while (!stopRecvThread_)
    {
		receive(msg);		
        std::cout << "++ " << sharedMessage_->name_sender << " said: " << msg << std::endl;
    }
}

void ChatRoom::receive(std::string& message)
{
    std::unique_lock<std::mutex> lck(sharedMessage_->mtx);

    while (!stopRecvThread_ && messageReceiveCounter_ == sharedMessage_->n_message)
    {
        sharedMessage_->cv.wait(lck);
    }

    if (!stopRecvThread_)
    {
        messageReceiveCounter_ = sharedMessage_->n_message;
        message = "";
        for (int i = 0; i < sharedMessage_->message_len; i++)
            message += sharedMessage_->message[i];
    }
}

void ChatRoom::execAndSend(const std::string& command)
{
    char buffer[128];
    std::string output = command + "\n";
    FILE* pipe = popen(command.c_str(), "r");

    if (!pipe)
    {
        strerror(errno);
        exit(-1);
    }

    try {
        while (!feof(pipe))
        {
            if (fgets(buffer, 128, pipe) != NULL)
                output += buffer;
        }
    } catch (...) {
            pclose(pipe);
    }

    send(output);
    pclose(pipe);
}
