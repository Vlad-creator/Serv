#include <iostream>
#include <set>
#include <vector>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <cstdlib>
#include <exception>

int game_server(int sock);
int max_set(std::set<int> clients);
int pow10(int i);

int main()
{
    int listener;
    sockaddr_in addr;
    try
    {

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if(listener < 0)
    {
        throw(std::logic_error{"socket listener error"});
    }
    
    fcntl(listener, F_SETFL, O_NONBLOCK);
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(2222);
    addr.sin_addr.s_addr = INADDR_ANY;
    if(bind(listener, (sockaddr*)&addr, sizeof(addr)) < 0)
    {
        close(listener);
        throw(std::logic_error{"bind error"});
    }

    listen(listener, 1);
    
    std::set<int> clients;
    clients.clear();

    while(true)
    {

        std::vector<std::set<int>::iterator> for_delete;

        fd_set readset;
        FD_ZERO(&readset);
        FD_SET(listener, &readset);

        for(auto it = clients.begin(); it != clients.end(); it++)
            FD_SET(*it, &readset);

        timeval timeout;
        timeout.tv_sec = 60;
        timeout.tv_usec = 0;

        int mx = std::max(listener, max_set(clients));
        if(select(mx+1, &readset, NULL, NULL, &timeout) <= 0)
        {
            for (auto it = clients.begin() ; it != clients.end() ; ++it)
            {
                close(*it);
            }
            throw(std::logic_error{"select error"});
        }
        
        if(FD_ISSET(listener, &readset))
        {
            int sock = accept(listener, NULL, NULL);
            if(sock < 0)
            {
                for (auto it = clients.begin() ; it != clients.end() ; ++it)
                {
                    close(*it);
                }
                throw(std::logic_error{"socket accept error"});
            }
            
            fcntl(sock, F_SETFL, O_NONBLOCK);

            clients.insert(sock);
        }
        for(auto it = clients.begin(); it != clients.end(); ++it)
        {
            if(FD_ISSET(*it, &readset))
            {
                if (game_server(*it))
                {
                    close(*it);
                    for_delete.push_back(it);
                    continue;
                }
            }
        }
        for (auto it = for_delete.begin() ; it != for_delete.end() ; ++it)
        {
            clients.erase(*it);
        }


    }
    }
    catch(std::exception const & e)
    {
        std::cout << e.what() << std::endl;
    }
    return 0;
}

int max_set(std::set<int> clients)
{
    int max = 0;
    for (auto it = clients.begin() ; it != clients.end() ; ++it)
    {
        if (*it > max)
            max = *it;
    }
    return max;
}

int pow10(int i)
{
    if (i == 0)
        return 1;
    int res = 10;
    for (int j = 1 ; j < i ; ++j)
    {
        res = res * 10;
    }
    return res;
}

int game_server(int sock)
{
    int check[2];
    int ans[3] = {0 , 0 , 0};

    recv(sock , &check , sizeof(check) , 0);
    if (check[1] == -1)
    {
        return 1;
    }
    if (check[0] == check[1])
    {
        ans[0] = 1;
        send(sock , &ans , sizeof(ans) , 0);
        return 1;
    }
    else
    {
        if ((check[1] < 1000) || (check[1] > 9999))
        {
            ans[0] = -2;
            send(sock , &ans , sizeof(ans) , 0);
            return 0;
        }
        else
        {
            for (size_t i = 0 ; i < 4 ; i++)
            {
                if ((check[0] % pow10(i + 1) / pow10(i)) == (check[1] % pow10(i + 1) / pow10(i)))
                {
                    //cows
                    ans[1]++;
                }
                else
                {
                    int per = check[1] % pow10(i + 1) / pow10(i);
                    for (size_t j = 0 ; j < 4 ; ++j)
                    {
                        if ((i != j) && ((check[0] % pow10(j + 1) / pow10(j)) == per))
                        {
                            //bulls
                            ans[2]++;
                            break;
                        }
                    }
                }
            }
        }
    }
    send(sock , &ans , sizeof(ans) , 0);
    return 0;
}
