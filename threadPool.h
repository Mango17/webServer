#ifndef _THREAD_H_
#define _THREAD_H_

#include <pthread.h>
#include <iostream>
#include <exception>
#include <queue>
#include <errno.h>

using namespace std;

#define DEFAULT_POOL 10

template <class T>
class ThreadPool {
public:
	ThreadPool() {}
	ThreadPool(int);
	~ThreadPool();

	void initialize();
	T* getTask();
	void addTask(void*);
	static void* pwork(void*);
	void run();

private:
	int thread_number;
	pthread_t *all_threads;
	queue<T*> tasks;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	bool is_free;
};

template <class T>
ThreadPool<T>::ThreadPool(int number): thread_number(number), is_free(true) {
       if (number <= 0) {
	       this->thread_number = DEFAULT_POOL;
       }	       
       all_threads = new pthread_t[this->thread_number];
       if (!all_threads) {
	       cerr << "Out of space!!!" << endl;
	       exit(1);
       }
       initialize();
}

template <class T>
void ThreadPool<T>::initialize() {
	pthread_mutex_init(&mutex, nullptr);
	pthread_cond_init(&cond, nullptr);

	for (int i = 0; i < this->thread_number; i++) {
		int ret = pthread_create(all_threads + i, nullptr, pwork, this);
		if (ret != 0) {
			delete []all_threads;
			cerr << "Thread create error!!!" << endl;
			throw exception();
		}
		ret = pthread_detach(all_threads[i]);
		if (ret != 0) {
			delete []all_threads;
			cerr << "Thread detach error!!!" << endl;
			throw exception();
		}
	}
}

template <class T>
T* ThreadPool<T>::getTask() {
	pthread_mutex_lock(&mutex);
	T* cur = nullptr;
	while (true) {
		if (!tasks.empty()) {
			cur = tasks.front();
			tasks.pop();
			break;
		}
		pthread_cond_wait(&cond, &mutex);
	}
	pthread_mutex_unlock(&mutex);

	return cur;
}

template <class T>
void ThreadPool<T>::addTask(void* task) {
	T* cur = static_cast<T*>(task);
	pthread_mutex_lock(&mutex);
	tasks.push(cur);
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);
}

template <class T>
void* ThreadPool<T>::pwork(void* arg) {
	ThreadPool *curPool = static_cast<ThreadPool*>(arg);
	curPool->run();
}

template <class T>
void ThreadPool<T>::run() {
	while (is_free) {
		T* curTask = getTask();
		curTask->doit();
		delete curTask;
	}
}

template <class T>
ThreadPool<T>::~ThreadPool() {
	delete []all_threads;
	is_free = false;
	pthread_cond_signal(&cond);
}

#endif
