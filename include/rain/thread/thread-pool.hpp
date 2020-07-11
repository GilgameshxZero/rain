#pragma once

#include "../platform/.hpp"
#include "./condition-variable.hpp"

#include <functional>
#include <list>
#include <queue>
#include <thread>

namespace Rain::Thread {
	// Shares threads among function tasks to minimize impact of thread
	class ThreadPool {
		public:
		struct Task {
			// Using std::function lets us capture with lambdas.
			typedef std::function<void(void *)> Executor;

			Task(Executor executor, void *param) : executor(executor), param(param) {}

			Executor executor;
			void *param;
		};

		ThreadPool(size_t maxThreads = 0) : maxThreads(maxThreads) {}
		~ThreadPool() {
			// Break any waiting threads.
			this->destructing = true;
			this->newTaskCV.notifyAll();

			// Now, we're only waiting on busy threads running tasks.
			for (auto it = this->threads.begin(); it != this->threads.end(); it++) {
				it->join();
			}
		}

		void queueTask(Task::Executor executor, void *param) {
			this->tasksMtx.lock();
			this->tasks.push(Task(executor, param));
			this->tasksMtx.unlock();

			if (this->cFreeThreads == 0) {
				if (this->maxThreads == 0 || this->threads.size() < this->maxThreads) {
					this->threads.push_back(std::thread(ThreadPool::threadFnc, this));
				}
			}

			// Any time a new task is added, notify all waiting threads. The thread
			// that takes the task will reset the condition variable.
			this->newTaskCV.notifyOne();
		}

		// Functions that each thread runs to wait on next task.
		static void threadFnc(ThreadPool *threadPool) {
			while (true) {
				// When we exit out of the inner while loop, we are guaranteed some task
				// on the queue.
				threadPool->tasksMtx.lock();
				while (threadPool->tasks.empty()) {
					// About to enter into wait.
					threadPool->cFreeThreadsMtx.lock();

					// No tasks in queue. If all other threads are waiting, then we are
					// done with all our tasks.
					if (++threadPool->cFreeThreads == threadPool->threads.size()) {
						// Fire an event for anyone waiting on it.
						threadPool->noTasksCV.notifyAll();
					}

					threadPool->cFreeThreadsMtx.unlock();
					threadPool->tasksMtx.unlock();

					// Enter into waiting status.
					std::unique_lock<std::mutex> lck(threadPool->newTaskCV.getMtx());
					threadPool->newTaskCV.wait(lck);

					if (threadPool->destructing) {
						return;
					}

					// Quit wait without shutting down.
					threadPool->tasksMtx.lock();
					threadPool->cFreeThreadsMtx.lock();
					threadPool->cFreeThreads--;
					threadPool->cFreeThreadsMtx.unlock();
				}

				// We know there is a task in the queue here, and it won't change while
				// we have the tasksMtx locked.
				Task task = threadPool->tasks.front();
				threadPool->tasks.pop();
				threadPool->tasksMtx.unlock();
				task.executor(task.param);
			}
		}

		// Block until all tasks completed and none are processing.
		void blockForTasks() {
			// If no tasks, return immediately.
			this->tasksMtx.lock();
			this->cFreeThreadsMtx.lock();
			if (this->tasks.size() == 0 &&
				this->cFreeThreads == this->threads.size()) {
				this->cFreeThreadsMtx.unlock();
				this->tasksMtx.unlock();
				return;
			}

			// Otherwise, there are some tasks, and the CV will trigger sometime, but
			// not until both mutexes are unlocked.
			this->noTasksCV.reset();
			this->cFreeThreadsMtx.unlock();
			this->tasksMtx.unlock();
			std::unique_lock<std::mutex> lck(this->noTasksCV.getMtx());
			this->noTasksCV.wait(lck);
		}

		// Getters.
		size_t getCTasks() const { return this->tasks.size(); }
		size_t getCBusyThreads() const {
			return this->threads.size() - this->cFreeThreads;
		}
		size_t getCFreeThreads() const { return this->cFreeThreads; }
		size_t getCThreads() const { return this->threads.size(); }

		private:
		// Threads quit if the newTaskCV is triggered and this is true.
		bool destructing = false;

		// Track the number of free threads.
		size_t cFreeThreads = 0;
		std::mutex cFreeThreadsMtx;

		// Track threads.
		size_t maxThreads;
		std::list<std::thread> threads;

		// Track tasks.
		std::mutex tasksMtx;
		std::queue<Task> tasks;
		ConditionVariable newTaskCV, noTasksCV;
	};
}
