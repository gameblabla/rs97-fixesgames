#! /bin/sh
#
# make-graphics.sh
# Make the graphics schemes for Chroma. Run from the resources directory.
#
# Copyright (C) 2010 Amf
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
./make-pieces-marble.pl pieces-marble.svg ../graphics/chroma-marble/
./make-base-marble.pl pieces-marble-base.svg ../graphics/chroma-marble/
./make-wall-marble.pl pieces-marble-wall.svg ../graphics/chroma-marble/
./crush.pl ../graphics/chroma-marble/
./make-pieces-neon.pl ../graphics/chroma-neon/
./crush.pl ../graphics/chroma-neon/
./make-pieces-zen.pl pieces-zen.svg ../graphics/chroma-zen/
./crush.pl ../graphics/chroma-zen/
