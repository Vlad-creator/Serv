#include <iostream>
#include <thread>
#include <functional>
#include <exception>
#include <queue>
#include <vector>
#include <mutex>
#include <memory>
#include <stdlib.h>
#include <set>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <cstdlib>

namespace my_pool
{

template<class T>
struct my_fut
{
	my_fut() : ready(false) {};
	T data;
	bool ready;
};

class ThreadRoutine final
{
public:
	ThreadRoutine() 
	{
		enable = true;
		working_thread = std::thread(&ThreadRoutine::work , this);
		counter = 0;
	};
	~ThreadRoutine();

	void work();
	void add_task(std::function<void()> task);

	size_t counter;
private:
	std::thread working_thread;
	std::queue<std::function<void()>> task_queue;
	std::mutex key;
	bool enable;
};

class ThreadPool final
{
public:
	ThreadPool(int num_thr = 1);
	int num_thr_;
	~ThreadPool() {};
public:
	template<class Fn , class... Args>
	std::shared_ptr<my_fut<int>> push(Fn func , Args... args)
	{
		std::function<void()> per_func = std::bind(func , args...);
		my_fut<int> res_fut;
		std::shared_ptr<my_fut<int>> res = std::make_shared<my_fut<int>>();
		std::function<void()> res_func = [=]{
								res->data = 0; 
								per_func();
								res->ready = true;
							};
		FindFreeThr()->add_task(std::bind(res_func));
		return res;
	}
	template<class R , class Fn , class... Args>
	std::shared_ptr<my_fut<R>> push(Fn func , Args... args)
	{
		std::function<R()> per_func = std::bind(func , args...);
		my_fut<R> res_fut;
		std::shared_ptr<my_fut<R>> res = std::make_shared<my_fut<R>>();
		std::function<void()> res_func = [=]{
								res->data = per_func();
								res->ready = true;
							};
		FindFreeThr()->add_task(std::bind(res_func));
		return res;
	}
	std::shared_ptr<ThreadRoutine> FindFreeThr();
private:
	std::vector<std::shared_ptr<ThreadRoutine>> workers;
};

void ThreadRoutine::work()
{
	while (enable)
	{
		if (!task_queue.empty())
		{
			key.lock();
			std::function<void()> func = task_queue.front();
			task_queue.pop();
			key.unlock();
			func();
			counter--;
		}
	}
}

void ThreadRoutine::add_task(std::function<void()> func)
{
	key.lock();
	task_queue.push(func);
	counter++;
	key.unlock();
}

ThreadRoutine::~ThreadRoutine()
{
	enable = false;
	working_thread.join();
}

ThreadPool::ThreadPool(int num_thr) : num_thr_(num_thr)
{
	if (num_thr < 0)
	{
		throw(std::logic_error("error"));
	}
	if (num_thr == 0)
	{
		num_thr = 1;
	}
	for (size_t count = 0 ; count < num_thr ; ++count)
	{
		workers.push_back(std::make_shared<ThreadRoutine>());
	}
}

std::shared_ptr<ThreadRoutine> ThreadPool::FindFreeThr()
{
	std::shared_ptr<ThreadRoutine> per = std::make_shared<ThreadRoutine>();
	per->counter = workers[0]->counter;
	for (auto it = workers.begin() ; it != workers.end() ; ++it)
	{
		if ((*it)->counter == 0)
		{
			std::cout << "found" << std::endl;
			return *it;
		}
		if (per->counter > (*it)->counter)
		{
			per = (*it);
		}
	}
	return per;
}

};
