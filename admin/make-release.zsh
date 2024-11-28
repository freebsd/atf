#!/usr/bin/env zsh

set -eux

tag=$1

mkdir -p releases
release_root=$(realpath releases)

release_dir=$release_root/$tag
release_artifact=$release_root/$tag.tar.gz

mkdir -p $release_dir
git archive $tag | tar xzvf - -C $release_dir
pushd $release_dir
autoreconf -is
popd
pushd $release_root
tar -cvpf $release_artifact -C $release_dir .
sha256 ${release_artifact##*/} > $release_artifact.sha256
