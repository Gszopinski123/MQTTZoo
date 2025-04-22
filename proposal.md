# MQTTZoo Proposal
Author: Gabriel Szopinski  
I wish to implement MQTT with Zookeeper.

## Problem/Scenario
Multiple devices(IoT) are in different areas and they all need to communicate in an effective way and sometimes they have different communication policies (i.e., Synchronous and Asynchronous). There also may be a high number of devices trying to connect to a server to communicate which can overload the server, cause delays, long queues, and even server crashes. The need for coordination, reliablity and replication to handle high loads and efficient message handling is extremely important and a must have. I plan on solving this problem with implementing coordination, reliability and replication in a MQTT Broker Distributed System.

## General Overview
My proposal is to implement MQTT with Zookeeper. I believe that my use of Zookeeper and MQTT fits the criteria of being a distributed system because zookeeper has reliability, coordination, and redundancy/replication aspects of it's API while supporting multiple servers(brokers). 
I plan on using zookeeper to manage the topics, subscribers, publishers in znodes. I also plan on adding a banking system that keeps track of large messages and time to live mail drops. I will do this over containers
and a service in kubernetes. I plan on exposing the service so the application is available outside the cluster. The combination of MQTT brokers and Zookeeper should help manage network traffic while also carrying out coordination, reliabilty and replication. This distributed system should be able to solve the issue of network traffic, and eventually managing various communication policies.

## TL;DR
    My idea for this project is to be able to have multiple brokers respond and serve as many publishers and subscribers as possible. I want to use zookeeper because I think that having a replication of all the topics, subscribers, publishers, and bank is a great way to keep an application more reliable.   
    I decided zookeeper was the best method for managing my brokers because of the tree hierarchy already implemented within the API. I think that znodes can be very successiful as well due to the "watcher" feature which allows servers to watch for changes in different znodes and be alerted when there is a change. I think this feature can speed up mail drops to subscribers if a server is always "watching" specific znodes. I also want to setup a banking system so the server doesn't get overloaded. The system will generate a random time to live (between 30s - TBD I still have to decide). Each message will be given a credit amount based on the random TTL and size of the message and the credit amount will be withdrawn from the bank. The bank will not accept any more messages if there are not enough credits(no overdrafting the bank). each message stored in the topic znode will have a TTL, credit amount, and message. messages that have passed their TTL will be removed and their credit amount will return to the bank. These checks will be done periodically (more specifically when a subscriber connects or a new message is published to that topic or the bank makes a periodic request for the credits back). The equation will be something like (.04)x + random(0,100) where x is the number of bytes.    
    My version of MQTT will be implemented in C++ and will follow basic standarization of MQTT (minus UDP) this include packet standardization. A lot of the standardization will have to be second thought due to time constraints. for the most part clients will be able to make their topics and connect through the mqtt_pub.o command and will be able to subscribe through mqtt_sub.o.   
    A little more about Zookeeper I plan on following the Zookeeper reference docs on [Zookeeper Docs](https://zookeeper.apache.org/). I will use the C version to implement this.   
    More on my MQTT I will use basic socket connection with the basic packet configurations that can be found [here](https://cedalo.com/blog/mqtt-packet-guide/). I will try to add wildcards in, when I start configuring Zookeeper (i.e., #, +). I will use select to manage the socket file descriptors which is more light weight than threads.  
    My goal is to implement this in a about 2 weeks(starting on 4/18/25). I am going to leave myself about a week to implement Zookeeper. Timeline wise from 4/18/25 - 4/25/25 I will be working on and finishing MQTT and from 4/25/25 - (when the assignment is due) I wish to add Zookeeper.  

### Steps to implementation  
 1. I first plan on building the MQTT Publisher and Subscriber source code.
 2. While I design and manage the publisher and subscriber source code I will get a basic MQTT broker running.
 3. I plan on following the complete semantics of MQTT including packet standardization (minus UDP)
 4. Testing of MQTT Broker, Publisher, and Subscriber
 5. I will then Setup a kubernetes cluster with (one or two services) these service(s) will handle inbound communication and pod-to-pod communication
 6. I will test inbound communication first
 7. I will then implement zookeeper with the pod-to-pod communication
 8. Test pod-to-pod coummunication
 9. If I have time I will build an SDK kit to make publishing and subscribing easier and more usable in larger applications
 10. Finally, if time permitting I will add udp.


### Design Features
*  TCP
*  UDP (time permitting)
*  Zookeeper
*  MQTT
*  Kubernetes  
   *  pods  
   *  services  
   *  StatefulSet (most likely)  
*  I will be implementing this all in C++

### Professor Confirmation  
  * I did receive Confirmation from Professor Swany that my Project Idea was Ok.  

### Conclusion
My main goal is to get a handle on setting up this Distributed System. 
my next goal is I hope to complete this project in a little under two weeks. 
I will implement the MQTT Broker(TCP) onto kubernetes with Zookeeper with only the Publisher/Subscriber source code connection.
