#define THREADED
#include <zookeeper/zookeeper.h>
#include <stdio.h>
#include <string.h>
#define zk_server "localhost:2181"
#define znode_data "Hello From Gabriel!"
#define top_level "/my_test"

int main(void) {
    zhandle_t *zh = zookeeper_init(zk_server,NULL,2000,0,0,0);
    
    if (zh == NULL)
        printf("Connection Failure!\n");
    else
        printf("Hello Zookeeper!\n");
    int ret = zoo_create(zh,top_level,znode_data,strlen(znode_data),&ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0);
    if (ret == ZOK) {
        printf("Znode creating success!\n");
    }
    char buffer[512];
    int buffer_len = 511;
    ret = zoo_get(zh,top_level, 0, buffer,&buffer_len, NULL);
    printf("%s\n",buffer); 
    zookeeper_close(zh);
    return 0;
}
