// Test for Multithreading::ThreadPool.
#include <rain/literal.hpp>
#include <rain/multithreading/thread-pool.hpp>
#include <rain/time.hpp>

#include <cassert>
#include <iostream>

int main() {
	using namespace Rain::Literal;

	// A counter for work.
	std::atomic_size_t counter = 0;

	// Two types of tasks which both increment the counter, one which prints the
	// counter as well.
	auto task = [&counter]() noexcept {
		std::this_thread::sleep_for(1s);
		counter++;
	};
	auto printTask = [&counter, task]() noexcept {
		task();
		std::cout << counter << "\n";
	};

	// 25 tasks, 8 threads; so expect 4 "rounds" of tasks. Test general behavior.
	{
		Rain::Multithreading::ThreadPool threadPool(8);
		std::cout << "Limited to 8 threads, incrementing counter by 25."
							<< std::endl;
		for (std::size_t a = 0; a < 25; a++) {
			threadPool.queueTask(printTask);
		}
		auto taskSpawnTime = std::chrono::system_clock::now();
		std::cout << "There are " << threadPool.getCThreads() << " threads."
							<< std::endl
							<< "Waiting for task batch completion..." << std::endl;
		threadPool.blockForTasks();
		std::cout << "First batch complete, counter = " << counter << "."
							<< std::endl
							<< "Time elapsed after task spawning: "
							<< std::chrono::duration_cast<std::chrono::milliseconds>(
									 std::chrono::system_clock::now() - taskSpawnTime)
									 .count()
							<< "ms." << std::endl
							<< std::endl;
		assert(counter == 25);
		assert(threadPool.getCThreads() == 8);
	}

	// Unlimited threads, 25 tasks.Test spawning additional threads, until we are
	// at the maximum of 25 tasks/threads.
	{
		Rain::Multithreading::ThreadPool unlimitedThreadPool;
		std::cout << "Unlimited threads, incrementing counter by 25." << std::endl;
		for (std::size_t a = 0; a < 25; a++) {
			unlimitedThreadPool.queueTask(task);
		}
		auto taskSpawnTime = std::chrono::system_clock::now();
		std::cout << "There are " << unlimitedThreadPool.getCThreads()
							<< " threads." << std::endl
							<< "Waiting for task batch completion..." << std::endl;
		unlimitedThreadPool.blockForTasks();
		std::cout << "Second batch complete, counter = " << counter << "."
							<< std::endl
							<< "Time elapsed after task spawning: "
							<< std::chrono::duration_cast<std::chrono::milliseconds>(
									 std::chrono::system_clock::now() - taskSpawnTime)
									 .count()
							<< "ms." << std::endl
							<< std::endl;
		assert(counter == 50);
		assert(unlimitedThreadPool.getCThreads() == 25);
	}

	// Very large number of tasks (4000), with a limit of 2048 threads.However, on
	// most systems, this fails during spawning threads (thread limit), so test
	// that behavior too.
	{
		Rain::Multithreading::ThreadPool bigThreadPool(2048);
		std::cout << "2048 threads, incrementing counter by 4000." << std::endl;
		for (std::size_t a = 0; a < 4000; a++) {
			try {
				bigThreadPool.queueTask(task);
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
		std::cout << "Third batch complete, counter = " << counter << "."
							<< std::endl
							<< "Time elapsed after task spawning: "
							<< std::chrono::duration_cast<std::chrono::milliseconds>(
									 std::chrono::system_clock::now() - taskSpawnTime)
									 .count()
							<< "ms." << std::endl
							<< std::endl;
		assert(counter == 4050);
	}

	// Tasks which throw exceptions should be reported but not crash.
	Rain::Multithreading::ThreadPool::Task throwingTask([]() {
		std::this_thread::sleep_for(500ms);
		throw std::runtime_error("*fanfare* You've been pranked!\n");
	});

	{
		Rain::Multithreading::ThreadPool threadPool(2);
		for (std::size_t index = 0; index < 5; index++) {
			threadPool.queueTask(throwingTask);
		}
		threadPool.blockForTasks();
	}

	return 0;
}
