# MQTTZoo
An MQTT implementation that uses Apache Zookeeper

### Proprosal  
  * I will use Apache Zookeeper to manage the mqtt broker.
  * I will store the data received from publishers on znodes
    * Znodes will also store addresses to send data back to subscribers and will hold publisher address for fault tolerance
    * all and all znodes will hold addresses, topics, published data, maybe more?
