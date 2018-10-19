#! /usr/bin/perl
#
# make-base-zen.pl
# Create the pieces for the chroma-zen graphics scheme.
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
    print STDERR "Usage: make-pieces-zen.pl <pieces.svg> <directory>\n";
    exit(1);
}

$file_svg = $ARGV[0];
$dir_out = $ARGV[1];

if(! -e $file_svg)
{
    print STDERR "make-pieces-zen: couldn't open SVG file\n";
    exit(1);
}
if(! -d $dir_out)
{
    mkdir($dir_out);
    if(! -d $dir_out)
    {
	print STDERR "make-pieces-zen: couldn't open output directory\n";
	exit(1);
    }
}

make_pieces(16, 0);
make_pieces(20, 0);
make_pieces(24, 0);
#make_pieces(28, 1);
make_pieces(32, 0);
make_pieces(40, 0);
make_pieces(64, 0);

exit(0);

sub make_pieces
{
    my $size = shift(@_);
    my $partial = shift(@_);

    $rows = 4;
    $columns = 7;

    @pieces = (
	    "arrow_red", "arrow_green", "arrow_blue", "circle", 
	    "bomb_red","bomb_green", "bomb_blue", "circle_double", 
	    "earth", "earth_double", "earth_y", "star*", 
	    "", "teleport", "switch", "door", 
	    "map_top_left", "map_top_right", "map_bottom_left", "map_bottom_right", 
	    "player_one*", "player_two*", "player_one_swapped", "player_two_swapped", 
	    "wall_x", "wall_y", "wall_outside", "wall_inside"
	);

    $width = $size;
    $height = $size;

    $file_png = "/tmp/chroma-pieces-$$.png";
    $width_png = $width * ($rows * 2 - 1);
    $height_png = $height * ($columns * 2 - 1);

    $background = "";
    $background = "-b \"#000000\"" if($partial);

    # Use Inkscape to convert the SVG to a PNG
    system("inkscape $background -w $width_png -h $height_png -e $file_png $file_svg");

    for($piece = 0; $piece < $rows * $columns; $piece ++)
    {
        $name = $pieces[$piece];

        next if($name eq "");

	next if($partial && $name !~ /\*$/);

	$name =~ s/\*$//;

	print "$size $name\n";

        $direction = "";
        $direction = "_left" if($name =~ /^arrow/ || $name =~ /^bomb/);
    
        $file_out = sprintf("%s/%d_%s%s.png", $dir_out, $size, $name, $direction);

        $xoff = $width * 2 * ($piece % $rows);
        $yoff = $height * 2 * (int($piece / $rows));

        system("convert -quality 100 -crop ".$width."x".$height."+".$xoff."+".$yoff." $file_png $file_out");

        if($name eq "earth_y")
        {
	    $file_two = sprintf("%s/%d_%s.png", $dir_out, $width, "earth_x");
	    system("convert -quality 100 -rotate 90 $file_out $file_two");
        }
        if($name =~ /^arrow/ || $name =~ /^bomb/)
        {
	    $file_two = sprintf("%s/%d_%s%s.png", $dir_out, $width, $name, "_up");
	    system("convert -quality 100 -rotate 90 $file_out $file_two");
	    $file_two = sprintf("%s/%d_%s%s.png", $dir_out, $width, $name, "_right");
	    system("convert -quality 100 -rotate 180 $file_out $file_two");
	    $file_two = sprintf("%s/%d_%s%s.png", $dir_out, $width, $name, "_down");
	    system("convert -quality 100 -rotate 270 $file_out $file_two");
        }
    }

    unlink($file_png);
}
