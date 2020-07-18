#include <rain.hpp>

int main() {
	// An integer, to do work on, and a mutex to lock the integer counter.
	std::atomic_size_t count = 0;

	Rain::ThreadPool::Task::Executor executor = [](void *param) {
		Rain::Time::sleepMs(500);
		++(*reinterpret_cast<std::atomic_size_t *>(param));
		Rain::Time::sleepMs(500);
	};
	Rain::ThreadPool::Task::Executor printExecutor = [](void *param) {
		Rain::Time::sleepMs(500);
		std::stringstream ss;
		ss << ++(*reinterpret_cast<std::atomic_size_t *>(param)) << "\n";
		std::cout << ss.str();
		Rain::Time::sleepMs(500);
	};
	void *param = reinterpret_cast<void *>(&count);

	{
		Rain::ThreadPool threadPool(8);
		std::cout << "Limited to 8 threads, incrementing by 25." << std::endl;
		for (std::size_t a = 0; a < 25; a++) {
			threadPool.queueTask(printExecutor, param);
		}
		std::cout << "There are " << threadPool.getCThreads() << " threads."
							<< std::endl
							<< "Waiting for task batch completion..." << std::endl;
		threadPool.blockForTasks();
		std::cout << "First batch complete. x = " << count << std::endl;
	}

	{
		Rain::ThreadPool unlimitedThreadPool;
		std::cout << "Unlimited threads, incrementing by 25." << std::endl;
		for (std::size_t a = 0; a < 25; a++) {
			unlimitedThreadPool.queueTask(executor, param);
		}
		std::cout << "There are " << unlimitedThreadPool.getCThreads()
							<< " threads." << std::endl
							<< "Waiting for task batch completion..." << std::endl;
		unlimitedThreadPool.blockForTasks();
		std::cout << "Second batch complete. x = " << count << std::endl;
	}

	{
		Rain::ThreadPool bigThreadPool(2048);
		std::cout << "2048 threads, incrementing by 4000." << std::endl;
		for (std::size_t a = 0; a < 4000; a++) {
			if (!bigThreadPool.queueTask(executor, param)) {
				std::cout << "queueTask failed: setting maxThreads." << std::endl;
				bigThreadPool.setMaxThreads(bigThreadPool.getCThreads());
			}
		}
		std::cout << "There are " << bigThreadPool.getCThreads() << " threads."
							<< std::endl
							<< "Waiting for task batch completion..." << std::endl;
		bigThreadPool.blockForTasks();
		std::cout << "Third batch complete. x = " << count << std::endl;
	}

	return 0;
}
