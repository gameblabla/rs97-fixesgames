#! /usr/bin/perl
#
# make-base-marble.pl
# Create the pieces for the chroma-marble graphics scheme.
#
# Requirements: ImageMagick, Inkscape, NetPBM
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
    print STDERR "Usage: make-pieces-marble.pl <pieces.svg> <directory>\n";
    exit(1);
}

$file_svg = $ARGV[0];
$dir_out = $ARGV[1];

if(! -e $file_svg)
{
    print STDERR "make-pieces-marble: couldn't open SVG file\n";
    exit(1);
}
if(! -d $dir_out)
{
    mkdir($dir_out);
    if(! -d $dir_out)
    {
	print STDERR "make-pieces-marble: couldn't open output directory\n";
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

    $rows = 8;
    $columns = 12;

    @pieces = (
            "", "", "", "", "", "", "", "",
            "arrow_shadow_left", "arrow_shadow_up", "arrow_shadow_right", "arrow_shadow_down", "bomb_shadow_left", "bomb_shadow_up", "bomb_shadow_right", "bomb_shadow_down",
            "arrow_blue_left", "arrow_blue_up", "arrow_blue_right", "arrow_blue_down", "bomb_blue_left", "bomb_blue_up", "bomb_blue_right", "bomb_blue_down",
            "arrow_green_left", "arrow_green_up", "arrow_green_right", "arrow_green_down", "bomb_green_left", "bomb_green_up", "bomb_green_right", "bomb_green_down",
            "arrow_red_left", "arrow_red_up", "arrow_red_right", "arrow_red_down", "bomb_red_left", "bomb_red_up", "bomb_red_right", "bomb_red_down",
            "", "circle", "circle_double", "star*", "dots", "dots_double", "dots_y", "dots_x",
            "", "circle_shadow", "circle_double_shadow", "star_shadow", "dots_shadow", "dots_double_shadow", "dots_y_shadow", "dots_x_shadow",
            "wall", "teleport", "switch", "door*", "map_top_left", "map_top_right", "map_bottom_left", "map_bottom_right",
            "", "teleport_shadow", "switch_shadow", "door_shadow", "map_shadow", "", "", "",
            "player_two*", "player_one*", "player_two_swapped", "player_one_swapped", "wall_inside#", "wall_y#", "wall_x#", "wall_outside#",
            "player_shadow", "", "", "", "wall_inside_shadow", "wall_y_shadow", "wall_x_shadow", "wall_outside_shadow"
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
 
        $grey = 0;
	$name =~ s/\*$//;
        if($name =~ /\#$/)
        {
            $name =~ s/\#$//;
            $grey = 1;
        }

	print "$size $name\n";

        $file_out = sprintf("%s/%d_%s%s.png", $dir_out, $size, $name, $direction);

        $xoff = $width * 2 * ($piece % $rows);
        $yoff = $height * 2 * (int($piece / $rows));
        $w = $width;
        $h = $height;

        system("convert -quality 100 -crop ".$w."x".$h."+".$xoff."+".$yoff." $file_png $file_out");
        if($grey)
        {
            grey2alpha($file_out, $file_out);
        }
    }

    unlink($file_png);
}

# A poor man's "colour to alpha" to save having to use GIMP by hand!
sub grey2alpha
{
    my $file_input = shift(@_);
    my $file_output = shift(@_);

    $file_tmp_pnm = "tmp.pnm";
    $file_tmp_pgm = "tmp.pgm";

    $grey = 111;

    open(INPUT, "pngtopnm $file_input |") || die "Couldn't open pngtopnm: $!";
    binmode INPUT;
    $line = <INPUT>;
    die "Unable to identify pnm: $file_input" if($line !~/^P6$/);
    $line = <INPUT>;
    if($line =~ /^(\d+) (\d+)$/)
    {
        $size_x = $1; $size_y = $2;
    }
    else
    {
        die "Unable to identify size from pnm: $file_input";
    }
    $line = <INPUT>;

    $slash = $/;
    undef $/;

    $data = <INPUT>;

    open(TMPCOLOUR, ">".$file_tmp_pnm) || die "Couldn't open tmp_pnm: $!";
    binmode TMPCOLOUR;
    print TMPCOLOUR "P6\n$size_x $size_y\n$line";
    open(TMPALPHA, ">".$file_tmp_pgm) || die "Couldn't open tmp_pgm: $!";
    print TMPALPHA "P5\n$size_x $size_y\n$line";
    binmode TMPALPHA;

    for($y = 0; $y < $size_y; $y ++)
    {
        for($x = 0; $x < $size_x; $x ++)
        {
    	    $r = ord(substr($data, ($y * $size_y + $x) * 3));
    	    if($r <= $grey)
	    {
	        $c = 0;
	        $a = 255 / ($grey - 0) * (111 - $r);
	    }
	    else
	    {
	        $c = 255;
	        $a = 255 / (255 - $grey) * ($r - 111);
	    }
	    printf TMPCOLOUR "%c%c%c", $c, $c, $c;
	    printf TMPALPHA "%c", $a;
        }
    }

    close(INPUT);

    close(TMPCOLOUR);
    close(TMPALPHA);

    $/ = $slash;

    system("pnmtopng -alpha $file_tmp_pgm $file_tmp_pnm > $file_output");
    unlink($file_tmp_pgm);
    unlink($file_tmp_pnm);
}
