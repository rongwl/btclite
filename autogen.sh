#!/bin/sh

set -eu

srcdir="$(dirname $0)"
cd "$srcdir"

if which glibtoolize 2>/dev/null
then
    glibtoolize && autoreconf --install --force --warnings=all
else
    libtoolize && autoreconf --install --force --warnings=all
fi
