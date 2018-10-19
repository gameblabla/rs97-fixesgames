#! /usr/bin/perl
#
# make-base-marble.pl
# Create the WALL pieces for the chroma-marble graphics scheme.
#
# Requirements: ImageMagick, Inkscape
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

if($#ARGV != 1)
{
    print STDERR "Usage: make-wall-marble.pl <pieces.svg> <directory>\n";
    exit(1);
}

$file_svg = $ARGV[0];
$dir_out = $ARGV[1];

if(! -e $file_svg)
{
    print STDERR "make-wall-marble: couldn't open SVG file\n";
    exit(1);
}
if(! -d $dir_out)
{
    mkdir($dir_out);
    if(! -d $dir_out)
    {
	print STDERR "make-wall-marble: couldn't open output directory\n";
	exit(1);
    }
}

make_pieces(16, 0);
make_pieces(20, 0);
make_pieces(24, 0);
make_pieces(32, 0);
make_pieces(40, 0);
make_pieces(64, 0);

exit(0);

sub make_pieces
{
    my $size = shift(@_);
    my $partial = shift(@_);

    $width = $size;
    $height = $size;

    $file_png = sprintf("%s/%d_%s%s.png", $dir_out, $size, "wall", $direction);
    $width_png = $width * 8;
    $height_png = $height * 8;

    $background = "";

    # Use Inkscape to convert the SVG to a PNG
    system("inkscape $background -w $width_png -h $height_png -e $file_png $file_svg");
}
