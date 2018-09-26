#!/bin/sh

echo "cd libgambatte && scons"
(cd libgambatte && scons -Q target=od) || exit

echo "cd gambatte_sdl && scons"
(cd gambatte_sdl && scons -Q target=od)
