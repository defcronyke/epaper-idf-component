#!/bin/bash

# ./build.sh && \
./node_modules/webpack/bin/webpack.js s --host 127.0.0.1 --port 3000 --mode production --hot --compress --progress $@
