# MQTTZoo
An MQTT implementation that uses Apache Zookeeper

## Getting Started

### Dependencies
 * C++ Compiler
 * Minikube or equivalent
 * Docker and access to Dockerhub
 * Kubernetes
### Start the Application
 * ``` bash automateBuild.sh ```
   * delete your current minikube
   * start minikube
   * apply the deploy.yaml
   * minikube tunnel to connect to the load balancer
* Once you have started the script this will take a minute
  * In General on a first attempt of the build this shouldn't take long, however I have had builds that have taken 7 minutes due to Zookeeper failing and restarting so give this a few minutes before using it.
* Once the build is up and running without failure for a little while compile the mqtt_pub.cpp and mqtt_sub.cpp.
  * These can take a command line arguments
    * -t is for the specific topic you want if you do not specify the topic, the default is /Testing
      * if you want a certain topic list it as is without any forward slashes. 
    * -h is if you need to select the host but if you are running this on a local machine it will default to localhost
* you should leave a terminal open for the publisher and subscriber
  * for the publisher to send a message the command line will do just hit enter with now characters if you wish to terminate the session
  * for the subscriber just leave the terminal open and ctrl-c to close the connection when you are done with it. 
### Details
This Application as stated as above and in the proposal is a Distributed Message Broker System that uses some of the semantics of MQTT and uses zookeeper for replication, coordination, reliability and Availability. 
More about the build... I use Docker for containerization and Kubernetes for Orchestration. I used two stateful sets that contain 3 pods per set. The first set is for the message brokers and the second set is for the zookeeper ensemble. I then use two headless services for DNS access and connectivity. I finally have a load balancer that distributes network traffic across the 3 brokers. All of the General information/code can be found in the deploy.yaml file. Now, I will move onto the code starting with the Publisher and Subscriber code. The code for the Publisher uses C++. The code uses basic sockets along with serialization to classify certain messages such as: connect, disconnect, publish, etc. the Publisher will also run forever until it is killed or you hit enter with no characters inputted. The code will take in new messages from the I/O. The publisher also handles a few messages while connecting such as: connack which the brokers sends to all new connections. Moving onto the Subscriber a similar build with the ability to handle multiple different messages such as publish and connack messages. The Subscriber will listen on a socket until killed or until ctrl-c is hit. For the Zookeeper builds I use a the basic command line server in which I run on in their own pods. The Ensemble runs in leader/follower configuration which is done through my zoo.cfg file. Now the broker build...
