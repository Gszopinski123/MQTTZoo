# syntax=docker/dockerfile:1
FROM ubuntu:latest

WORKDIR /mqtt_broker/
COPY broker/ .
RUN apt-get update
RUN apt-get install g++ net-tools wget autoconf make libcppunit-dev libtool pkg-config libzookeeper-mt-dev -y
RUN apt update
RUN apt install maven default-jdk -y
RUN wget https://dlcdn.apache.org/zookeeper/zookeeper-3.8.4/apache-zookeeper-3.8.4.tar.gz
RUN tar -xvf apache-zookeeper-3.8.4.tar.gz
RUN (cd apache-zookeeper-3.8.4/zookeeper-jute && mvn compile)
RUN (cd apache-zookeeper-3.8.4/zookeeper-client/zookeeper-client-c && autoreconf -if && ./configure && make && make install)
RUN g++ mqtt_broker.cpp -o mqtt_broker.o -lzookeeper_mt

CMD ["sleep" , "50000"]