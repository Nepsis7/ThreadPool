#pragma once

#include <vector>
#include "windows.h"
#include <functional>


//TODO make that into a variable ?
#define MAX_THREAD_AWAIT_MS_BEFORE_TERMINATE 5000 //if a thread takes more than this to exit, it'll be terminated by the pool

class Thread
{
private:
	bool shouldTerminate = false;
	bool isTerminated = false;
	std::vector<void*> allocatedMemory = std::vector<void*>();
public:
	HANDLE Handle = NULL;
	DWORD ID = 0;
	bool IsBusy = false;
	std::vector<std::function<void()>> Tasks = std::vector<std::function<void()>>();
	_declspec(property(get = GetIsTerminated)) bool IsTerminated;
public:
	Thread() = default;
	~Thread()
	{
		for (void* _mem : allocatedMemory)
			delete _mem;
	}
	template<typename TReturn, typename ...TParams>
	inline void AddTask(TReturn(*_task)(TParams...), TParams ..._params);
	inline const bool GetIsTerminated() const { return isTerminated; }
	inline void Terminate() { shouldTerminate = true; }
	void ExecuteTasks();
};

class ThreadPool
{
private:
	static inline std::vector<Thread*> threads = std::vector<Thread*>();
private:
	static void KillThread(int _index);
public:
	static void AddToPool(int _count);
	static void RemoveFromPool(int _count, bool _prioritizeNonBusyThreads);
	static void SetPoolSize(int _poolSize);
	template<typename TReturn, typename ...TParams>
	static const bool ThreadTask(std::function<TReturn(TParams...)>& _task, TParams... _params);
	template<typename TReturn, typename ...TParams>
	static const bool ThreadTask(TReturn(*_task)(TParams...), TParams... _params);
	static inline void Cleanup() { SetPoolSize(0); }
};

template<typename TReturn, typename ...TParams>
inline const bool ThreadPool::ThreadTask(std::function<TReturn(TParams...)>& _task, TParams... _params)
{
	for (Thread* _thread : threads)
		if (!(_thread->IsBusy))
		{
			_thread->Tasks.push_back([&]() { _task(_params...); });
			_thread->IsBusy = true;
			return true;
		}
	return false;
}

template<typename TReturn, typename ...TParams>
inline const bool ThreadPool::ThreadTask(TReturn(*_task)(TParams...), TParams ..._params)
{
	for (Thread* _thread : threads)
		if (!(_thread->IsBusy))
		{
			_thread->AddTask(_task, _params...);
			_thread->IsBusy = true;
			return true;
		}
	return false;
}

template<typename TReturn, typename ...TParams>
inline void Thread::AddTask(TReturn(*_task)(TParams...), TParams ..._params)
{
	std::function<void()>* _lambda = new std::function<void()>([&]() { _task(_params...); });
	Tasks.push_back(*_lambda);
	allocatedMemory.push_back(_lambda);
}
