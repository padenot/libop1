#!/bin/sh

make doc
rsync --delete -r doc/html/* dist
git add dist
git commit -m"Update docs."
git subtree push --prefix dist origin gh-pages

