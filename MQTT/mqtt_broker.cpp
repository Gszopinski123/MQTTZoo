#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#define PORT 5001
using namespace std;


enum messagerType { PUB, SUB, ERR };
class Messager {
    private:
        messagerType t;
        string topic;
        string ipaddr;
        int fd;
    public:
        Messager(messagerType it, string itopic) {
            t = it;
            for (int i = 0; i != itopic.length(); i++)
                topic += itopic[i];
        }
        void toString() {
            cout << "Type: " << t << ", topic: "<< topic << ", From: "<< ipaddr << endl;
        }
        void setIp(char iipaddr[]) {
            for (int i = 0; i != strlen(iipaddr); i++) {
                ipaddr += iipaddr[i];
            }
        }
        void setFd(int ifd) {
            fd = ifd;
        }
        int getFd() {
            return fd;
        }
};
void* parseConnection(char *buf);
void* acceptConnection(int sockfd);
int setupServer(void);
int main(int argc, char** argv) {
    fd_set current_sockets, ready_sockets;
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
                    m->toString();
                } else {
                    bzero(buf,511);
                    int bytes = recv(i,buf,511,0);
                    if (strlen(buf) != 0) {
                        cout << "Message Received!" << " Bytes Received: " << bytes <<endl;
                    cout << buf << endl;
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
