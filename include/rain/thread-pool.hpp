#pragma once

#include "./condition-variable.hpp"
#include "./string.hpp"

#include <functional>
#include <list>
#include <queue>
#include <thread>

namespace Rain {
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
			this->shuttingDown = true;
			this->tasksCV.notify_all();

			// Now, we're only waiting on the threads with actual tasks.
			for (auto it = this->threads.begin(); it != this->threads.end(); it++) {
				it->join();
			}
		}

		// Queue task to be done by the threads.
		void queueTask(Task::Executor executor, void *param) {
			this->tasksMtx.lock();
			this->tasks.push(Task(executor, param));
			this->tasksMtx.unlock();

			if (this->cWaitingThreads == 0) {
				if (this->maxThreads == 0 || this->threads.size() < this->maxThreads) {
					this->threads.push_back(std::thread(ThreadPool::threadFnc, this));
				}
			}

			// Any time a new task is added, notify all waiting threads. The thread
			// that takes the task will reset the condition variable.
			this->tasksCV.notify_one();
		}

		// Functions that each thread runs to wait on next task.
		static void threadFnc(ThreadPool *threadPool) {
			while (true) {
				// When we exit out of the inner while loop, we are guaranteed some task
				// on the queue.
				threadPool->tasksMtx.lock();
				while (threadPool->tasks.empty()) {
					// About to enter into wait.
					threadPool->cWaitingThreadsMtx.lock();

					// No tasks in queue. If all other threads are waiting, then we are
					// done with all our tasks.
					if (++threadPool->cWaitingThreads == threadPool->threads.size()) {
						// Fire an event for anyone waiting on it.
						threadPool->tasksDoneCV.notify_all();
					}

					threadPool->cWaitingThreadsMtx.unlock();
					threadPool->tasksMtx.unlock();

					// Enter into waiting status.
					std::unique_lock<std::mutex> lck(threadPool->tasksCV.getMutex());
					threadPool->tasksCV.wait(lck);

					if (threadPool->shuttingDown) {
						return;
					}

					// Quit wait without shutting down.
					threadPool->tasksMtx.lock();
					threadPool->cWaitingThreadsMtx.lock();
					threadPool->cWaitingThreads--;
					threadPool->cWaitingThreadsMtx.unlock();
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
		void blockUntilDone() {
			// If no tasks, return immediately.
			this->tasksMtx.lock();
			this->cWaitingThreadsMtx.lock();
			if (this->tasks.size() == 0 &&
				this->cWaitingThreads == this->threads.size()) {
				this->cWaitingThreadsMtx.unlock();
				this->tasksMtx.unlock();
				return;
			}

			// Otherwise, there are some tasks, and the CV will trigger sometime, but
			// not until both mutexes are unlocked.
			this->tasksDoneCV.unNotifyAll();
			this->cWaitingThreadsMtx.unlock();
			this->tasksMtx.unlock();
			std::unique_lock<std::mutex> lck(this->tasksDoneCV.getMutex());
			this->tasksDoneCV.wait(lck);
		}

		size_t getCThreads() { return this->threads.size(); }
		size_t getCWaitingThreads() { return this->cWaitingThreads; }
		size_t getCTasks() { return this->tasks.size(); }

		private:
		// Threads quit if the tasksCV is triggered and this is true.
		bool shuttingDown = false;

		// Track the number of free threads.
		std::mutex cWaitingThreadsMtx;
		size_t cWaitingThreads = 0;

		// Track threads.
		size_t maxThreads;
		std::list<std::thread> threads;

		// Track tasks.
		std::mutex tasksMtx;
		std::queue<Task> tasks;
		ConditionVariable tasksCV, tasksDoneCV;
	};
}
