#!/usr/bin/env perl

use strict;

my @args;

my $in = *STDIN;
if ($ARGV[0]) {
	open $in, $ARGV[0];
}

my $out = *STDOUT;
if ($ARGV[1]) {
	open $out, "> $ARGV[1]";
}

print $out "extern (C) {\n";
while (<$in>) {
	s/const //g;
	if (parseArgs("D_CPP_CLASS")) {
		my $class = shift @args;
		my $name = shift @args;
		print $out "alias int $name;\n";
	}
	if (parseArgs("D_CPP_NEW_\\d")) {
		my $class = shift @args;
		my $name = shift @args;
		my $args = join ', ', @args;
		print $out "int* $name($args);\n";
	}
	if (parseArgs("D_CPP_DELETE")) {
		my $class = shift @args;
		my $name = shift @args;
		print $out "void $name(int*);\n";
	}
	if (parseArgs("D_CPP_METHOD_\\d")) {
		my $class = shift @args;
		my $method = shift @args;
		my $name = shift @args;
		my $ret = shift @args;
		my $args = join ', ', @args;
		if ($args) {
			$args = ", $args";
		}
		print $out "$ret $name(int* $args);\n";
	}
	if (parseArgs("D_CPP_STATIC_METHOD_\\d")) {
		my $class = shift @args;
		my $method = shift @args;
		my $name = shift @args;
		my $ret = shift @args;
		my $args = join ', ', @args;
		print $out "$ret $name($args);\n";
	}
	if (parseArgs("D_CPP_VIRTUAL_METHOD_SETTER_\\d")) {
		my $class = shift @args;
		my $method = shift @args;
		my $name = shift @args;
		my $ret = shift @args;
		my $args = join ', ', @args;
		if ($args) {
			$args = ", $args";
		}
		print $out "void $name(int*, $ret (*fp) (int* $args)); \n";
	}
	if (parseArgs("D_CPP_D_DECLARE")) {
		my $declare = shift @args;
		$declare =~ s/^\"//;
		$declare =~ s/\"$//;
		print $out "$declare\n";
	}
}
print $out "}\n";

sub parseArgs {
	my $name = shift;
	if (/$name\s*\(([^\)]*)\)/) {
		@args = split /\s*,\s*/, $1;
		return 1;
	}
	else {
		return 0;
	}
}
