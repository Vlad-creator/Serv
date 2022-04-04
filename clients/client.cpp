#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <exception>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <cstdlib>
#include <ctime>

int game_client(int sock);

int main()
{
    try
    {
    int sock;
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock < 0)
    {
        throw(std::logic_error{"socket error"});
    }
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(2222);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(connect(sock , (sockaddr*)&addr , sizeof(addr)) < 0)
    {
        close(sock);
        throw(std::logic_error{"connect error"});
    }

    int res = game_client(sock);
    close(sock);
    }
    catch(std::exception const & e)
    {
        std::cout << e.what() << std::endl;
    }
    return 0;
}

int game_client(int sock)
{
    std::srand(std::time(0));
    int secret = std::rand() % 10000;
    if (secret < 1000)
    {
        secret += 1000;
    }

    std::cout << secret << std::endl;
    int inf[2];
    inf[0] = secret;
    while (true)
    {
        std::cin >> inf[1];
        if (inf[1] == -1)
        {
            send(sock , &inf , sizeof(inf) , 0);
            std::cout << "There was very little left but you failed" << std::endl;
            break;
        }
        int ans[3];
        send(sock , &inf , sizeof(inf) , 0);
        recv(sock , &ans , sizeof(ans) , 0);
        if (ans[0] == -2)
        {
            std::cout << "enter 4-digit number" << std::endl;
            continue;
        }
        if (ans[0] == 0)
        {
            std::cout << "cows - " << ans[1] << std::endl;
            std::cout << "bulls - " << ans[2] << std::endl;
            std::cout << "Try guess again" << std::endl;
        }
        else
        {
            std::cout << "You're right" << std::endl;
            break;
        }
    }
    return 0;
}
