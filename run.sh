#!/bin/bash

export LD_LIBRARY_PATH=./lib:$LD_LIBRARY_PATH 

#if there is more than one qtmdirver, the port of each qtmdirver must be defferent!
./qtmdriver 5000
