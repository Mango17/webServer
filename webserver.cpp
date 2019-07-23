#ifndef _WEB_SERVER_
#define _WEB_SERVER_

#include "threadPool.h"
#include "task.h"
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>

#endif

#define MAX_NUM 100

using namespace std;

void addEpoll(int epfd, int sockfd, bool is_et) {
	//fcntl(sockfd, F_SETFD, O_NONBLOCK);
	struct epoll_event curEvent;
	curEvent.data.fd = sockfd;
	curEvent.events = EPOLLIN | EPOLLET;
	if (is_et) {
		curEvent.events |= EPOLLONESHOT;
	}
	epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &curEvent);
	fcntl(sockfd, F_SETFL, O_NONBLOCK);
}

int main(int args, char* argv[]) {
	
	int sockfd, connfd;
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) == -1) {
		cerr << "Socket error!!!" << endl;
		exit(1);
	}

	struct sockaddr_in saddr, client;
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	saddr.sin_port = htons(8089);

	if (bind(sockfd, (struct sockaddr*)&saddr, sizeof(saddr)) == -1) {
		cerr << "Bind error!!!" << endl;
		exit(1);
	}

	if (listen(sockfd, 50) == -1) {
		cerr << "Listen error!!!" << endl;
		exit(1);
	}

	//fcntl(sockfd, F_SETFD, O_NONBLOCK);
	struct epoll_event events[MAX_NUM];
	int epfd = epoll_create(5);
	if (epfd == -1) {
		cerr << "epoll create error!!!" << endl;
		exit(1);
	}

	addEpoll(epfd, sockfd, false);

	ThreadPool<Task> *pool = new ThreadPool<Task>(20);

	while (true) {
		int ret = epoll_wait(epfd, events, MAX_NUM, 1000);
		if (ret == -1 && errno != EINTR) {
			cerr << "epoll wait error!!!" << endl;
			break;
		}
		for (int i = 0; i < ret; i++) {
			int curfd = events[i].data.fd;
			if (curfd == sockfd) {
				struct sockaddr_in client;
				socklen_t len = sizeof(client);
				connfd = accept(sockfd, (struct sockaddr*)&client, &len);
				if (connfd == -1) {
					cout << "accept error!!!" << endl;
					continue;
				}
				addEpoll(epfd, connfd, true);
			}
			else if (events[i].events & EPOLLIN) {
				Task* cur = new Task(curfd);
				pool->addTask(cur);
			}
			else {
				cout << "Other events" << endl;
			}
		}
	}
	close(sockfd);

	return 0;
}
