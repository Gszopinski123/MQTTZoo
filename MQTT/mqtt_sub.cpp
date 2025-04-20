#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
using namespace std;
#define PORT 5001
char* firstMessage(char* topic);
int main(int argc, char** argv) {
    sockaddr_in address;
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0) {
        printf("Socket Failure!\n");
        exit(1);
    }
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    address.sin_family = AF_INET;
    char * topic = (char*)"hello/there/does/this/work";
    char *buf = firstMessage(topic);
    int connectfd = connect(socketfd,(struct sockaddr*)&address, sizeof(address));
    if (connectfd < 0) {
        printf("Connect Failure\n");
    }
    send(socketfd, buf, 512, 0);
    close(socketfd);
    return 0;
}
char* firstMessage(char* topic) {
    char* message = (char*)malloc(sizeof(char)*512);
    int lenTopic = strlen(topic);
    bzero(message,512);
    int offset = 0;
    memcpy(message+offset,"Subscriber",10);
    offset += 10;
    memcpy(message+offset, "\n" ,1);
    offset += 1;
    memcpy(message+offset,"Topic: ",7);
    offset += 7;
    memcpy(message+offset, topic, lenTopic);
    offset += lenTopic;
    memcpy(message+offset, "\n" ,1);
    offset += 1;
    offset += 1;
    memcpy(message+offset,"\0",1);
    return message;
}
