#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
using namespace std;
#define PORT 5001
char* firstMessage(void);
int parseAck(char* buf);
char* formatMessage(int code ,char * buffer, char * topic);
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
    char *buf = firstMessage();
    char buffer[512];
    int connectfd = connect(socketfd,(struct sockaddr*)&address, sizeof(address));
    if (connectfd < 0) {
        printf("Connect Failure\n");
    }
    send(socketfd, buf, 512, 0);
    int numOfbytes = recv(socketfd, buffer, 511,0);
    if (numOfbytes == 0) {
        printf("Failed Message receive!\n");
        exit(1);
    }
    if (!parseAck(buffer)) {
        printf("Failed Connection!\n");
        exit(1);
    }
    send(socketfd,formatMessage(8,"","Testing/"),511,0);
    while (1) {
        bzero(buf,512);
        int bytes = recv(socketfd,buf,511,0);
        buf[bytes] = '\0';
        printf("Bytes: %d, Message: %s\n",bytes,buf);
    }
    close(socketfd);
    return 0;
}
char* firstMessage(void) {
    char *message = (char*)malloc(256*sizeof(char));
    bzero(message,256);    
    unsigned char conn = 1;
    int offset = 0;
    //no flags
    message[offset++] = conn;
    unsigned int remainingLength = 10;
    message[offset++] = 0xff & remainingLength;
    message[offset++] = 0xff & (remainingLength >> 8);
    message[offset++] = 0xff & (remainingLength >> 16);
    message[offset++] = 0xff & (remainingLength >> 24);
    //no payload data
    unsigned char protocolLength = 4;
    message[offset++] = protocolLength;
    char protocol[5] = "MQTT";// 4 bytes
    memcpy(message+offset,protocol,4);
    offset += 4;
    unsigned char protocolLevel = 4;
    message[offset++] = protocolLevel;
    unsigned char connectFlag = 0;
    message[offset++] = connectFlag;
    unsigned short keepalive = 60; 
    message[offset++] = 0xff & (keepalive >> 0);
    message[offset++] = 0xff & (keepalive >> 8);
    return message;
}

int parseAck(char* buf) {
    unsigned char connect = buf[0];
    if (!(connect & 0x20)) {
        printf("Wrong Message!\n");
        return 0;
    }
    unsigned char sizeOfMes = buf[1];
    unsigned char session = buf[2];
    unsigned char returnCode = buf[3];
    int sessionReturn = (0xf & session);
    
    return sessionReturn;
}
char* formatMessage(int code ,char * buffer, char * topic) {
    char * buf = (char*)malloc(sizeof(char)*512);
    bzero(buf,512);
    int offset = 0;
    if (code == 3) {
        buf[offset++] |= (code << 4);
        short topicLen = strlen(topic); // check if valid topic (optional)
        int payloadLen = strlen(buffer);
        int length = topicLen+payloadLen; // sizeof(buf);
        for (int i = 0; i != 4; i++)
            buf[offset++] = length >> ((3-i) *8);
        
        for (int i = 0; i != 2; ++i)
            buf[offset++] = topicLen >> ((1-i) * 8);
        memcpy(buf+offset,topic,topicLen);
        offset += topicLen;
        memcpy(buf+offset,buffer,payloadLen);
        return buf;
    } else if (code == 8) {
        buf[offset++] |= (code << 4);
        int variableLength = 2 + strlen(topic);
        for (int i = 0; i != 4; i++) {
            buf[offset++] = variableLength >> ((3-i) *8);
        }
        short numOfRequests = 1;
        for (int i = 0; i != 2; ++i) {
            buf[offset++] = numOfRequests >> ((1-i) *8);
        }
        short topicLen = strlen(topic);
        for (int i = 0; i != 2; ++i) {
            buf[offset++] = topicLen >> ((1-i) *8);
        }
        memcpy(buf+offset,topic,topicLen);
        return buf;
    }
    return NULL;
}
