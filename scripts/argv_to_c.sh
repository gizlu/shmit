#!/bin/sh
# dead simple script for printing argv as c array. Inspired by ImgToC.c
# License: Do what the fuck you want, just dont blame me for anything
usage()
{
  printf 'Usage: %s {--with-argc|-h|--help} command [command_args...]\n' "$0" >&2
  exit 1
}

[ "$1" = '-h' ] || [ "$1" = '--help' ] || [ "$#" -lt 1 ] && usage

if [ "$1" = '--with-argc' ]; then
    [ "$#" -lt 2 ] && usage
    shift; printf 'int _argc = %d;\n' "$#"
fi

printf 'char** _argv = {'
while [ $# -gt 1 ]; do
  printf '"%s", ' "$1"
  shift
done
printf '"%s", NULL};\n' "$1"
