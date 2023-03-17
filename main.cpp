#include "stdio.h"
#include "time.h"
#include "ThreadPool.h"

const int PreciseWait(const size_t _ms)
{
	printf("waiting for %dms\n", (int)_ms);
	const size_t _clock = clock();
	const size_t _targetClock = _clock + _ms;
	while (clock() < _targetClock);
	printf("done waiting.\n");
	return 0;
}

int main()
{
	printf("Press VK_ESCAPE to stop the threads and cleanup,\nif the threads don't respond they'll be terminated, default timeout is 5s\n");
	ThreadPool::SetPoolSize(20);
	for (size_t i = 0; i < 20; i++)
		ThreadPool::ThreadTask(PreciseWait, (size_t)10000);
	while (!(GetAsyncKeyState(VK_ESCAPE) & 0x01b)) {}
	printf("Cleaning Up\n");
	ThreadPool::Cleanup();
}
