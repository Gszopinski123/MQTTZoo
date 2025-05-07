# MQTTZoo
An MQTT implementation that uses Apache Zookeeper

### Before Getting Started
* please allow at least 5 minutes after building the system.
  * The zookeeper ensemble tends to fail mulitple times before configuring
  * I also have set the build for the brokers to wait 5 minutes before trying to to connect to the ensemble.
  * Carry on!
  
## Getting Started
* download all the necessary files [here](https://github.com/Gszopinski123/MQTTZoo/archive/refs/heads/main.zip) (warning: zip download)
* you can find the broker code in the broker subdirectory.
* you can find the client code in the client subdirectory.
### Dependencies
 * C++ Compiler
 * Minikube or equivalent
   * if you are not using minikube you will need to find the equivalent to minkibe tunnel. 
 * Docker and access to Dockerhub
 * Kubernetes
### Start the Application
 * ``` bash build.sh ```
   * delete your current minikube
   * start minikube
   * apply the deploy.yaml
   * minikube tunnel to connect to the load balancer
* Once you have started the script this will take a minute
  * In General on a first attempt of the build this shouldn't take long, however I have had builds that have taken 7 minutes due to Zookeeper failing and restarting so give this a few minutes before using it. I have set the connection between the zookeeper server and the broker to happen 5 minutes after starting each broker pod.
* Once the build is up and running without failure for a little while compile the mqtt_pub.cpp and mqtt_sub.cpp.
  * These can take command line arguments
    * -t is for the specific topic you want if you do not specify the topic, the default is /Testing
      * if you want a certain topic, list it as is without the first forward slash and any slashes at the end of the topic. 
    * -h is if you need to select the host but if you are running this on a local machine it will default to localhost
* you should leave a terminal open for the publisher and subscriber
  * for the publisher to send a message the command line will do, just hit enter with after the characters you want to send, if you wish to terminate the session just hit enter with no characters.
  * for the subscriber just leave the terminal open and ctrl-c to close the connection when you are done with it.
* if you wish to see connection and some general print out messages you can shell into any of the brokers and they will constantly printout messages/Debugging messages.
### Details
This Application as stated as above and in the proposal is a Distributed Message Broker System that uses some of the semantics of MQTT and uses Zookeeper for replication, coordination, reliability and Availability. 
More about the build... I use Docker for containerization and Kubernetes for Orchestration. I used two stateful sets that contain 3 pods per set. The first set is for the message brokers and the second set is for the zookeeper ensemble. I then use two headless services for DNS access and connectivity. I finally have a load balancer that distributes network traffic across the 3 brokers via the minikube tunnel. All of the General information/code can be found in the deploy.yaml file. 
Now, I will move onto the code starting with the Publisher and Subscriber code. The code for the Publisher uses C++. The code uses basic sockets along with serialization to classify certain messages such as: connect, disconnect, publish, etc. the Publisher will also run forever until it is killed or you hit enter with no characters inputted. The code will take in new messages from the I/O. The publisher also handles a few messages while connecting such as: connack which the brokers sends to all new connections. Moving onto the Subscriber a similar build with the ability to handle multiple different messages such as publish and connack messages. The Subscriber will listen on a socket until killed or until ctrl-c is hit. 
For the Zookeeper builds I use a the basic command line server in which I run on in their own pods. The Ensemble runs in leader/follower configuration which is done through my zoo.cfg file. 
Now the broker build... The broker is built using a lot of the c standard libraries but also uses objects to keep track of publisher and subscriber data. I also use the zookeeper client API to interact with the zookeeper servers. I use a basic implementation of a message broker system through the use of sockets, serialized messages and select to easily identify who needs to be served. I use the watcher feature to keep track of who needs to be updated and when. The basic configuration starts with setting up zookeeper by connecting through its DNS, each broker connects to a different zookeeper server although more than one can connect to a zookeeper server. Next, the code will intialize some of the starting znodes that includes the Bank, which will store the amount of credit left to be used to keep the znodes less cluttered and have better efficiency, time which will have a standard time that all the brokers should use and I am completely aware that each server may be off by a few seconds but with a TTL of 60 seconds+ this should not be that big of an issue and finally a topic znode, this node is just to keep track of where the topics go. The code then will setup the basic server through the socket and will start to accept connections. When a message comes through from a new connection it will be acknowledge through a connect message and then be sent a connack message to tell the new connection they are connected. when a message from an existing connection comes through it will be determined to be a publish, subscribe, disconnect, or unsubscribe message. the message will be parsed and then be placed into its assorted category this is where the Messager class comes into play because I do not have stateful publishers but I do have stateful subscribers each connection once they send a message will create a new object that will store topic, type(sub,pub,uns,err), socketfd, its ip address and if the connection was good or not. The actually class can be checked in the broker header file. When a new publisher or subscriber comes in with a topic that has not been created a new znode will be created and a watch will be placed upon it. if a subscriber connects to a different server and a topic has been created a watch will be placed on the existing znode. Each message before it is sent of will contain a TTL and a credit amount that will be stored on the znode. The broker will handle disconnects as well. Overall, the broker does most of the work but gets help from zookeeper to keep availbility open.

### Notes about the build/Proposal.
 * I did not have time to implement a system where a new connection will get all the currently available messages from that topic.
 * I also was not able to setup the banking system to be more accurate.
 * I also did not have time to setup wildcards.
