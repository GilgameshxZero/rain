#include "rain.hpp"

int main() {
	// An integer, to do work on, and a mutex to lock the integer counter.
	std::pair<int, std::mutex> px;

	Rain::ThreadPool threadPool(4);
	Rain::ThreadPool::Task::Executor executor = [](void *param) {
		Rain::sleep(500);
		std::pair<int, std::mutex> &px =
			*reinterpret_cast<std::pair<int, std::mutex> *>(param);
		px.second.lock();
		++px.first;
		px.second.unlock();
		Rain::sleep(500);
	};
	void *param = reinterpret_cast<void *>(&px);
	std::cout << "Limited to 4 threads, incrementing by 25." << std::endl;
	for (int a = 0; a < 25; a++) {
		threadPool.queueTask(executor, param);
	}

	std::cout << "Waiting for task batch completion..." << std::endl;
	threadPool.blockUntilDone();
	std::cout << "First batch complete. x = " << px.first << std::endl;

	Rain::ThreadPool unlimitedThreadPool(0);
	std::cout << "Unlimited threads, incrementing by 25." << std::endl;
	for (int a = 0; a < 25; a++) {
		unlimitedThreadPool.queueTask(executor, param);
	}

	std::cout << "Waiting for task batch completion..." << std::endl;
	unlimitedThreadPool.blockUntilDone();
	std::cout << "Second batch complete. x = " << px.first << std::endl;

	Rain::ThreadPool bigThreadPool(512);
	std::cout << "512 threads, incrementing by 8192."
						<< std::endl;
	for (int a = 0; a < 8192; a++) {
		bigThreadPool.queueTask(executor, param);
	}

	std::cout << "Waiting for task batch completion..." << std::endl;
	bigThreadPool.blockUntilDone();
	std::cout << "Third batch complete. x = " << px.first << std::endl;

	return 0;
}
