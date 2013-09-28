#! /bin/bash

src=${1%%.*}
[ "$src" = "" ] && exit 1

as -o ${src}.o $1 && \
ld -o ${src} ${src}.o && \
rm -f ${src}.o
