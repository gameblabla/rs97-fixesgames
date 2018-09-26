#!/bin/sh

echo "cd libgambatte && scons"
(cd libgambatte && scons -Q target=gcw0) || exit

echo "cd gambatte_sdl && scons"
(cd gambatte_sdl && scons -Q target=gcw0)
