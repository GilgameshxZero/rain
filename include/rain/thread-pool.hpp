#pragma once

#include "platform.hpp"
#include "time.hpp"
#include "types.hpp"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <list>
#include <mutex>
#include <queue>
#include <system_error>
#include <thread>

namespace Rain {
	// Shares threads among function tasks to minimize impact of thread
	class ThreadPool {
		public:
		struct Task {
			// Using std::function lets us capture with lambdas.
			typedef std::function<void(void *)> Executor;

			Task(Executor executor = NULL, void *param = NULL)
					: executor(executor), param(param) {}

			Executor executor;
			void *param;
		};

		// 0 is unlimited.
		ThreadPool(const std::size_t &maxThreads = 0) : maxThreads(maxThreads) {}
		~ThreadPool() noexcept {
			// Break any waiting threads.
			this->destructing = true;
			this->newTaskEv.notify_all();

			// Now, we're only waiting on busy threads running tasks.
			// Any tasks not completed at this point won't be completed.
			for (auto it = this->threads.begin(); it != this->threads.end(); it++) {
				it->join();
			}
		}

		// Returns nonzero if some error (exception while creating thread).
		void queueTask(const Task::Executor &executor, void *param) {
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
				// May cause exception if system resources are not available.
				this->threads.push_back(std::thread(ThreadPool::threadFnc, this));
				this->cFreeThreads++;
			}
		}

		// Functions that each thread runs to wait on next task.
		static void threadFnc(ThreadPool *threadPool) noexcept {
			while (true) {
				// Wait until we have a task or need to shut down.
				Task task;
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
					task = threadPool->tasks.front();
					threadPool->tasks.pop();
				}
				threadPool->cFreeThreads--;
				task.executor(task.param);
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
		}

		// Block until all tasks completed and none are processing.
		void blockForTasks() noexcept {
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
		std::size_t getCTasks() noexcept {
			std::lock_guard<std::mutex> tasksLckGuard(this->tasksMtx);
			return this->tasks.size();
		}
		std::size_t getCBusyThreads() noexcept {
			std::lock_guard<std::mutex> threadsLckGuard(this->threadsMtx);
			return this->threads.size() - this->cFreeThreads;
		}
		std::size_t getCFreeThreads() const noexcept { return this->cFreeThreads; }
		std::size_t getCThreads() noexcept {
			std::lock_guard<std::mutex> threadsLckGuard(this->threadsMtx);
			return this->threads.size();
		}
		std::size_t getMaxThreads() const noexcept { return this->maxThreads; }

		// Setters.
		void setMaxThreads(const size_t &newMaxThreads) noexcept {
			this->maxThreads = newMaxThreads;
		}

		private:
		// Threads quit if the newTaskCV is triggered and this is true.
		std::atomic_bool destructing = false;

		// Track threads.
		std::atomic_size_t maxThreads = 0;
		std::atomic_size_t cFreeThreads =
			0;	// A thread is free if it isn't in its executor.
		std::mutex threadsMtx;	// Locks threads.
		std::list<std::thread> threads;

		// Track tasks.
		std::mutex tasksMtx;
		std::queue<Task> tasks;
		std::condition_variable newTaskEv,	// Breaks waiting in threads.
			noTaskEv;	 // Breaks blocking on task completion.
	};
}
