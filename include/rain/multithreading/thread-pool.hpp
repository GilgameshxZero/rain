// ThreadPool manages an upper-bounded number of threads to work on a queue of
// tasks.
#pragma once

#include "../error/consume-throwable.hpp"
#include "../error/incrementer.hpp"
#include "../functional.hpp"
#include "../platform.hpp"
#include "../time/timeout.hpp"
#include "../type.hpp"

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <list>
#include <mutex>
#include <queue>
#include <system_error>
#include <thread>

namespace Rain::Multithreading {
	// ThreadPool manages an upper-bounded number of threads to work on a queue of
	// tasks.
	//
	// If any Task may block, the ThreadPool may block during destruction.
	// Otherwise, the ThreadPool is guaranteed not to block during destruction.
	//
	// ThreadPool consumes any exceptions thrown by its tasks.
	class ThreadPool {
		public:
		// A task is simply a callable which is executed. Parameters should be
		// captured in the lambda used as function. Should not throw.
		using Task = std::function<void()>;

		private:
		// Maximum number of threads the ThreadPool will spawn. Zero is unbounded.
		std::atomic_size_t maxThreads,
			// A thread is idle if it is not waiting on newTaskEv.
			cIdleThreads;

		// Locks cIdleThreads. Used when threads are idle.
		mutable std::mutex cIdleThreadsMtx;

		// Tracks all threads.
		std::list<std::thread> threads;

		// Locks this->threads.
		mutable std::mutex threadsMtx;

		// Task queue.
		std::queue<Task> tasks;

		// Locks this->tasks.
		mutable std::mutex tasksMtx;

		// Lock acquisition order is: tasksMtx, threadsMtx, cIdleThreadsMtx to avoid
		// deadlock.

		// Breaks when a new task comes, or when destructing.
		std::condition_variable newTaskEv,
			// Breaks when no tasks in progress and no tasks in queue.
			noTasksEv;

		// In destructor. No new tasks will be taken.
		std::atomic_bool destructing;

		public:
		ThreadPool(std::size_t maxThreads = 0) noexcept
				: maxThreads(maxThreads), cIdleThreads(0), destructing(false) {}

		// Forbid copy/move.
		ThreadPool(ThreadPool const &) = delete;
		ThreadPool &operator=(ThreadPool const &) = delete;
		ThreadPool(ThreadPool &&) = delete;
		ThreadPool &operator=(ThreadPool &&) = delete;

		// Getters.
		std::size_t getCQueuedTasks() const noexcept {
			std::lock_guard<std::mutex> tasksLckGuard(this->tasksMtx);
			return this->tasks.size();
		}
		std::size_t getCBusyThreads() const noexcept {
			std::lock_guard<std::mutex> tasksLckGuard(this->tasksMtx);
			std::lock_guard<std::mutex> threadsLckGuard(this->threadsMtx);
			std::lock_guard<std::mutex> cIdleThreadsLckGuard(this->cIdleThreadsMtx);
			return this->threads.size() - this->cIdleThreads;
		}
		std::size_t getCTasks() const noexcept {
			std::lock_guard<std::mutex> tasksLckGuard(this->tasksMtx);
			std::lock_guard<std::mutex> threadsLckGuard(this->threadsMtx);
			std::lock_guard<std::mutex> cIdleThreadsLckGuard(this->cIdleThreadsMtx);
			return this->tasks.size() + this->threads.size() - this->cIdleThreads;
		}
		std::size_t getCIdleThreads() const noexcept {
			// No need to lock here, since this atomic will always be consistent by
			// itself.
			return this->cIdleThreads;
		}
		std::size_t getCThreads() const noexcept {
			std::lock_guard<std::mutex> threadsLckGuard(this->threadsMtx);
			return this->threads.size();
		}
		std::size_t getMaxThreads() const noexcept { return this->maxThreads; }

		// Setters.
		void setMaxThreads(std::size_t newMaxThreads) noexcept {
			this->maxThreads = newMaxThreads;
		}

		void queueTask(Task const &task) {
			// Any time a new task is added, notify one waiting thread.
			std::lock_guard<std::mutex> tasksLckGuard(this->tasksMtx);
			this->tasks.push(task);
			this->newTaskEv.notify_one();

			// If no threads free, make a new thread if possible.
			std::lock_guard<std::mutex> threadsLckGuard(this->threadsMtx);
			std::lock_guard<std::mutex> cIdleThreadsLckGuard(this->cIdleThreadsMtx);
			if (
				this->cIdleThreads == 0 &&
				(this->maxThreads == 0 || this->threads.size() < this->maxThreads)) {
				// May cause exception to try again if system resources are not
				// available. In that case, it is good to set the max thread limit.
				this->threads.push_back(std::thread(ThreadPool::threadFnc, this));
			}
		}

		// Thread function.
		// If Task throws, the error is ignored to stderr.
		static void threadFnc(ThreadPool *threadPool) {
			while (!threadPool->destructing) {
				// Wait until task or destruction.
				std::unique_ptr<Task> task;

				// RAII incrementation in case of exception, with unique_ptr
				// allowing for delayed initialization.
				std::unique_ptr<Error::Incrementer<std::atomic_size_t>> incrementer;

				{
					// Before a thread idles, check to trigger a tasks done event.
					std::unique_lock<std::mutex> tasksLck(threadPool->tasksMtx);
					{
						std::lock_guard<std::mutex> threadsLckGuard(threadPool->threadsMtx);
						std::lock_guard<std::mutex> cIdleThreadsLckGuard(
							threadPool->cIdleThreadsMtx);

						// Increment via constructor.
						// If failure before destructor, the decrementing destructor
						// will be called automatically.
						incrementer.reset(new Error::Incrementer(threadPool->cIdleThreads));

						if (
							threadPool->tasks.empty() &&
							threadPool->cIdleThreads == threadPool->threads.size()) {
							threadPool->noTasksEv.notify_all();
						}
					}

					// Idle with just the tasksLck lock.
					threadPool->newTaskEv.wait(tasksLck, [threadPool]() {
						return !threadPool->tasks.empty() || threadPool->destructing;
					});

					{
						std::lock_guard<std::mutex> threadsLckGuard(threadPool->threadsMtx);
						std::lock_guard<std::mutex> cIdleThreadsLckGuard(
							threadPool->cIdleThreadsMtx);

						// Decrement via destructor.
						incrementer.reset();
					}

					// Exit if necessary.
					if (threadPool->destructing) {
						return;
					}

					// Otherwise, take a task.
					task.reset(new Task(threadPool->tasks.front()));
					threadPool->tasks.pop();

					// Release tasksMtx.
				}

				// Execute the task. Task pointer is automatically freed afterwards.
				Rain::Error::consumeThrowable(*task, RAIN_ERROR_LOCATION)();
			}
		}

		// Block until all tasks completed and none are in queue, or up to a
		// timeout. Return false if all tasks completed, or true on
		// timeout.
		bool blockForTasks(Time::Timeout timeout = {}) {
			// If no tasks, return immediately.
			{
				std::lock_guard<std::mutex> tasksLckGuard(this->tasksMtx);
				std::lock_guard<std::mutex> threadsLckGuard(this->threadsMtx);
				std::lock_guard<std::mutex> cIdleThreadsLckGuard(this->cIdleThreadsMtx);
				if (
					this->tasks.size() == 0 &&
					this->cIdleThreads == this->threads.size()) {
					return false;
				}
			}

			// Otherwise, wait for event.
			std::unique_lock<std::mutex> tasksLck(this->tasksMtx);
			auto predicate = [this]() {
				// tasksMtx is locked in predicate.

				std::lock_guard<std::mutex> threadsLckGuard(this->threadsMtx);
				std::lock_guard<std::mutex> cIdleThreadsLckGuard(this->cIdleThreadsMtx);
				return this->tasks.empty() &&
					this->cIdleThreads == this->threads.size();
			};

			if (timeout.isInfinite()) {
				this->noTasksEv.wait(tasksLck, predicate);
				return false;
			} else {
				return !this->noTasksEv.wait_until(
					tasksLck, timeout.asTimepoint(), predicate);
			}
		}

		~ThreadPool() {
			// Break any idle threads.
			this->destructing = true;
			this->newTaskEv.notify_all();

			// Wait on remaining busy threads to shutdown.
			// Any tasks not completed at this point won't be completed.
			// Do not need to lock mutexes, since only ThreadFnc may be executing
			// here, and that does not modify this->threads.
			for (auto it{this->threads.begin()}; it != this->threads.end(); it++) {
				it->join();
			}
		}
	};
}
