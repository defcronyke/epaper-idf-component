#!/bin/bash

./build.sh && \
./node_modules/webpack/bin/webpack.js s --host 127.0.0.1 --port 3000 --content-base ../public --watch-content-base --entry ./index.js --compress --mode production --progress $@
