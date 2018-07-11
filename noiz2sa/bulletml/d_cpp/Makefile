CXX=g++
DMD=dmd
PERL=perl
CPPFLAGS=-g
DFLAGS=-g

all: cpp_test

cpp_test: cpp.o cpp_test.o
	$(DMD) cpp_test $(CPPFLAGS) cpp.o 

cpp.o: d_cpp_interface.h cpp.cpp
	$(CXX) $(CPPFLAGS) -c cpp.cpp

cpp_test.o: cpp_inter.d cpp_test.d
	$(DMD) $(DFLAGS) -c cpp_test.d

cpp_inter.d: cpp.cpp
	$(PERL) create_d_import.pl cpp.cpp cpp_inter.d

clean:
	rm -f *.o cpp_test cpp_inter.d
