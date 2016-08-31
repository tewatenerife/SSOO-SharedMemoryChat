#include "chatroom.h"

bool process_cmd_line(int n_params, char *params[], std::string& chatRoomId, std::string& userName)
{
    bool cmd_ok = false;

    if (n_params > 1)
    {
        if (!strcmp(params[1], "-h") || !strcmp(params[1], "--help"))
        {
            std::cout << "Mode of use: ./ssoo_shmchat [-h | --help] [[-u | --user] CHATROOMID]" << std::endl;
            cmd_ok = true;
        }
        else if (!strcmp(params[1], "-u") || !strcmp(params[1], "--user"))
        {
            userName = params[2];
            if (n_params == 4)
            {
                chatRoomId = params[3];
                cmd_ok = true;
            }
        }
        else
        {
            if (n_params == 2)
            {
                userName = getenv("USER");
                chatRoomId = params[1];
                cmd_ok = true;
            }
        }
    }

    if (!cmd_ok)
    {
        std::cout << "Type './ssooshmchat (-h | --help)' to display the mode of use" << std::endl;
    }

    return cmd_ok;
}

int main(int argc, char *argv[])
{
    std::string chatRoomId;
    std::string userName;
    ChatRoom chatRoom;
    int result;

    if (!process_cmd_line(argc, argv, chatRoomId, userName))
    {
        return -1;
    }

    if ((result = chatRoom.connectTo(chatRoomId, userName)) < 0)
    {
    	return result;
    }

    chatRoom.run();
    return 0;
}

