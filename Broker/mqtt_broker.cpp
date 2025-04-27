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
char * ackMessage(char sessionflag);
char * parseGeneralMessage(Messager* m, char* buf);
int setupServer(void);
int main(int argc, char** argv) {
    fd_set current_sockets, ready_sockets;
    unordered_map<int, Messager> who;
    unordered_map<string, vector<int>> topic_to_Sub;
    char buf[512];
    char dummy[512];
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
                    if (m->getGoodConnect()) {
                        who[m->getFd()] = *m;
                        FD_SET(m->getFd(),&current_sockets);
                    }   
                    send(m->getFd(),ackMessage(m->getGoodConnect()),5,0);
                    FD_SET(m->getFd(),&current_sockets);
                    recv(m->getFd(),dummy,512,0);
                    cout << "New Connection" << endl;
                } else {
                    Messager m = who[i];
                    bzero(buf,511);
                    int bytes = recv(i,buf,511,0);
                    if (bytes == 0) {
                        FD_CLR(i,&current_sockets);
                        continue;
                    }
                    char* mes = (char*)malloc(sizeof(char)*512);
                    bzero(mes,512);
                    mes = parseGeneralMessage(&m,buf);
                    if (m.gett() == ERR) {
                        FD_CLR(i,&current_sockets);
                    } else if (m.gett() == PUB) {
                        if (topic_to_Sub.find(m.getTopic()) != topic_to_Sub.end()) {
                            vector<int> j = topic_to_Sub[m.getTopic()];
                            for (int i = 0; i != j.size(); i++) {
                                if (send(j[i],mes,strlen(mes),MSG_NOSIGNAL) <= -1) {
                                    FD_CLR(j[i],&current_sockets);
                                    who[j[i]].setT(ERR);
                                }
                            }
                            
                        }
                    } else if (m.gett() == SUB) {
                        if (topic_to_Sub.find(m.getTopic()) != topic_to_Sub.end()) {
                            vector<int> j = topic_to_Sub[m.getTopic()];//shallow copy
                            j.push_back(m.getFd());
                            topic_to_Sub[m.getTopic()] = j;
                        } else {
                            vector<int> j = {m.getFd()};
                            topic_to_Sub[m.getTopic()] = j;
                        }
                    } else if (m.gett() == UNS) {
                        vector<int> j = topic_to_Sub[m.getTopic()];
                        vector<int> k = {};
                        for (int i = 0; i != j.size(); i++) {
                            if (m.getFd() != j[i])
                                k.push_back(j[i]);
                        }
                        topic_to_Sub[m.getTopic()] = k;
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
    char buf[512] = {0};
    bzero(buf,512);
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
    if (m == NULL) {
        return NULL;
    }
    m->setIp(clientAddr);
    m->setFd(acceptfd);
    return (void*)m;
}

void* parseConnection(char *buf) {
    Messager *m = new Messager();
    int offset = 0;
    int connect = buf[offset++];
    int getSizeOfRest = 0;
    int connectVarSize = 10;
    int sizeProtocol = 0;
    m->setGoodConnect(1);
    if (connect != 1) {// we have a normal connection
        printf("Wrong Message\n");
        m->setGoodConnect(0);
        return (void*)m;
    }
    for (int i = 0; i != 4; ++i) {
        getSizeOfRest |= buf[offset++] << (8 * (i-1));
    }
    sizeProtocol |= buf[offset++];
    if (memcmp(buf+offset,"MQTT",sizeProtocol)) {
        printf("Bad Protocol\n");
        m->setGoodConnect(0);
        return (void*)m;
    }
    offset += sizeProtocol;
    return (void*)m;
}

char * ackMessage(char sessionflag) {// 4 bytes in total
    char* buf = (char*)malloc(sizeof(char)*16);
    unsigned char connect = (0xff & 0x20);
    unsigned char sizeOfMes = (0xf & 2);
    unsigned char session = (0xf & sessionflag);
    unsigned char returnCode = 1;
    buf[0] = connect;
    buf[1] = sizeOfMes;
    buf[2] = session;
    buf[3] = returnCode;
    return buf;
}

char * parseGeneralMessage(Messager* m, char* buf) {
    unsigned char mesType = buf[0];
    unsigned int variableLength = 0;
    int offset = 1;
    if ((mesType>>4) == 3) {//publish
        int payloadLength = 0;
        short topicLength = 0;
        string topic = "";
        for (int i = 0; i != 4; i++) {
            variableLength |= buf[offset++] << ((3 - i) * 8);
        }
        for (int i = 0; i != 2; ++i)
            topicLength |= buf[offset++] << ((1 - i) * 8);
        payloadLength = variableLength - topicLength;
        char * payload = (char*)malloc(sizeof(char)*(payloadLength+1));
        bzero(payload,payloadLength);
        for (int i = 0; i != topicLength; i++) {
            topic = topic + buf[offset++];
        }
        for (int i = 0; i != payloadLength; i++) {
            payload[i] = buf[offset++];
        }
        payload[payloadLength] = '\0';
        m->setTopic(topic);
        m->setT(PUB);
        return payload;
    } else if ((mesType>>4) == 8) {// subscribe
        int variableLength = 0;
        short numOfRequests = 0;
        char *mes = (char*)malloc(512*sizeof(char));
        bzero(mes,512);
        for (int i = 0; i != 4; ++i) {
            variableLength |= buf[offset++] << ((3 - i) * 8);
        }
        for (int i = 0; i != 2; ++i) {
            numOfRequests |= buf[offset++] << ((1 - i) * 8);
        }
        int payloadlength = variableLength - 2;
        short topicLen = 0;
        for (int j = 0; j != numOfRequests; ++j) {
            for (int i = 0; i != 2; ++i) {
                topicLen |= buf[offset++] << ((1 - i) * 8);
            }
            int i = 0;
            for (i = 0; i!= topicLen; i++) {
                mes[i] = buf[offset++];
            }
            mes[i] = '\0';
        }
        m->setTopic(mes);
        m->setT(SUB);
        return NULL;
    } else if ((mesType >>4) == 14) {// disconnect
        m->setT(ERR);
        return NULL;
    } else if ((mesType >> 4) == 10) {// unsubscribe
        m->setT(UNS);
        return NULL;
    } else {
        printf("Unrecognized Message Type!\n");
        m->setT(ERR);
        return NULL;
    }
    return NULL;
}
