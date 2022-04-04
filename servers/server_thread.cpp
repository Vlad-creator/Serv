#include "../include/threads.h"

int game_server(int sock);
int pow10(int i);

using namespace my_pool;

int main()
{
    int listener;
    int num_pools = std::thread::hardware_concurrency();
    try
    {
    sockaddr_in addr;
    std::cout << num_pools << std::endl;
    ThreadPool pool(num_pools);

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if(listener < 0)
    {
        throw(std::logic_error{"listener socket error"});
    }
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(4562);
    addr.sin_addr.s_addr = INADDR_ANY;
    if(bind(listener, (sockaddr*)&addr, sizeof(addr)) < 0)
    {
        close(listener);
        throw(std::logic_error{"bind error"});
    }

    listen(listener, 5);

    while(true)
    {
        int sock = accept(listener, NULL, NULL);
        //std::cout << "added connection" << std::endl;
        if(sock < 0)
        {
            close(listener);
            throw(std::logic_error{"accept error"});
        }
        pool.push(game_server , sock);
    }
    //std::cout << "end" << std::endl;
    }
    catch(std::exception const & e)
    {
        std::cout << e.what() << std::endl;
    }
    return 0;
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
    std::cout << sock << std::endl;
    while (true)
    {
        int check[2];
        int ans[3] = {0 , 0 , 0};

        recv(sock , &check , sizeof(check) , 0);
        if (check[1] == -1)
        {
            break;
        }
        if (check[0] == check[1])
        {
            ans[0] = 1;
            send(sock , &ans , sizeof(ans) , 0);
            break;
        }
        else
        {
            if ((check[1] < 1000) || (check[1] > 9999))
            {
                ans[0] = -2;
                send(sock , &ans , sizeof(ans) , 0);
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
                send(sock , &ans , sizeof(ans) , 0);
            }
        }
    }
    close(sock);
    return 1;
}