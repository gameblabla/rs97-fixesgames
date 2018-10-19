#! /usr/bin/perl
#
# make-base-neon.pl
# Create the pieces for the chroma-neon graphics scheme.
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

# The colours used are as follows:
#
#    black   000000  000000  ffffff
#    red     800000  ff0000  ffc0c0   
#    green   008000  00ff00  c0ffc0
#    yellow  dd0000  e8d912  ffffff
#    blue    000080  0000ff  c0c0ff
#    magenta 400080  8000ff  eeddff
#    cyan    0000dd  12d9e8  ffffff
#
# Use blur-full for most pieces, blur-half for earth and earth_double, and
# blur-quarter for earth_directional, maps and teleport.

if($#ARGV != 0)
{
    print STDERR "Usage: make-pieces-neon.pl <directory>\n";
    exit(1);
}

$file_svg_base = "pieces-neon-base.svg";
$file_svg_overlay = "pieces-neon-overlay.svg";
$file_svg_overlay_double = "pieces-neon-overlay-double.svg";
$dir_out = $ARGV[0];

if(! -e $file_svg_base || ! -e $file_svg_overlay || ! -e $file_svg_overlay_double)
{
    print STDERR "make-pieces-neon: couldn't open SVG file\n";
    exit(1);
}
if(! -d $dir_out)
{
    mkdir($dir_out);
    if(! -d $dir_out)
    {
	print STDERR "make-pieces-neon: couldn't open output directory\n";
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

    # Pieces to make: * = create small piece, # = don't make shadow
    @pieces = (
	    "arrow_red", "arrow_green", "arrow_blue", "circle",
	    "bomb_red","bomb_green", "bomb_blue", "circle_double", 
	    "earth", "earth_double", "earth_y", "star*", 
	    "wall", "teleport", "switch", "door", 
	    "map_top_left", "map_top_right", "map_bottom_left", "map_bottom_right", 
	    "player_one*", "player_two*", "player_one_swapped", "player_two_swapped", 
	    "wall_x#", "wall_y#", "wall_outside#", "wall_inside#"
	);

    $width = $size;
    $height = $size;

    $width_png = $width * ($rows * 2 + 1);
    $height_png = $height * ($columns * 2 + 1);

    $file_png_base = "/tmp/chroma-pieces-base-$$.png";
    system("inkscape -w $width_png -h $height_png -e $file_png_base $file_svg_base");

    $file_png_overlay = "/tmp/chroma-pieces-overlay-$$.png";
    system("inkscape -w $width_png -h $height_png -e $file_png_overlay $file_svg_overlay");

    $file_png_overlay_double = "/tmp/chroma-pieces-double-$$.png";
    system("inkscape -w $width_png -h $height_png -e $file_png_overlay_double $file_svg_overlay_double");

    $file_png_partial = "/tmp/chroma-pieces-partial-$$.png";
    system("convert -compose overlay -composite $file_png_base $file_png_overlay $file_png_partial");

    $file_png_result = "/tmp/chroma-pieces-overlayed-$$.png";
    system("convert -compose overlay -composite $file_png_partial $file_png_overlay_double $file_png_result");

    $file_png_crop = "/tmp/chroma-pieces-crop-$$.png";
    $xoff = $width * 0.5;
    $yoff = $height * 12.5;
    system("convert -quality 100 -crop ".($width * 2)."x".($height * 2)."+".$xoff."+".$yoff." $file_png_overlay $file_png_crop");

    $file_png_back = "/tmp/chroma-pieces-back-$$.png";
    $xoff = $width * 1;
    $yoff = $height * 13;
    system("convert -quality 100 -crop ".$width."x".$height."+".$xoff."+".$yoff." $file_png_overlay $file_png_back");

    for($piece = 0; $piece < $rows * $columns; $piece ++)
    {
        $name = $pieces[$piece];

        next if($name eq "");

	next if($partial && $name !~ /\*$/);

        if($name =~ /\#$/)
        {
            $shadow = 0;
        }
        else
        {
            $shadow = 1;
        }

        if($partial)
        {
            $name = "small_$name";
        }

	$name =~ s/\*$//;
	$name =~ s/\#$//;

	print "$size $name\n";

        $direction = "";
        $direction = "_left" if($name =~ /^arrow/ || $name =~ /^bomb/);
    
        # Make shadow
        if(!$partial && $shadow)
        {
        $file_out = sprintf("%s/%d_%s%s.png", $dir_out, $size, $name, $direction);
        $xoff = $width * (0.5 + 2 * ($piece % $rows));
        $yoff = $height * (0.5 + 2 * (int($piece / $rows)));

        system("convert -quality 100 -crop ".($width * 2)."x".($height * 2)."+".$xoff."+".$yoff." $file_png_result $file_out");

        $file_shadow = $file_out; $file_shadow =~ s/\.png/_shadow\.png/;
        system("composite -compose Dst_Out -gravity South $file_png_crop $file_out -matte $file_shadow");
        }

        # Make piece
        $file_out = sprintf("%s/%d_%s%s.png", $dir_out, $size, $name, $direction);
        $xoff = $width * (1 + 2 * ($piece % $rows));
        $yoff = $height * (1 + 2 * (int($piece / $rows)));

        system("convert -quality 100 -crop ".$width."x".$height."+".$xoff."+".$yoff." $file_png_result $file_out");

        # Set background if needed
        if($partial)
        {
            system("composite $file_out $file_png_back $file_out");
        }


        # Make shadow rotations
        if($name eq "earth_y")
        {
	    $file_two = sprintf("%s/%d_%s_shadow.png", $dir_out, $width, "earth_x");
	    system("convert -quality 100 -rotate 90 $file_shadow $file_two");
        }
        if($name =~ /^arrow/ || $name =~ /^bomb/)
        {
	    $file_two = sprintf("%s/%d_%s%s_shadow.png", $dir_out, $width, $name, "_up");
	    system("convert -quality 100 -rotate 90 $file_shadow $file_two");
	    $file_two = sprintf("%s/%d_%s%s_shadow.png", $dir_out, $width, $name, "_right");
	    system("convert -quality 100 -rotate 180 $file_shadow $file_two");
	    $file_two = sprintf("%s/%d_%s%s_shadow.png", $dir_out, $width, $name, "_down");
	    system("convert -quality 100 -rotate 270 $file_shadow $file_two");
        }

        # Make piece rotations
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

    unlink($file_png_base);
    unlink($file_png_overlay);
    unlink($file_png_overlay_double);
    unlink($file_png_partial);
    unlink($file_png_result);
    unlink($file_png_crop);
    unlink($file_png_back);
}

