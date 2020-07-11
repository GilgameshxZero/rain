#include "rain.hpp"

int main() {
	// An integer, to do work on, and a mutex to lock the integer counter.
	std::pair<int, std::mutex> px;

	Rain::Thread::ThreadPool threadPool(8);
	Rain::Thread::ThreadPool::Task::Executor executor = [](void *param) {
		Rain::Time::sleep(250);
		std::pair<int, std::mutex> &px =
			*reinterpret_cast<std::pair<int, std::mutex> *>(param);
		px.second.lock();
		++px.first;
		px.second.unlock();
		Rain::Time::sleep(250);
	};
	void *param = reinterpret_cast<void *>(&px);
	std::cout << "Limited to 8 threads, incrementing by 25." << std::endl;
	for (int a = 0; a < 25; a++) {
		threadPool.queueTask(executor, param);
	}

	std::cout << "Waiting for task batch completion..." << std::endl;
	threadPool.blockForTasks();
	std::cout << "First batch complete. x = " << px.first << std::endl;

	Rain::Thread::ThreadPool unlimitedThreadPool(0);
	std::cout << "Unlimited threads, incrementing by 25." << std::endl;
	for (int a = 0; a < 25; a++) {
		unlimitedThreadPool.queueTask(executor, param);
	}

	std::cout << "Waiting for task batch completion..." << std::endl;
	unlimitedThreadPool.blockForTasks();
	std::cout << "Second batch complete. x = " << px.first << std::endl;

	Rain::Thread::ThreadPool bigThreadPool(512);
	std::cout << "512 threads, incrementing by 3000." << std::endl;
	for (int a = 0; a < 3000; a++) {
		bigThreadPool.queueTask(executor, param);
	}

	std::cout << "Waiting for task batch completion..." << std::endl;
	bigThreadPool.blockForTasks();
	std::cout << "Third batch complete. x = " << px.first << std::endl;

	return 0;
}
