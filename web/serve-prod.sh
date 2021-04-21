#!/bin/bash

./build.sh && \
./node_modules/http-server/bin/http-server ../public -p 8080 -g -b --log-ip -r --no-dotfiles --proxy http://localhost:8080?
