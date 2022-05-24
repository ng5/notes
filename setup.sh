#!/usr/bin/env bash
DIR="$(
  cd "$(dirname "$0")" || exit >/dev/null 2>&1
  pwd -P
)"
if ! command -v cmake &>/dev/null; then
  echo "cmake not installed"
  exit
fi
if ! command -v conda &>/dev/null; then
  echo "conda not installed"
  exit
fi
envDirectory=$(conda info | grep -i "envs directories" | awk -F':' '{print $2}' | xargs)
if [ ! -d "$envDirectory" ]; then
  echo "conda env directory does not exist, check if conda installation is correct"
  exit
fi

function buildEnv() {
  local envName="$1"
  local version="$2"
  # shellcheck disable=SC2155
  conda create -n "$envName" python="$version" -y
  git init "$envDirectory/$envName"
}

function buildCleanPythonEnvs() {
  buildEnv py37 3.7
  buildEnv py38 3.8
  buildEnv py39 3.9
  buildEnv py310 3.10
}

function buildPackage() {
  local pyEnv="$1"
  source ~/miniconda3/bin/activate "$pyEnv"
  python --version
  cd "$DIR" || exit
  python setup.py bdist_wheel
}

function buildAllPackages(){
  rm -rf "$DIR/build" "$DIR/dist" "$DIR/*.egg-info"
  buildPackage py37
  buildPackage py38
  buildPackage py39
  buildPackage py310
}
USAGE="Usage: Either 1) create conda environments (one time only): ./setup.sh -n  or 2) create package: ./setup.sh"
while getopts ":n:" opt; do
  case $opt in
  n)
    buildCleanPythonEnvs
    exit 0
    ;;
  \?)
    echo "Invalid option -$OPTARG" >&2
    echo $USAGE
    exit 1
    ;;
  esac
done
buildAllPackages
