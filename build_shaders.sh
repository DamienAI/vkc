#!/bin/bash

GLSL_FLAGS="-e main -V"


function build {
  for s in $1/*.$2.glsl
  do
    if [ ! -f $s ]; then
      return 0
    fi
    target_dir=`dirname $s`
    target=`basename $s .glsl`.spv
    echo "Compiling $3 shader $s to ${target_dir}/${target}"
    glslangValidator $GLSL_FLAGS -S $2 -o ${target_dir}/${target} $s
  done
}

build $1 vert Vertex
build $1 frag Fragment
build $1 comp Compute
