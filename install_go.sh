#!/usr/bin/env bash
LATEST="$(curl https://golang.org/VERSION?m=text)"
URL=https://dl.google.com/go/"$LATEST".linux-amd64.tar.gz
set -x
wget -q -O- "$URL" | sudo tar -C /usr/local -xz
