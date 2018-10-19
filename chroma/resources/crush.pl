#! /usr/bin/perl
#
# crush.pl
# Reduce the size of the graphics files produced by the make-pieces scripts
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

$pngcrush = `which pngcrush`; chop $pngcrush;
die "Unable to locate pngcrush" if(!$pngcrush);

$directory = $ARGV[0];
die "Unable to open directory: $!" if(!opendir(DIR, $directory));

$file_tmp = "tmp.png";

while($entry = readdir(DIR))
{
    next if($entry eq "." || $entry eq "..");
    next if($entry !~ /.png$/);

    system("$pngcrush $directory/$entry $file_tmp");
    if(-e $file_tmp)
    {
        system("cp $file_tmp $directory/$entry");
        unlink($file_tmp);
    }
}

