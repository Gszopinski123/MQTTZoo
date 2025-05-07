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
