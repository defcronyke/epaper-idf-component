#!/bin/bash
#
# epaper-idf-component
#
# Copyright (c) 2021 Jeremy Carter <jeremy@jeremycarter.ca>
#
# This code is released under the license terms contained in the
# file named LICENSE, which is found in the top-level folder in
# this project. You must agree to follow those license terms,
# otherwise you aren't allowed to copy, distribute, or use any 
# part of this project in any way.

epaper_idf_component_serve_web_exit() {
  cd "$1"
}

epaper_idf_component_serve_web() {
  pwd="$PWD"

  trap "epaper_idf_component_serve_web_exit $pwd" INT

  cd web

  EPAPER_IDF_WEBPACK_HOST=${EPAPER_IDF_WEBPACK_HOST:-"127.0.0.1"}
  EPAPER_IDF_WEBPACK_PORT=${EPAPER_IDF_WEBPACK_PORT:-3000}
  EPAPER_IDF_WEBPACK_MODE=${EPAPER_IDF_WEBPACK_MODE:-"development"}
  EPAPER_IDF_WEBPACK_CONTENT_BASE=${EPAPER_IDF_WEBPACK_CONTENT_BASE:-"."}

  ./node_modules/webpack/bin/webpack.js s --host ${EPAPER_IDF_WEBPACK_HOST} --port $EPAPER_IDF_WEBPACK_PORT --mode $EPAPER_IDF_WEBPACK_MODE --content-base $EPAPER_IDF_WEBPACK_CONTENT_BASE --watch-content-base --no-watch-options-stdin --inline --hot --compress --progress --history-api-fallback &

  EPAPER_IDF_WEBPACK_PID=$!
  EPAPER_IDF_WEBPACK_PARENT_PID=$$

  cd "$pwd"

  ./build-web.sh &

  wait $EPAPER_IDF_WEBPACK_PID

  cd "$pwd"
}

epaper_idf_component_serve_web $@
