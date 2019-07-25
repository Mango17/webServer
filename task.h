#ifndef _TASK_H_
#define _TASK_H_

#include "HttpServer.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <string>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

const int MAX_BUFFER_SIZE = 4096;

class Task {
public:
	Task(){}
	Task(int);
	~Task();
	void doit();
	
private:
	int connfd;
	HttpServer *hs;
};

Task::Task(int fd): connfd(fd) {
	hs = new HttpServer;
	if (!hs) {
		cerr << "Out of space!!!" << endl;
		exit(1);
	}
}

Task::~Task() {}

void Task::doit() {
	char buf[MAX_BUFFER_SIZE];
	int ret = recv(connfd, buf, sizeof(buf), 0);
	while (ret == -1) {
		ret = recv(connfd, buf, sizeof(buf), 0);
	}

	string method("");
	int i = 0;
	while (buf[i] != ' ' && buf[i] != '\0') {
		method += buf[i++];
	}
	i++;

	string URI("");
	while (buf[i] != ' ' && buf[i] != '\0') {
		URI += buf[i++];
	}
	i++;

	transform(method.begin(), method.end(), method.begin(), ::toupper);

	string file("");
	int size = 0;
	string response("");

	cout << "method:" << method << endl;
	if (method == "GET") {
		response = hs->Get(URI, file, size);
	}
	else if (method == "POST") {	
		string args("");
		int len = strlen(buf);
		while (buf[len-1] != '\r' && buf[len-1] != '\n') {
			args.insert(args.begin(), buf[len-1]);
			len--;
		}
		response = hs->Post(URI, args, file, size);
	}
	else {
		response = hs->Other(file, size);
	}

	char buffer[4096], filename[128];
	for (i = 0; i < response.length(); i++) {
		buffer[i] = response.at(i);
	}
	buffer[i] = '\0';
	for (i = 0; i < file.length(); i++) {
		filename[i] = file.at(i);
	}
	filename[i] = '\0';
	send(connfd, buffer, strlen(buffer), 0);
	if (method != "POST") {
		int filefd = open(filename, O_RDONLY);
		sendfile(connfd, filefd, 0, size);
	}

	sleep(3);
	close(connfd);
}

#endif
