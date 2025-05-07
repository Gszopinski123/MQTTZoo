#define THREADED
#include <zookeeper/zookeeper.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <unordered_map>
#include <vector>
#include <netdb.h>
#include <unistd.h>
#include <time.h>
#include "mqtt_broker.h"
#define PORT 5001
#define zookeeper_port 2181
using namespace std;

char* convertAddress(char ip[], char* newIp);
void* parseConnection(char *buf);
void* acceptConnection(int sockfd);
char * ackMessage(char sessionflag);
char * parseGeneralMessage(Messager* m, char* buf);
void * intializeZK(zhandle_t *zh);
char* setupMessage(int flag, char* message, char* path, zhandle_t *zh);
int setupServer(void);
char* getPath(const char *path);
void my_watcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx);
unordered_map<int, Messager> who;
unordered_map<string, vector<int> > topic_to_Sub;
fd_set current_sockets, ready_sockets;
int main(int argc, char** argv) {
    int flag[2] = {0};
    char host[128] = "zk-gszopin-sset-x.zk-gszopin-svc.default.svc.cluster.local";
    char newHost[128] = {0};
    char topic_handle[] = "/topic";
    char* ip = (char*)malloc(sizeof(char)*128);
    if (argc > 1) {
        for (int i = 1; i != argc-1; i++) {
            if (!strcmp(argv[i],"-t")) {
                flag[0] = i+1;
                flag[1] = 1;
            }
        }
    } else {
        printf("Not enough Arguments!\n");
        exit(1);
    }
    if (flag[1]) {
        int offset = 0;
        strcpy(newHost,host);
        offset += 16;
        strcpy(newHost+offset,argv[flag[0]]);
        offset += strlen(argv[flag[0]]);
        strcpy(newHost+offset,host+17);
        offset += strlen(host+17);
        ip = convertAddress(newHost,ip);
        strcpy(ip+strlen(ip),":2181");
        cout << ip << endl;
    }
    zhandle_t *zh = zookeeper_init(ip,my_watcher,2000,0,0,0);
    if (zh == NULL) {
        printf("Bad Connection!\n");
        exit(1);
    }
    intializeZK(zh);
    
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
                        int offset = 0;
                        char topic_path[256] = {0};
                        offset = strlen(topic_handle);
                        memcpy(topic_path,topic_handle,offset);
                        memcpy(topic_path+offset,m.getTopic().c_str(),strlen(m.getTopic().c_str()));
                        offset += strlen(m.getTopic().c_str());
                        topic_path[offset++] = '\0';
                        int rt = zoo_exists(zh,topic_path,1,NULL);
                        if (rt == ZOK) {
                            cout << "Znode already Exists!" << endl;
                            setupMessage(1,mes,topic_path,zh);
                        } else if (rt == ZNONODE) {
                            zoo_create(zh,topic_path,NULL,-1,&ZOO_OPEN_ACL_UNSAFE,0,NULL,0);
                            cout << "Znode Created!" << endl;
                            setupMessage(0,mes,topic_path,zh);
                            cout << "Data sent to Znode!" << endl;
                        } else {
                            cout << "There was an error!" << endl;
                        }    
                    } else if (m.gett() == SUB) {
                        int offset = 0;
                        char topic_path[256] = {0};
                        offset = strlen(topic_handle);
                        memcpy(topic_path,topic_handle,offset);
                        memcpy(topic_path+offset,m.getTopic().c_str(),strlen(m.getTopic().c_str()));
                        int rt = zoo_exists(zh,topic_path,1,NULL);
                        if (rt == ZOK) {
                            cout << "Znode already Exists!" << endl;
                        } else if (rt == ZNONODE) {
                            zoo_create(zh,topic_path,NULL,-1,&ZOO_OPEN_ACL_UNSAFE,0,NULL,0);
                            cout << "Znode Created!" << endl;
                        } else {
                            cout << "There was an error!" << endl;
                        }
                        cout << topic_path << endl;
                        if (topic_to_Sub.find(topic_path) != topic_to_Sub.end()) {
                            cout << "Old Topic" << endl;
                            vector<int> j = topic_to_Sub[topic_path];//shallow copy
                            j.push_back(m.getFd());
                            topic_to_Sub[topic_path] = j;
                            cout << "Topic: " << topic_path << endl;
                            cout << "Length: "<< strlen(topic_path) << endl;
                            cout << "Number of Sockets!: " << topic_to_Sub[topic_path].size() << endl;
                        } else {
                            cout << "New Topic" << endl;
                            vector<int> j; 
                            j.push_back(m.getFd());
                            topic_to_Sub[topic_path] = j;
                        }
                    } else if (m.gett() == UNS) {
                        vector<int> j = topic_to_Sub[m.getTopic()];
                        vector<int> k;
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
char* convertAddress(char ip[],char* newIp) {
    struct addrinfo hints, *res, *p;
    void* addr;
    char ipv4[128];
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    getaddrinfo(ip, "2181", &hints, &res);
    p = res;
    while (res) {
        struct sockaddr_in *address = (struct sockaddr_in*)res->ai_addr;
        addr = &(address->sin_addr);
        inet_ntop(res->ai_family, addr, ipv4, sizeof(ipv4));
        res = res->ai_next;
    }
    freeaddrinfo(p);
    for (int i = 0; i != strlen(ipv4); i++) 
        newIp[i] = ipv4[i];
    return newIp;
}
/*
 * Intialize bank
 * Intialize topic
*/
void * intializeZK(zhandle_t *zh) {
    int rc = zoo_exists(zh,"/bank",0,NULL);
    if (rc == ZOK) {
        //printf("Bank Already Exists!\n");
    } else if (rc == ZNONODE) {
        char value[5] = {0};
        int vault = 100000;
        for (int i = 0; i != 4; i++)
            value[i] = (0xff&(vault >> ((3-i)*8)));
        value[4] = '\0';
        int buffer_len = 5;
        zoo_create(zh,"/bank",value,buffer_len,&ZOO_OPEN_ACL_UNSAFE,0,NULL,0);
    } else {
        printf("Failure checking!\n");
    }
    rc = zoo_exists(zh,"/topic",0,NULL);
    if (rc == ZOK) {
        //printf("Bank Already Exists!\n");
    } else if (rc == ZNONODE) {
        //printf("Creating Topics\n");
        zoo_create(zh,"/topic","ListOfTopics\0",strlen("ListOfTopics\0"),&ZOO_OPEN_ACL_UNSAFE,0,NULL,0);
    } else {
        printf("Failure checking!\n");
    }
    rc = zoo_exists(zh,"/time",0,NULL);
    if (rc == ZOK) {
        //printf("Bank Already Exists!\n");
    } else if (rc == ZNONODE) {
        //printf("Creating Topics\n");
        long start_time = time(NULL);
        char epoch[9] = {0};
        epoch[8] = '\0';
        for (int i = 0; i != 8; i++) {
            epoch[i] = (0xff&(start_time >> ((7-i)*8)));
        }
        zoo_create(zh,"/time",epoch,9,&ZOO_OPEN_ACL_UNSAFE,0,NULL,0);
    } else {
        printf("Failure checking!\n");
    }
    //sanity check
    char buffer[512] = {0};
    int buffer_len = sizeof(buffer)-1;
    rc = zoo_get(zh,"/bank", 0, buffer,&buffer_len, NULL);
    if (rc == ZOK) {
    } else if (rc == ZNONODE) {
        printf("No Node!\n");
    } else {
        printf("Error!\n");
    }
    buffer_len = sizeof(buffer)-1;
    bzero(buffer,512);
    rc = zoo_get(zh,"/time", 0, buffer,&buffer_len, NULL);
    if (rc == ZOK) {
    } else if (rc == ZNONODE) {
        printf("No Node!\n");
    } else {
        printf("Error!\n");
    }
    buffer_len = sizeof(buffer)-1;
    rc = zoo_get(zh,"/topic",0,buffer,&buffer_len,NULL);
    if (rc == ZOK) {
        //printf("Data: %.*s\n",buffer_len,buffer);
    } else if (rc == ZNONODE) {
        printf("No Node!\n");
    } else {
        printf("Error!\n");
    }
    return NULL;
}
void my_watcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx) {
    char buffer[2048] = {0};
    int buffer_len = 2047;
    char message[2048] = {0};
    int message_len = 2047;
    if (type == ZOO_CHANGED_EVENT) {
        cout << "Something Changed!" << endl;
        zoo_get(zh,path,1,buffer,&buffer_len,NULL);
        memcpy(message,buffer+8,strlen(buffer+8));
        cout << message << endl;
        if (topic_to_Sub.find(path) != topic_to_Sub.end()) {
            cout << "There are sockets Open" << endl;
            vector<int> j = topic_to_Sub[path];
            for (int i = 0; i != j.size(); i++) {
                cout << "Sending Messages!" << endl;
                if (send(j[i],message,strlen(message),MSG_NOSIGNAL) <= -1) {
                    cout << "Message Not Sent!" << endl;
                    FD_CLR(j[i],&current_sockets);
                    who[j[i]].setT(ERR);
                }
            }
        } else {
            cout << "Making a New topic!\n";
            vector<int> j;
            topic_to_Sub[path] = j;
        }
    }
    int rt = zoo_exists(zh,path,1,NULL);
        if (rt != ZOK) {
            cout << "znode does not exist (yet)!" << endl;
        } else {
            cout << "Watch Relaunched!" << endl;
        }
}
char* getPath(const char *path) {
    int slashes = 0;
    int offset = 0;
    char* newPath = (char*)malloc(256*sizeof(char));
    while (slashes != 2 && path[offset] != '\0') {
        if (path[offset++] == '/') {
            slashes++;
        }
    }
    offset -= 1;
    int size = strlen(path+offset);
    memcpy(newPath,path+offset,size);
    return newPath;
}

char* setupMessage(int flag, char* message, char* path, zhandle_t *zh) {
    char buffer[2048] = {0};
    int buffer_len = 2047;
    char time_buf[9] = {0};
    int time_buf_len = 9;
    char bank_buf[5] = {0};
    int bank_buf_len = 5;
    long time = 0;
    int vault = 0;
    zoo_get(zh,"/time",0,time_buf,&time_buf_len,NULL);
    for (int i = 0; i != 8; i++) {
        time |= ((0xff)&time_buf[i]) << ((7-i)*8);
    }
    cout << "time: " << time << endl;
    zoo_get(zh,"/bank",0,bank_buf,&bank_buf_len,NULL);
    for (int i = 0; i != 4; i++) {
        vault |= ((0xff)&bank_buf[i]) << ((3-i)*8);
    }
    cout << "bank: " << vault << endl;
    int r = (rand() % 180)+60;
    int len = strlen(message);
    int total = r + len;
    vault -= total;
    int offset = 0;
    for (int i = 0; i != 4; i++) {
        buffer[offset++] |= (0xff & (r >> ((3-i)*8)));
    }
    for (int i = 0; i != 4; i++) {
        buffer[offset++] |= ((0xff) & (total >> ((3-i)*8)));
    }
    memcpy(buffer+offset,message,len);
    zoo_set(zh,path,buffer,buffer_len,-1);
    bzero(bank_buf, 0);
    for (int i = 0; i != 4; i++) {
        bank_buf[i] &= (0xff & (vault >> ((3-i)*8)));
    }
    zoo_set(zh,"/bank",bank_buf,5,-1);
    return NULL;
}
