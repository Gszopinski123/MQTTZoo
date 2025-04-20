#include <stdio.h>
#include <iostream>
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
        Messager() : t(ERR), topic(""), ipaddr(""), fd(-1) {}
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
        int getFd(void) {
            return fd;
        }
        int gett(void) {
            return t;
        }
        string getTopic(void) {
            return topic;
        }
};
