#!/bin/bash
set -e
minikube delete
docker build -t gszopinski123/broker-gszopin:broker .
docker push gszopinski123/broker-gszopin:broker
minikube start
kubectl apply -f deploy.yaml