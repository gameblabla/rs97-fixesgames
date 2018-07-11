use strict;

print "#define D_CPP_CLASS(CLASS, CLASS_NAME) ;\n";
print "\n";

for (my $i = 0; $i < 10; $i++) {
	print "#define D_CPP_NEW_$i(CLASS, NEW_NAME";
	my @args;
	my @args2;
	for (my $j = 1; $j <= $i; $j++) {
		print ", ARG$j";
		push @args, "ARG$j arg$j";
		push @args2, "arg$j";
	}
	print ") \\\n";
	my $argStr = join ', ', @args;
	my $argStr2 = join ', ', @args2;

	print "CLASS* NEW_NAME($argStr) { \\\n";
	print "  return new CLASS($argStr2); \\\n";
	print "}\n";

	print "\n";
}

print "#define D_CPP_DELETE(CLASS, DELETE_NAME) \\\n";
print "void DELETE_NAME(CLASS* c) { delete c; } \n";
print "\n";

for (my $i = 0; $i < 10; $i++) {
	print "#define D_CPP_METHOD_$i(CLASS, METHOD, METHOD_NAME, RETURN";
	my $argStr;
	my @args2;
	for (my $j = 1; $j <= $i; $j++) {
		print ", ARG$j";
		$argStr .= ", ARG$j arg$j";
		push @args2, "arg$j";
	}
	print ") \\\n";
	my $argStr2 = join ', ', @args2;

	print "RETURN METHOD_NAME(CLASS* c$argStr) { \\\n";
	print "  return c->METHOD($argStr2); \\\n";
	print "}\n";

	print "\n";
}

for (my $i = 0; $i < 10; $i++) {
	print "#define D_CPP_STATIC_METHOD_$i(CLASS, METHOD, METHOD_NAME, RETURN";
	my @args;
	my @args2;
	for (my $j = 1; $j <= $i; $j++) {
		print ", ARG$j";
		push @args, "ARG$j arg$j";
		push @args2, "arg$j";
	}
	print ") \\\n";
	my $argStr = join ', ', @args;;
	my $argStr2 = join ', ', @args2;

	print "RETURN METHOD_NAME($argStr) { \\\n";
	print "  return CLASS::METHOD($argStr2); \\\n";
	print "}\n";

	print "\n";
}

print "#define D_CPP_BASE_CLASS_OPEN(BASE, BASE_NAME) \\\n";
print "struct BASE_NAME : public BASE { \n";

for (my $i = 0; $i < 10; $i++) {
	print "#define D_CPP_VIRTUAL_METHOD_$i(CLASS, METHOD, RETURN";
	my @args;
	my @args2;
	my @args3;
	for (my $j = 1; $j <= $i; $j++) {
		print ", ARG$j";
		push @args, "ARG$j arg$j";
		push @args2, ", arg$j";
		push @args3, ", ARG$j";
	}
	print ") \\\n";
	my $argStr = join ', ', @args;;
	my $argStr2 = join '', @args2;
	my $argStr3 = join '', @args3;

	print "  virtual RETURN METHOD($argStr) { \\\n";
	print "    return D_##METHOD##_fp(this $argStr2); \\\n";
	print "  } \\\n";
	print "  void D_set_##METHOD(RETURN (*fp) (CLASS* $argStr3)) { \\\n";
	print "    D_##METHOD##_fp = fp; \\\n";
	print "  } \\\n";
	print "  RETURN (*D_##METHOD##_fp) (CLASS*$argStr3); \n";

	print "\n";
}

for (my $i = 0; $i < 10; $i++) {
	print "#define D_CPP_VIRTUAL_METHOD_SETTER_$i(CLASS, METHOD, SETTER_NAME, RETURN";
	my @args;
	my @args2;
	my @args3;
	for (my $j = 1; $j <= $i; $j++) {
		print ", ARG$j";
		push @args, "ARG$j arg$j";
		push @args2, ", arg$j";
		push @args3, ", ARG$j";
	}
	print ") \\\n";
	my $argStr = join ', ', @args;;
	my $argStr2 = join '', @args2;
	my $argStr3 = join '', @args3;

	print "void SETTER_NAME(CLASS* c, RETURN (*fp) (CLASS*$argStr3)) { \\\n";
	print "  c->D_set_##METHOD(fp); \\\n";
	print "}\n";

	print "\n";
}

print "#define D_CPP_BASE_CLASS_CLOSE() \\\n";
print "};\n";
print "\n";

print "#define D_CPP_D_DECLARE(arg) ; \n";
print "\n";
