#include "stdio.h"
#include "ThreadPool.h"
#include "time.h"

void Thread::ExecuteTasks()
{
	int i = 0;
	size_t _taskSize = 0;
	while (!shouldTerminate)
	{
		if ((_taskSize = Tasks.size()) < 1)
			continue;
		IsBusy = true;
		for (i = 0; i < _taskSize; i++)
		{
			Tasks[0](); //TODO this causes an exception when too much threads are created at once, todo investigate that
			Tasks.erase(Tasks.begin());
		}
		IsBusy = false;
	}
	isTerminated = true;
}

DWORD RunThread(PVOID _thread)
{
	Thread* __thread = reinterpret_cast<Thread*>(_thread);
	if (__thread)
		__thread->ExecuteTasks();
	return 0;
}

void ThreadPool::KillThread(int _index)
{
	const clock_t _clock = clock();
	const clock_t _timeOutClock = _clock + MAX_THREAD_AWAIT_MS_BEFORE_TERMINATE;
	Thread* _thread = threads[_index];
	_thread->Terminate();
	while (clock() < _timeOutClock && !_thread->IsTerminated);
#pragma warning(disable:6258) //TerminateThread is unsafe
	if (!_thread->IsTerminated)
	{
		printf("Safe Thread Termination timed out, forcing it\n");
		TerminateThread(_thread->Handle, 0);
	}
	delete _thread;
	threads.erase(threads.begin() + _index);
}

void ThreadPool::AddToPool(const int _count)
{
	Thread* _thread = nullptr;
	for (int i = 0; i < _count; i++)
	{
		_thread = new Thread();
		_thread->Handle = CreateThread(NULL, 0, RunThread, _thread, 0, &(_thread->ID));
		if (_thread->Handle == NULL)
			printf("Failed to create a thread");
		else
			threads.push_back(_thread);
	}
}

void ThreadPool::RemoveFromPool(int _count, bool _prioritizeNonBusyThreads)
{
	if (_count < 1)
		return;
	const int _threadsSize = (int)threads.size();
	if (_count > _threadsSize)
		_count = _threadsSize;
	int _removedThreadsCount = 0;
	if (_prioritizeNonBusyThreads)
		for (int i = 0; i < _count;)
		{
			if (!threads[i]->IsBusy)
			{
				KillThread(i);
				_removedThreadsCount++;
				if (_removedThreadsCount >= _count)
					return;
			}
			else
				i++;
		}
	while (true)
	{
		KillThread(0);
		_removedThreadsCount++;
		if (_removedThreadsCount >= _count)
			return;
	}
}

void ThreadPool::SetPoolSize(int _poolSize)
{
	const int _currentPoolSize = (int)threads.size();
	if (_poolSize == _currentPoolSize)
		return;
	const int _offset = _poolSize - _currentPoolSize;
	if (_offset > 0)
		AddToPool(_offset);
	else
		RemoveFromPool(-_offset, true);
}
