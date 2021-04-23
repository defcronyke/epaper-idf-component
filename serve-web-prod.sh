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

epaper_idf_component_serve_web_prod_exit() {
  cd "$1"
}

epaper_idf_component_serve_web_prod() {
  pwd="$PWD"

  trap "epaper_idf_component_serve_web_prod_exit $pwd" INT

  ./build-web.sh && \
  cd web && \
  ./node_modules/http-server/bin/http-server ../public -p 8080 -g -b --log-ip -r --no-dotfiles

  # Use this line instead to bypass the 404.html page and serve index.html for not found errors.
  # ./node_modules/http-server/bin/http-server ../public -p 8080 -g -b --log-ip -r --no-dotfiles --proxy http://localhost:8080?

  cd "$pwd"
}

epaper_idf_component_serve_web_prod $@
