#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <unordered_map>
#include <vector>
#define PORT 5001
using namespace std;
#include "mqtt_broker.h"


void* parseConnection(char *buf);
void* acceptConnection(int sockfd);
int setupServer(void);
int main(int argc, char** argv) {
    fd_set current_sockets, ready_sockets;
    unordered_map<int, Messager> who;
    unordered_map<string, vector<int>> topic_to_Sub;
    char buf[512];
    int socketfd = setupServer();
    FD_ZERO(&current_sockets);
    FD_SET(socketfd,&current_sockets);
    while (1) { 
        ready_sockets = current_sockets;
        if (select(FD_SETSIZE, &ready_sockets, NULL , NULL, NULL) < 0) {
            printf("Select Failure\n");
            exit(1);
        }
        for (int i = 0; i != FD_SETSIZE; i++) {
            if (FD_ISSET(i, &ready_sockets)) {
                if (i == socketfd) {
                    Messager *m = (Messager*)acceptConnection(socketfd);
                    printf("New Connection Acceptfd: %d\n",m->getFd());
                    FD_SET(m->getFd(),&current_sockets);
                    if (m->gett() == 1) {
                        if (topic_to_Sub.find(m->getTopic()) != topic_to_Sub.end())
                            topic_to_Sub[m->getTopic()].push_back(m->getFd());
                        else {
                            vector<int> l = {0, m->getFd()};
                            topic_to_Sub[m->getTopic()] = l;
                        }
                    } else if (m->gett() == 0) {
                        who[m->getFd()] = *m;
                        if (topic_to_Sub.find(m->getTopic()) != topic_to_Sub.end()) {
                            topic_to_Sub[m->getTopic()][0] += 1;
                        } else {
                            vector<int> l = {1};
                            topic_to_Sub[m->getTopic()] = l;
                        }
                    }
                } else {
                    Messager m = who[i];
                    bzero(buf,511);
                    int bytes = recv(i,buf,511,0);
                    if (strlen(buf) != 0) {
                        cout << "Message Received!" << " Bytes Received: " << bytes <<endl;
                        cout << buf << endl;
                        vector<int> j = topic_to_Sub[m.getTopic()];
                        for (int i = 1; i != j.size(); i++) {
                            send(j[i],buf,511,0);// check if still open
                        }
                    }
                    if (bytes == 0) {
                        FD_CLR(i,&current_sockets);
                    }
                }
            }
        }
    }
    
    close(socketfd);
    return 0;
}
int setupServer(void) {
    sockaddr_in address;
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0) {
        printf("Socket Failure\n");
        exit(1);
    }
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    int bindid = bind(socketfd, (struct sockaddr*)&address, sizeof(address));
    if (bindid < 0) {
        printf("Binding Failure\n");
    }
    int listenid = listen(socketfd, 10);
    return socketfd;
}
void* acceptConnection(int sockfd) {
    char buf[256] = {0};
    char clientAddr[64] = {0};
    sockaddr_in cliAddress;
    socklen_t sizeofclient = sizeof(cliAddress);
    int acceptfd = accept(sockfd,(struct sockaddr*)&cliAddress, &sizeofclient);
    if (acceptfd < 0) {
        printf("Accept Failure\n");
    }
    inet_ntop(AF_INET,&cliAddress.sin_addr,clientAddr,64);
    recv(acceptfd,buf,255,0);
    Messager *m = (Messager*)parseConnection(buf);
    m->setIp(clientAddr);
    m->setFd(acceptfd);
    return (void*)m;
}

void* parseConnection(char *buf) {
    Messager *m;
    int idx = 0, tidx = 0;
    string messagertype;
    string topic;
    while (buf[idx] != '\n') {
        messagertype = messagertype + buf[idx];
        idx++;
    }
    messagertype[idx] = '\0';
    idx++;
    idx += 7;
    while (buf[idx] != '\n') {
        topic = topic + buf[idx];
        tidx++;
        idx++;
    }
    topic[tidx] = '\0';
    if (messagertype == "Publisher") {
        m = new Messager(PUB,topic);
        
    }
    if (messagertype == "Subscriber") {
        m = new Messager(SUB,topic);
    }
    return (void*)m;
}
