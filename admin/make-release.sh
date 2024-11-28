#!/bin/sh
#
# Create a release from `ref`.
#
# Example:
# 	./admin/make-release.sh atf-0.23

set -eux

ref=$1

cd "$(dirname "$(dirname "$0")")"

mkdir -p releases
release_root=$(realpath releases)

release_dir="${release_root}/${ref}"
release_artifact="${release_root}/${ref}.tar.gz"

rm -Rf "${release_dir}"
mkdir -p "${release_dir}"
git archive "${ref}" | tar xzvf - -C "${release_dir}"
cd "${release_dir}"
autoreconf -is
cd "${release_root}"
bsdtar \
    --exclude '*/.cirrus.yml' \
    --exclude '*/.github/*' \
    --exclude '*/.gitignore' \
    --exclude '*/.travis.yml' \
    --exclude '*/admin*' \
    --exclude '*/autom4te.cache/*' \
    --exclude '*/m4/libtool.m4' \
    --exclude '*/m4/ltoptions.m4' \
    --exclude '*/m4/ltsugar.m4' \
    --exclude '*/m4/ltversion.m4' \
    --exclude '*/m4/lt~obsolete.m4' \
    --uname "" --gname "" \
    --uid 0 --gid 0 \
    -cvpzf "${release_artifact}" "${ref}"
sha256 "${release_artifact##*/}" > "${release_artifact}.sha256"
