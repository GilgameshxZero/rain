/*
Thread management implementation.
*/

#pragma once

#include "../platform.hpp"
#include "../time.hpp"
#include "../type.hpp"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <list>
#include <mutex>
#include <queue>
#include <system_error>
#include <thread>

namespace Rain::Multithreading {
	// Shares threads among function tasks to minimize impact of thread
	template <typename ExecutorParamType = void *>
	class ThreadPool {
		public:
		class Task {
			public:
			// Using std::function lets us capture with lambdas. Must guarantee
			// noexcept, but cannot specify with std::function.
			typedef std::function<void(ExecutorParamType)> Executor;

			Task(Executor executor, ExecutorParamType param)
					: executor(executor), param(param) {}

			Executor executor;
			ExecutorParamType param;
		};

		private:
		// Threads quit if the newTaskCV is triggered and this is true.
		std::atomic_bool destructing;

		// Track threads.
		std::atomic_size_t maxThreads,
			cFreeThreads;	 // A thread is free if it isn't in its executor.
		mutable std::mutex threadsMtx;	// Locks threads.
		std::list<std::thread> threads;

		// Track tasks.
		mutable std::mutex tasksMtx;
		std::queue<Task> tasks;
		std::condition_variable newTaskEv,	// Breaks waiting in threads.
			noTaskEv;	 // Breaks blocking on task completion.

		public:
		// 0 is unlimited.
		ThreadPool(std::size_t maxThreads = 0) noexcept
				: destructing(false), maxThreads(maxThreads), cFreeThreads(0) {}
		~ThreadPool() {
			// If an exception is thrown, just note it and throw it away.
			try {
				// Break any waiting threads.
				this->destructing = true;
				this->newTaskEv.notify_all();

				// Now, we're only waiting on busy threads running tasks.
				// Any tasks not completed at this point won't be completed.
				for (auto it = this->threads.begin(); it != this->threads.end(); it++) {
					// If a single join throws an exception, just skip it.
					try {
						it->join();
					} catch (std::exception const &exception) {
						std::cerr << "Unexpected std::exception in "
												 "Rain::ThreadPool::~ThreadPool during it->join(): "
											<< exception.what() << std::endl;
					} catch (...) {
						std::cerr << "Unexpected non-std exception in "
												 "Rain::ThreadPool::~ThreadPool during it->join()."
											<< std::endl;
					}
				}
			} catch (std::exception const &exception) {
				std::cerr
					<< "Unexpected std::exception in Rain::ThreadPool::~ThreadPool: "
					<< exception.what() << std::endl;
			} catch (...) {
				std::cerr
					<< "Unexpected non-std exception in Rain::ThreadPool::~ThreadPool."
					<< std::endl;
			}
		}

		// Setters.
		void setMaxThreads(std::size_t newMaxThreads) noexcept {
			this->maxThreads = newMaxThreads;
		}

		void queueTask(typename Task::Executor const &executor,
			ExecutorParamType param) {
			// Any time a new task is added, notify one waiting thread.
			{
				std::lock_guard<std::mutex> tasksLckGuard(this->tasksMtx);
				this->tasks.push(Task(executor, param));
				this->newTaskEv.notify_one();
			}

			// Do we need to make a new thread?
			std::lock_guard<std::mutex> tasksLckGuard(this->tasksMtx);
			std::lock_guard<std::mutex> threadsLckGuard(this->threadsMtx);
			if (!this->tasks.empty() &&
				(this->maxThreads == 0 || this->threads.size() < this->maxThreads)) {
				// May cause exception to try again if system resources are not
				// available. In that case, it is good to set the max thread limit at
				// that, so we dont spend additional time failing to create threads.
				// Otherwise, limit threads to a reasonable (<1024) number, or a number
				// based on hardware concurrency (16?).
				this->threads.push_back(std::thread(ThreadPool::threadFnc, this));
				this->cFreeThreads++;
			}
		}

		// Functions that each thread runs to wait on next task.
		// Guaranteed to not throw; discards all exceptions to stderr.
		static void threadFnc(ThreadPool *threadPool) noexcept {
			try {
				while (true) {
					// Wait until we have a task or need to shut down.
					Task *task;
					{
						std::unique_lock<std::mutex> lck(threadPool->tasksMtx);
						threadPool->newTaskEv.wait(lck, [threadPool]() {
							return !threadPool->tasks.empty() || threadPool->destructing;
						});

						// We own the mutex now.
						// Exit if we need to.
						if (threadPool->destructing) {
							break;
						}

						// Otherwise, take on a task.
						task = new Task(threadPool->tasks.front());
						threadPool->tasks.pop();
					}
					threadPool->cFreeThreads--;
					task->executor(task->param);
					delete task;
					threadPool->cFreeThreads++;

					// Once we finish a task, see if we have anything left, and
					// conditionally trigger an event.
					std::lock_guard<std::mutex> tasksLckGuard(threadPool->tasksMtx);
					std::lock_guard<std::mutex> threadsLckGuard(threadPool->threadsMtx);
					if (threadPool->tasks.empty() &&
						threadPool->cFreeThreads == threadPool->threads.size()) {
						threadPool->noTaskEv.notify_all();
					}
				}
			} catch (std::exception const &exception) {
				std::cerr
					<< "Unexpected std::exception in Rain::ThreadPool::threadFunc: "
					<< exception.what() << std::endl;
			} catch (...) {
				std::cerr
					<< "Unexpected non-std exception in Rain::ThreadPool::threadFunc."
					<< std::endl;
			}
		}

		// Block until all tasks completed and none are processing.
		void blockForTasks() {
			// If no tasks, return immediately.
			{
				std::lock_guard<std::mutex> tasksLckGuard(this->tasksMtx);
				std::lock_guard<std::mutex> threadsLckGuard(this->threadsMtx);
				if (this->tasks.size() == 0 &&
					this->cFreeThreads == this->threads.size()) {
					return;
				}
			}

			// Otherwise, wait for tasks to be changed.
			std::unique_lock<std::mutex> lck(this->tasksMtx);
			this->noTaskEv.wait(lck, [this]() {
				std::lock_guard<std::mutex> threadsLckGuard(this->threadsMtx);
				return this->tasks.empty() &&
					this->cFreeThreads == this->threads.size();
			});
		}

		// Getters.
		std::size_t getCTasks() const noexcept {
			std::lock_guard<std::mutex> tasksLckGuard(this->tasksMtx);
			return this->tasks.size();
		}
		std::size_t getCBusyThreads() const noexcept {
			std::lock_guard<std::mutex> threadsLckGuard(this->threadsMtx);
			return this->threads.size() - this->cFreeThreads;
		}
		std::size_t getCFreeThreads() const noexcept { return this->cFreeThreads; }
		std::size_t getCThreads() const noexcept {
			std::lock_guard<std::mutex> threadsLckGuard(this->threadsMtx);
			return this->threads.size();
		}
		std::size_t getMaxThreads() const noexcept { return this->maxThreads; }
	};
}
