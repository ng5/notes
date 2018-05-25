#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
w_implicitfallthrough=""
COMPILE_FLAGS=""
LINK_FLAGS=""

function checkprerequisite()
{
    local checkflags="$(gcc -Q --help=warning | grep -i implicit-fallthrough)"
    if [[ ! -z "$checkflags" ]]; then
        w_implicitfallthrough="-Wimplicit-fallthrough=0"
    fi
    local checkflags="$(gcc -Q --help=warning | grep -i implicit-fallthrough)"
    COMPILE_FLAGS=" -I$SCRIPT_DIR/INSTALL/include ${w_implicitfallthrough} "
    LINK_FLAGS=" -L$SCRIPT_DIR/INSTALL/lib64 "
    
    local checksign="$(gcc -Q --help=warning | grep -i sign-conversion)"
    if [[ ! -z "$checksign" ]]; then
        COMPILE_FLAGS=" $COMPILE_FLAGS  -Wno-sign-conversion"
    fi    
}
function cloneorpull()
{
    mkdir -p "$SCRIPT_DIR"/SOURCE
    local folder=$1
    local url=$2
    local tag=$3
    echo "SCRIPT_DIR:" $SCRIPT_DIR
    echo "folder:" $folder
    echo "url:" $url
    echo "tag:" $tag
    if [ -d "$SCRIPT_DIR/SOURCE/$folder" ]; then
        echo "folder exists:" $folder
        cd $SCRIPT_DIR/SOURCE/$folder && git pull
    else
        echo "folder DOES NOT exists:" $folder
        if [[ ! -z "$tag" ]]; then
            cd $SCRIPT_DIR/SOURCE/ && git clone --depth=1 --recursive $url --branch $tag && cd $folder
        else
            cd $SCRIPT_DIR/SOURCE/ && git clone --depth=1 --recursive $url && cd $folder
        fi
    fi
    cd $SCRIPT_DIR
}
function download()
{
    mkdir -p $SCRIPT_DIR/SOURCE
    mkdir -p $SCRIPT_DIR/DOWNLOAD
    local folder=$1
    local filename=$2
    local url=$3
    wget $url -O $SCRIPT_DIR/DOWNLOAD/$filename
    rm -rf $SCRIPT_DIR/SOURCE/$folder
    tar -xvf $SCRIPT_DIR/DOWNLOAD/$filename --directory $SCRIPT_DIR/SOURCE/
    cd $SCRIPT_DIR
}
function buildConfigureMake()
{
    local folder=$1
    local extraargs=$2
    cd $SCRIPT_DIR/SOURCE/$folder
    ./configure CFLAGS="${COMPILE_FLAGS}" CXXFLAGS="${COMPILE_FLAGS}" LDFLAGS="$LINK_FLAGS" --prefix=$SCRIPT_DIR/INSTALL $extraargs
    make -j 8 && make prefix=$SCRIPT_DIR/INSTALL PREFIX=$SCRIPT_DIR/INSTALL install
    cd $SCRIPT_DIR
}
function buildAutoGenConfigureMake()
{
    local folder=$1
    local extraargs=$2
    cd $SCRIPT_DIR/SOURCE/$folder
    ./autogen.sh
    ./configure CFLAGS="${COMPILE_FLAGS}" CXXFLAGS="${COMPILE_FLAGS}" LDFLAGS="$LINK_FLAGS" --prefix=$SCRIPT_DIR/INSTALL $extraargs
    make -j 8 && make prefix=$SCRIPT_DIR/INSTALL PREFIX=$SCRIPT_DIR/INSTALL install
    cd $SCRIPT_DIR
}
function buildMake()
{
    local folder=$1
    local extraargs=$2
    cd $SCRIPT_DIR/SOURCE/$folder
    make CFLAGS="${COMPILE_FLAGS}" CXXFLAGS="${COMPILE_FLAGS}" -j 8 && make prefix=$SCRIPT_DIR/INSTALL PREFIX=$SCRIPT_DIR/INSTALL $extraargs install
    cd $SCRIPT_DIR
}
function buildCMake()
{
    local folder=$1
    local extraargs=$2
    cd $SCRIPT_DIR/SOURCE/$folder
    mkdir -p build && cd build
    echo "=======================>"$COMPILE_FLAGS
    cmake .. -Wno-dev -DCMAKE_CXX_FLAGS="${COMPILE_FLAGS}" -DCMAKE_INSTALL_PREFIX=$SCRIPT_DIR/INSTALL $extraargs
    make CFLAGS="${COMPILE_FLAGS}" CXXFLAGS="${COMPILE_FLAGS}" -j 8 && make install
    cd $SCRIPT_DIR
}
function buildMongoCXX()
{
    local folder=$1
    local extraargs=$2
    cd $SCRIPT_DIR/SOURCE/$folder
    mkdir -p build && cd build
    cmake .. -DCMAKE_CXX_FLAGS="${COMPILE_FLAGS}" -DCMAKE_INSTALL_PREFIX=$SCRIPT_DIR/INSTALL $extraargs
    make EP_mnmlstc_core
    make CFLAGS="${COMPILE_FLAGS}" CXXFLAGS="${COMPILE_FLAGS}" -j 8 && make install
    cd $SCRIPT_DIR
}
function buildboost()
{
    local folder=$1
    cd $SCRIPT_DIR/SOURCE/$folder
    ./bootstrap.sh --prefix=$SCRIPT_DIR/INSTALL
    ./b2 install
    cd $SCRIPT_DIR
}
function showhelp()
{
    printf "please choose ALL or (one at a time) from the list below\n "
    printf "./build_external.sh [ALL|boost|armadillo|cgal|gtest|rapidjson|pcg|spdlog|uriparser|staticjson|uws|xxhash|lucene++|mongoc|mongocxx|zmq|czmq|quantlib]\n"
}
function install3rdparty()
{
    local validselection=0
    local package=$1
    if [ "$package" = "" ]; then
        showhelp
        exit
    fi
    if [[ "$package" = "ALL" || "$package" = "boost" ]]; then
        validselection=1
        printf "==> installing [boost]\n"
        download "boost_1_66_0" "boost_1_66_0.tar.gz" "https://dl.bintray.com/boostorg/release/1.66.0/source/boost_1_66_0.tar.gz"
        buildboost "boost_1_66_0"
    fi
    if [[ "$package" = "ALL" || "$package" = "armadillo" ]]; then
        validselection=1
        printf "==> installing [armadillo]\n"
        download "armadillo-8.400.0" "armadillo-8.400.0.tar.xz" "http://sourceforge.net/projects/arma/files/armadillo-8.400.0.tar.xz"
        buildCMake "armadillo-8.400.0"
    fi
    if [[ "$package" = "ALL" || "$package" = "cgal" ]]; then
        validselection=1
        printf "==> installing [cgal]\n"
        download "CGAL-4.11.1" "CGAL-4.11.1.tar.xz" "https://github.com/CGAL/cgal/releases/download/releases%2FCGAL-4.11.1/CGAL-4.11.1.tar.xz"
        buildCMake "CGAL-4.11.1" "-DBOOST_ROOT=$SCRIPT_DIR/INSTALL"
    fi
    if [[ "$package" = "ALL" || "$package" = "gtest" ]]; then
        validselection=1
        printf "==> installing [gtest]\n"
        download "googletest-release-1.8.0" "release-1.8.0" "https://github.com/google/googletest/archive/release-1.8.0.tar.gz"
        buildCMake "googletest-release-1.8.0"
    fi
    if [[ "$package" = "ALL" || "$package" = "rapidjson" ]]; then
        validselection=1
        printf "==> installing [rapidjson]\n"
        download "rapidjson-1.1.0" "v1.1.0.tar.gz" "https://github.com/Tencent/rapidjson/archive/v1.1.0.tar.gz"
        buildCMake "rapidjson-1.1.0" " -DRAPIDJSON_BUILD_EXAMPLES=OFF -DRAPIDJSON_BUILD_TESTS=OFF "
    fi
    if [[ "$package" = "ALL" || "$package" = "pcg" ]]; then
        validselection=1
        printf "==> installing [pcg]\n"
        cloneorpull "pcg-cpp" "https://github.com/ng5/pcg-cpp"
        buildMake "pcg-cpp"
    fi
    if [[ "$package" = "ALL" || "$package" = "spdlog" ]]; then
        validselection=1
        printf "==> installing [spdlog]\n"
        download "spdlog-0.16.3" "v0.16.3.tar.gz" "https://github.com/gabime/spdlog/archive/v0.16.3.tar.gz"
        buildCMake "spdlog-0.16.3"
    fi
    if [[ "$package" = "ALL" || "$package" = "uriparser" ]]; then
        validselection=1
        printf "==> installing [uriparser]\n"
        download "uriparser-uriparser-0.8.5" "uriparser-0.8.5" "https://github.com/uriparser/uriparser/archive/uriparser-0.8.5.tar.gz"
        buildAutoGenConfigureMake "uriparser-uriparser-0.8.5" " --disable-test --disable-doc"
    fi
    if [[ "$package" = "ALL" || "$package" = "staticjson" ]]; then
        validselection=1
        printf "==> installing [staticjson]\n"
        cloneorpull "StaticJSON" "https://github.com/ng5/StaticJSON"
        buildCMake "StaticJSON" "-DEXTERNAL_INSTALL_LOCATION="$SCRIPT_DIR"/INSTALL"
    fi
    if [[ "$package" = "ALL" || "$package" = "uws" ]]; then
        validselection=1
        printf "==> installing [uws]\n"
        cloneorpull "uWebSockets" "https://github.com/ng5/uWebSockets.git"
        buildMake "uWebSockets"
    fi
    if [[ "$package" = "ALL" || "$package" = "xxhash" ]]; then
        validselection=1
        printf "==> installing [xxhash]\n"
        cloneorpull "xxHash" "https://github.com/Cyan4973/xxHash.git"
        buildMake "xxHash"
    fi
    if [[ "$package" = "ALL" || "$package" = "lucene++" ]]; then
        validselection=1
        printf "==> installing [lucene++]\n"
        cloneorpull "LucenePlusPlus" "https://github.com/luceneplusplus/LucenePlusPlus.git"
        buildCMake "LucenePlusPlus" "-DENABLE_TEST=OFF -DENABLE_DEMO=OFF"
    fi
    if [[ "$package" = "ALL" || "$package" = "mongoc" ]]; then
        validselection=1
        printf "==> installing [mongoc]\n"
        download "mongo-c-driver-1.9.4" "mongo-c-driver-1.9.4" "https://github.com/mongodb/mongo-c-driver/releases/download/1.9.4/mongo-c-driver-1.9.4.tar.gz"
        buildConfigureMake "mongo-c-driver-1.9.4" " --disable-automatic-init-and-cleanup"
    fi
    if [[ "$package" = "ALL" || "$package" = "mongocxx" ]]; then
        validselection=1
        printf "==> installing [mongocxx]\n"
        download "mongo-cxx-driver-r3.2.0" "r3.2.0" "https://github.com/mongodb/mongo-cxx-driver/archive/r3.2.0.tar.gz"
        buildMongoCXX "mongo-cxx-driver-r3.2.0" "-DCMAKE_BUILD_TYPE=Release"
    fi
    if [[ "$package" = "ALL" || "$package" = "zmq" ]]; then
        validselection=1
        printf "==> installing [zmq]\n"
        cloneorpull "libzmq" "https://github.com/zeromq/libzmq" "v4.2.5"
        buildCMake "libzmq"
    fi
    if [[ "$package" = "ALL" || "$package" = "czmq" ]]; then
        validselection=1
        printf "==> installing [libczmq]\n"
        cloneorpull "czmq" "https://github.com/zeromq/czmq.git" "v4.1.1"
        buildAutoGenConfigureMake "czmq" " "
    fi
    if [[ "$package" = "ALL" || "$package" = "quantlib" ]]; then
        validselection=1
        printf "==> installing [quantlib]\n"
        download "QuantLib-1.12" "QuantLib-1.12.tar.gz" "https://bintray.com/quantlib/releases/download_file?file_path=QuantLib-1.12.tar.gz"
        buildConfigureMake "QuantLib-1.12" " --with-boost-include=$SCRIPT_DIR/INSTALL/include --with-boost-lib=$SCRIPT_DIR/INSTALL/lib --enable-thread-safe-observer-pattern --enable-thread-safe-singleton-init "
    fi
    if [ "$validselection" = 0 ]; then
        printf "==> Invalid package [$package]\n"
        showhelp
    fi
}
checkprerequisite
install3rdparty "$@"
