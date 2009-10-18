#!/bin/bash

## VARIABLES ######################################################################################################################
USAGE="Usage: bthServer.sh [start|stop]
  where
   start ...  Starts the Server
   stop  ...  Sends the signal to stop the Server"

case "$1" in
    start)  './../Linux GCC/WebServer';;
    stop)  'kill -SIGUSR1 ???';;
    *)  echo "$USAGE" && exit 1;;
esac
