# MQTTZoo

## General Overview
My proposal is to implement MQTT with zookeeper. I believe that my use of Zookeeper and MQTT fits the criteria of being a distributed system because zookeeper has reliability, coordination, and redundancy/replication aspects of it's API. 
I plan on using zookeeper to manage the topics, subscribers, publishers in znodes. I also plan on adding a banking system that keeps track of large messages and time to live mail drops. 

## TL:DR
My idea for this project is to be able to have multiple brokers respond and serve as many publishers and subscribers as possible. I want to use zookeeper because I think that having a replication of all the topics, subscribers, publishers, and bank is a cool way to keep
an application more reliable. I decided zookeeper was the best method for managing my brokers because of the tree hierarchy already implemented within the leader, and other servers. I think that znodes can be very successiful as well due to the "watcher" feature which allows
servers to watch for changes in different znodes and be alerted when there is a change. I think this feature can speed up mail drops to subscribers if a server is always "watching" specific znodes. I also want to setup a banking system so the server doesn't get overloaded.
The system will generate a random time to live (between 30 - 2 min have to decide). Each message will be given a credit amount based on the random TTL and size of the message and the credit amount will be withdrawn from the bank. The bank will not accept any more messages if
there are not enough credits(no overdrafting the bank). each message stored in the topic znode will have a TTL, credit amount, and message. messages that have passed their TTL will be removed and their credit amount will return to the bank.

### Steps to implementation  
 1. I first plan on building the MQTT Publisher and Subscriber source code.
 2. While I design and manage the publisher and subscriber source code I will get a basic MQTT broker running.
 3. I will then Setup a kubernetes cluster with (one or two services) these service(s) will handle inbound communication and pod-to-pod communication
 4. I will test inbound communication first
 5. I will then implement zookeeper with the pod-to-pod communication
 6. Test pod-to-pod coummunication
 7. If I have time I will build an SDK kit to make publishing and subscribing easier and more usable in larger applications


### Design Features
*  TCP
*  I will be using Zookeeper
*  MQTT
*  kubernetes
*  pods
*  services
*  StatefulSet (most likely)
*  I will be implementing this all in C++

### Professor Confirmation  
  * I did receive Confirmation from Professor Swany that my Project Idea was Ok.  
