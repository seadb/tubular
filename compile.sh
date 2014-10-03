#!/bin/tcsh

set COMMAND = emcc
set INPUT = ""
$COMMAND -v >& /dev/null
if ($status != "0") then
  set COMMAND = python
  set INPUT = /Users/charles/Local/emsdk_portable/emscripten/1.16.0/emcc
endif

if ($2 == "") then
  set OUTDIR = /Users/charles/Sites/EmCanvas/
else
  set OUTDIR = $2
endif

$COMMAND $INPUT -std=c++11 --js-library ../../KinEmAtic/libs/library_kinetic.js -g4 -Werror -DEMK_DEBUG $1.cc -o $1.js -s EXPORTED_FUNCTIONS="['_emkMain', '_emkJSDoCallback']"
cp $1.js $OUTDIR
cp $1.html $OUTDIR
