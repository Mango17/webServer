#ifndef _HTTP_SERVER_H_	
#define _HTTP_SERVER_H_

#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>
#include <string>
#include <stdio.h>
#include <algorithm>

using namespace std;

const string path = "/home/lzx/cpp/webTest/pages";

class HttpServer {
public:
	HttpServer(){}
	~HttpServer(){}

	string Get(string, string&, int&);
	string Post(string, string, string&, int&);
	string Other(string&, int&);
private:
	string getHeader(int, int);
};

string HttpServer::Get(string uri, string &file, int &size) {
	bool is_dynamic = false;
	string args("");
	string header("");
	int i = 0;
	while (i < uri.length() && uri.at(i) != '?') { i++; }
	if (i < uri.length()) {
		args.assign(uri.begin() + i + 1, uri.end());
		string tmp(uri.begin(), uri.begin() + i);
		uri.assign(tmp.begin(), tmp.end());
		is_dynamic = true;
	}

	if (uri == "/") {
		file = path + "/index.html";
	}
	else {
		file = path + uri;
	}

	char filename[128];
	i = 0;
	for (i; i < file.length(); i++) {
		filename[i] = file.at(i);
	}
	filename[i] = '\0';
	struct stat filestat;
	int ret = stat(filename, &filestat);
	if (ret < 0 || S_ISDIR(filestat.st_mode)) {
		file = path + "/404.html";
		for (i = 0; i < file.length(); i++) {
			filename[i] = file.at(i);
		}
		filename[i] = '\0';
		ret = stat(filename, &filestat);
		header = getHeader(filestat.st_size, 404);
		size = filestat.st_size;
	}
	else {
		header = getHeader(filestat.st_size, 200);
		size = filestat.st_size;
	}

	return header;
}

string HttpServer::Post(string uri, string args, string &file, int &size) {
	file = path + uri;
	struct stat filestat;
	char filename[128];
	int i = 0;
	for (i; i < file.length(); i++) {
		filename[i] = file.at(i);
	}
	filename[i] = '\0';
	string header;
	int ret = stat(filename, &filestat);
	if (ret < 0 || S_ISDIR(filestat.st_mode)) {
		file = path + "/404.html";
		for (i = 0; i < file.length(); i++) {
			filename[i] = file.at(i);
		}
		filename[i] = '\0';
		ret = stat(filename, &filestat);
		header = getHeader(filestat.st_size, 404);
	}
	else {
		header = getHeader(filestat.st_size, 200);
	}
	size = filestat.st_size;
	string user(""), pass("");
	int v = 0, w = 0;
	for (int j = 0; j < args.length(); j++) {
		if (args.at(j) == '=') { v = j;}
		if (args.at(j) == '&') { w = j; break;}
	}
	user = args.substr(v+1, w - v - 1);
	for (int j = w + 1; w < args.length(); j++) {
		if (args.at(j) == '=') { v = j; break;}
	}
	pass.assign(args.begin() + v + 1, args.end());
	//int rets = sscanf(argvs, "username=%s&password=%s", &user, &pass);
	if (user.empty() || pass.empty()) {
		string body("");
		body += "<html><title>Error</title>\r\n";
		body +=	"<body>\r\n";
		body +=	"<h3>Error<h3>\r\n<p>Post: Paremeter error</p>\r\n</body>\r\n</html>";
		header = getHeader(body.length(), 404);
		header += body;
	}
	else {
		string body("");
		body += "<html><title>OK</title>\r\n";
		body +=	"<body>\r\n";
		body +=	"<h3>OK<h3>\r\n<p>username: " + user + "</p>\r\n<p>password: " + pass + "</p>\r\n</body>\r\n</html>";
		header = getHeader(body.length(), 200);
		header += body;
	}

	//sprintf(buf, "%s", header);

	return header;
}

string HttpServer::Other(string &file, int &size) {
	char buf[128];
	file = path + "/501.html";
	struct stat filestat;
	char filename[128];
	sprintf(filename, "%s", file);
	int ret = stat(filename, &filestat);
	string header = getHeader(filestat.st_size, 501);
	size = filestat.st_size;
	//sprintf(buf,"%s", header);

	return header;
}

string HttpServer::getHeader(int size, int status) {
	string header("");
	header += "HTTP/1.1 " + to_string(status) + " OK\r\n"
		"Connection: Close\r\n" + "content-length:" + to_string(size) + "\r\n\r\n";

	return header;
}

#endif
