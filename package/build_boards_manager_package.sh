#!/bin/bash
set -o xtrace

[ -z "${REMOTE_URL}" ] && REMOTE_URL=https://github.com/earlephilhower/arduino-pico/releases/download

if [ ! -z "${manualversion}" ]; then

    # manual-made release based on $manualversion
    ver=${manualversion}
    plain_ver=${ver}
    visible_ver=${ver}

else

    # Extract the release name from a release

    # Default to draft tag name
    ver=$(basename $(jq -e -r '.ref' "$GITHUB_EVENT_PATH"))
    # If not available, try the publish tag name
    if [ "$ver" == "null" ]; then
        ver=$(jq -e -r '.release.tag_name' "$GITHUB_EVENT_PATH")
    fi
    # Fall back to the git description OTW (i.e. interactive)
    if [ "$ver" == "null" ]; then
        ver=$(git describe --tag)
    fi
    visible_ver=$ver
    plain_ver=$ver

    # Match 0.0.* as special-case early-access builds
    if [ "${ver%.*}" = 0.0 ]; then
        git tag -d ${ver}
        ver=`git describe --tag HEAD`
        plain_ver=$ver
    fi
fi

set -e

package_name=rp2040-$visible_ver
echo "Version: $visible_ver ($ver)"
echo "Package name: $package_name"

# Set REMOTE_URL environment variable to the address where the package will be
# available for download. This gets written into package json file.
if [ -z "$REMOTE_URL" ]; then
    REMOTE_URL="http://localhost:8000"
    echo "REMOTE_URL not defined, using default"
fi
echo "Remote: $REMOTE_URL"

if [ -z "$PKG_URL" ]; then
    if [ -z "$PKG_URL_PREFIX" ]; then
        PKG_URL_PREFIX="$REMOTE_URL/$visible_ver"
    fi
    PKG_URL="$PKG_URL_PREFIX/$package_name.zip"
fi
echo "Package: $PKG_URL"

pushd ..
# Create directory for the package
outdir=package/versions/$visible_ver/$package_name
srcdir=$PWD
rm -rf package/versions/$visible_ver
mkdir -p $outdir

# Some files should be excluded from the package
cat << EOF > exclude.txt
.git
.gitignore
.gitmodules
.travis.yml
package
doc
EOF
# Also include all files which are ignored by git
git ls-files --other --directory >> exclude.txt
# Now copy files to $outdir
rsync -a -L -K --exclude-from 'exclude.txt' $srcdir/ $outdir/
#mv $outdir/ArduinoCore-API/api $outdir/cores/rp2040/api
rm -rf $outdir/ArduinoCore-API
rm exclude.txt

# For compatibility, on OS X we need GNU sed which is usually called 'gsed'
if [ "$(uname)" == "Darwin" ]; then
    SED=gsed
else
    SED=sed
fi

# Do some replacements in platform.txt file, which are required because IDE
# handles tool paths differently when package is installed in hardware folder
cat $srcdir/platform.txt | \
$SED 's/^runtime.tools.pqt-.*.path=.*//g' | \
$SED 's/^tools.uf2conv.cmd=.*//g' | \
$SED 's/^#tools.uf2conv.cmd=/tools.uf2conv.cmd=/g' | \
$SED 's/^tools.picoprobe.cmd=.*//g' | \
$SED 's/^#tools.picoprobe.cmd=/tools.picoprobe.cmd=/g' | \
$SED "s/version=.*/version=$ver/g" |\
$SED -E "s/name=([a-zA-Z0-9\ -]+).*/name=\1($ver)/g"\
 > $outdir/platform.txt

# Put core version and short hash of git version into core_version.h
ver_define=`echo $plain_ver | tr "[:lower:].\055" "[:upper:]_"`
echo Ver define: $ver_define
echo \#define ARDUINO_RP2040_GIT_VER 0x`git rev-parse --short=8 HEAD 2>/dev/null` >$outdir/cores/rp2040/core_version.h
echo \#define ARDUINO_RP2040_GIT_DESC `git describe --tags 2>/dev/null` >>$outdir/cores/rp2040/core_version.h
echo \#define ARDUINO_RP2040_RELEASE_$ver_define >>$outdir/cores/rp2040/core_version.h
echo \#define ARDUINO_RP2040_RELEASE \"$ver_define\" >>$outdir/cores/rp2040/core_version.h

$SED -i 's/"version": .*/"version": "'$visible_ver'"/' $outdir/package.json

# Zip the package
pushd package/versions/$visible_ver
echo "Making $package_name.zip"
zip -qr $package_name.zip $package_name
rm -rf $package_name

# Calculate SHA sum and size
sha=`shasum -a 256 $package_name.zip | cut -f 1 -d ' '`
size=`/bin/ls -l $package_name.zip | awk '{print $5}'`
echo Size: $size
echo SHA-256: $sha

echo "Making package_rp2040_index.json"

jq_arg=".packages[0].platforms[0].version = \"$visible_ver\" | \
    .packages[0].platforms[0].url = \"$PKG_URL\" |\
    .packages[0].platforms[0].archiveFileName = \"$package_name.zip\""

if [ -z "$is_nightly" ]; then
    jq_arg="$jq_arg |\
        .packages[0].platforms[0].size = \"$size\" |\
        .packages[0].platforms[0].checksum = \"SHA-256:$sha\""
fi


cat $srcdir/package/package_pico_index.template.json | \
    jq "$jq_arg" > package_rp2040_index.json

#exit

if [ -z "prev_any_release" ]; then
    # Use Github API token, if available
    curl_gh_token_arg=()
    if [ ! -z "$CI_GITHUB_API_KEY" ]; then
        curl_gh_token_arg=(-H "Authorization: token $CI_GITHUB_API_KEY")
    fi
    # Get previous release name
    curl --silent "${curl_gh_token_arg[@]}" https://api.github.com/repos/earlephilhower/arduino-pico/releases > releases.json
    # Previous final release (prerelase == false)
    prev_release=$(jq -r '. | map(select(.draft == false and .prerelease == false)) | sort_by(.created_at | - fromdateiso8601) | .[0].tag_name' releases.json)
    # Previous release (possibly a pre-release)
    prev_any_release=$(jq -r '. | map(select(.draft == false)) | sort_by(.created_at | - fromdateiso8601)  | .[0].tag_name' releases.json)
    # Previous pre-release
    prev_pre_release=$(jq -r '. | map(select(.draft == false and .prerelease == true)) | sort_by(.created_at | - fromdateiso8601)  | .[0].tag_name' releases.json)
else
    prev_any_release=$prev_release
fi

echo "Previous release: $prev_release"
echo "Previous (pre-?)release: $prev_any_release"
echo "Previous pre-release: $prev_pre_release"

# Make all released versions available in one package (i.e. don't separate stable/staging versions)
base_ver=$prev_any_release

# Download previous release
echo "Downloading base package: $base_ver"
old_json=package_rp2040_index_stable.json
curl -L -o $old_json "https://github.com/earlephilhower/arduino-pico/releases/download/${base_ver}/package_rp2040_index.json"
new_json=package_rp2040_index.json
new_log=package_rp2040_index.log

set +e
# Merge the old and new
python3 ../../merge_packages.py $new_json $old_json > tmp

# additional json to merge (for experimental releases)
echo "Additional json package files: ${MOREJSONPACKAGES}"
for json in ${MOREJSONPACKAGES}; do
    if [ ! -z "$json" -a -r "$json" ]; then
        echo "- merging $json"
        python3 ../../merge_packages.py tmp $json > tmp2
        mv tmp2 tmp
    fi
done

mv tmp $new_json

# Verify the JSON file can be read, fail if it's not OK
set -e
cat $new_json | jq empty

git log $base_ver..HEAD --oneline | sed 's/\b / * /' | cut -f2- -d" " > $new_log

popd
popd

echo "All done"
