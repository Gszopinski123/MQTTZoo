#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
using namespace std;
#define PORT 5001
char* firstMessage(char* topic, char* initialData);
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
    char * data = NULL;
    char *buf = firstMessage(topic,data);
    int connectfd = connect(socketfd,(struct sockaddr*)&address, sizeof(address));
    if (connectfd < 0) {
        printf("Connect Failure\n");
    }
    send(socketfd, buf, 512, 0);
    while (1) {
        //bzero(buf,512);
        cin.getline(buf,511);
        int sent = send(socketfd, buf, strlen(buf), 0);
        cout << "Sent! " << sent <<endl;
    }
    close(socketfd);
    return 0;
}
/* First 4 bits
 * Connect 1
 * ConnAck 2
 * Publish 3
 * Subscribe 8
 * Disconnect 14
 * Second 4 bits (flags)
 * DUP - Duplicate (3 bit)
 * QoS - Quality of service (bit 2 and 1)
 * RETAIN (bit 0)
 * Going to max the first message at 256 bytes 
 * */
char* firstMessage(char* topic, char* initialData) {
    char* message = (char*)malloc(sizeof(char)*256);
    int lenTopic = strlen(topic);
    bzero(message,256);
    int offset = 0;
    char *conn = 0;
    *conn = (*conn & 0) | 1;// first byte
    char *remainingLength = 0;
    *remainingLength = (*remainingLength & 0) | 8;
    char *protocolLength = 0;
    *protocolLength = (*protocolLength & 0) | 4;//1 bytes
    char protocol[5] = "MQTT";// 4 bytes
    char protocolLevel = '4';//1 byte
    //no connect flag
    //keep alive 2 bytes
    char keepalive[3] = "60";//total variable length = 8 bytes
    memcpy(message+offset,conn,1);
    offset += 1;
    memcpy(message+offset,remainingLength,4);
    offset += 1;
    memcpy(message+offset,protocolLength,1);
    offset += 1;
    memcpy(message+offset,protocol,4);
    offset += 4;
    memcpy(message+offset, topic, lenTopic);
    offset += lenTopic;
    memcpy(message+offset, "\n" ,1);
    offset += 1;
    if (initialData != NULL) {
        int lenInitData = strlen(initialData);
        memcpy(message+offset, "\n" ,1);
        offset += 1;
        memcpy(message+offset, "Data: ", 6);
        offset += 6;
        memcpy(message+offset,initialData,lenInitData);
        offset += lenInitData;
        memcpy(message+offset, "\n" ,1);
        offset += 1;
    }
    memcpy(message+offset,"\0",1);
    return message;
}
char* formatMessage(char * buf) {
    return NULL;
}
