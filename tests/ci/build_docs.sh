#!/bin/bash
#
# CI job to run the documentation build

set -ev

cd $GITHUB_WORKSPACE/doc

SPHINXOPTS="-W" make html
