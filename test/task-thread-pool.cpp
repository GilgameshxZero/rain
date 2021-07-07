/*
Tests select thread-management utilities from `rain/thread-pool.hpp`.
*/

#include <rain/multithreading/thread-pool.hpp>

#include <cassert>
#include <iostream>

int main() {
	// A counter, to do work on, and a mutex to lock the integer counter.
	std::atomic_size_t count = 0;

	// Two types of executor/tasks, which both increment the count, with a delay
	// of 1 second. One optionally prints the count as well.
	Rain::Multithreading::ThreadPool<std::atomic_size_t &>::Task::Executor
		executor = [](std::atomic_size_t &param) noexcept {
			Rain::Time::sleepMs(500);
			param++;
			Rain::Time::sleepMs(500);
		};
	Rain::Multithreading::ThreadPool<std::atomic_size_t &>::Task::Executor
		printExecutor = [](std::atomic_size_t &param) noexcept {
			Rain::Time::sleepMs(500);
			std::cout << ++param << "\n";
			Rain::Time::sleepMs(500);
		};

	// Three configurations of the ThreadPool, each of which is destructed at the
	// conclusion of its scope. Each tests different things.

	// 25 tasks, 8 threads; so expect 4 "rounds" of tasks. Test general behavior.
	{
		Rain::Multithreading::ThreadPool<std::atomic_size_t &> threadPool(8);
		std::cout << "Limited to 8 threads, incrementing counter by 25."
							<< std::endl;
		for (std::size_t a = 0; a < 25; a++) {
			threadPool.queueTask(printExecutor, count);
		}
		auto taskSpawnTime = std::chrono::system_clock::now();
		std::cout << "There are " << threadPool.getCThreads() << " threads."
							<< std::endl
							<< "Waiting for task batch completion..." << std::endl;
		threadPool.blockForTasks();
		std::cout << "First batch complete, counter = " << count << "." << std::endl
							<< "Time elapsed after task spawning: "
							<< std::chrono::duration_cast<std::chrono::milliseconds>(
									 std::chrono::system_clock::now() - taskSpawnTime)
									 .count()
							<< "ms." << std::endl
							<< std::endl;
		assert(count == 25);
		assert(threadPool.getCThreads() == 8);
	}

	// Unlimited threads, 25 tasks. Test spawning additional threads, until we are
	// at the maximum of 25 tasks/threads.
	{
		Rain::Multithreading::ThreadPool<std::atomic_size_t &> unlimitedThreadPool;
		std::cout << "Unlimited threads, incrementing counter by 25." << std::endl;
		for (std::size_t a = 0; a < 25; a++) {
			unlimitedThreadPool.queueTask(executor, count);
		}
		auto taskSpawnTime = std::chrono::system_clock::now();
		std::cout << "There are " << unlimitedThreadPool.getCThreads()
							<< " threads." << std::endl
							<< "Waiting for task batch completion..." << std::endl;
		unlimitedThreadPool.blockForTasks();
		std::cout << "Second batch complete, counter = " << count << "."
							<< std::endl
							<< "Time elapsed after task spawning: "
							<< std::chrono::duration_cast<std::chrono::milliseconds>(
									 std::chrono::system_clock::now() - taskSpawnTime)
									 .count()
							<< "ms." << std::endl
							<< std::endl;
		assert(count == 50);
		assert(unlimitedThreadPool.getCThreads() == 25);
	}

	// Very large number of tasks (4000), with a limit of 2048 threads. However,
	// on most systems, this fails during spawning threads (thread limit), so test
	// that behavior too.
	{
		auto startTime = std::chrono::system_clock::now();
		Rain::Multithreading::ThreadPool<std::atomic_size_t &> bigThreadPool(1700);
		std::cout << "2048 threads, incrementing counter by 4000." << std::endl;
		for (std::size_t a = 0; a < 4000; a++) {
			try {
				bigThreadPool.queueTask(executor, count);
			} catch (std::system_error const &exception) {
				std::cout << exception.what() << std::endl;
				std::cout << "queueTask failed: setting maxThreads." << std::endl;
				bigThreadPool.setMaxThreads(bigThreadPool.getCThreads());
			}
		}
		auto taskSpawnTime = std::chrono::system_clock::now();
		std::cout << "There are " << bigThreadPool.getCThreads() << " threads."
							<< std::endl
							<< "Waiting for task batch completion..." << std::endl;
		bigThreadPool.blockForTasks();
		std::cout << "Third batch complete, counter = " << count << "." << std::endl
							<< "Time elapsed after task spawning: "
							<< std::chrono::duration_cast<std::chrono::milliseconds>(
									 std::chrono::system_clock::now() - taskSpawnTime)
									 .count()
							<< "ms." << std::endl
							<< std::endl;
		assert(count == 4050);
	}

	return 0;
}
