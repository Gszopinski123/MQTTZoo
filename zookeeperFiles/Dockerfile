# syntax=docker/dockerfile:1
FROM ubuntu:latest

WORKDIR /zookeeper/
COPY zoo.cfg .
RUN mkdir -p /zookeeper/zookeeper-data
RUN apt-get update
RUN apt-get install g++ wget tar openjdk-11-jdk libzookeeper-mt-dev vim -y
RUN apt-get install net-tools -y
RUN wget https://dlcdn.apache.org/zookeeper/zookeeper-3.9.3/apache-zookeeper-3.9.3-bin.tar.gz
RUN tar -xzvf apache-zookeeper-3.9.3-bin.tar.gz
RUN mv zoo.cfg apache-zookeeper-3.9.3-bin/conf
RUN chmod +x apache-zookeeper-3.9.3-bin/bin/zkServer.sh
CMD ["sleep" , "50000"]