# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.12

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/anonymous/Documents/Dev/rs97-source/cdogs

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/anonymous/Documents/Dev/rs97-source/cdogs/gcw0build

# Include any dependencies generated for this target.
include src/tests/CMakeFiles/color_test.dir/depend.make

# Include the progress variables for this target.
include src/tests/CMakeFiles/color_test.dir/progress.make

# Include the compile flags for this target's objects.
include src/tests/CMakeFiles/color_test.dir/flags.make

src/tests/CMakeFiles/color_test.dir/color_test.c.o: src/tests/CMakeFiles/color_test.dir/flags.make
src/tests/CMakeFiles/color_test.dir/color_test.c.o: ../src/tests/color_test.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/anonymous/Documents/Dev/rs97-source/cdogs/gcw0build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object src/tests/CMakeFiles/color_test.dir/color_test.c.o"
	cd /home/anonymous/Documents/Dev/rs97-source/cdogs/gcw0build/src/tests && /opt/rs97-toolchain/usr/bin/mipsel-buildroot-linux-musl-gcc --sysroot=/opt/rs97-toolchain/usr/mipsel-buildroot-linux-musl/sysroot $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/color_test.dir/color_test.c.o   -c /home/anonymous/Documents/Dev/rs97-source/cdogs/src/tests/color_test.c

src/tests/CMakeFiles/color_test.dir/color_test.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/color_test.dir/color_test.c.i"
	cd /home/anonymous/Documents/Dev/rs97-source/cdogs/gcw0build/src/tests && /opt/rs97-toolchain/usr/bin/mipsel-buildroot-linux-musl-gcc --sysroot=/opt/rs97-toolchain/usr/mipsel-buildroot-linux-musl/sysroot $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/anonymous/Documents/Dev/rs97-source/cdogs/src/tests/color_test.c > CMakeFiles/color_test.dir/color_test.c.i

src/tests/CMakeFiles/color_test.dir/color_test.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/color_test.dir/color_test.c.s"
	cd /home/anonymous/Documents/Dev/rs97-source/cdogs/gcw0build/src/tests && /opt/rs97-toolchain/usr/bin/mipsel-buildroot-linux-musl-gcc --sysroot=/opt/rs97-toolchain/usr/mipsel-buildroot-linux-musl/sysroot $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/anonymous/Documents/Dev/rs97-source/cdogs/src/tests/color_test.c -o CMakeFiles/color_test.dir/color_test.c.s

src/tests/CMakeFiles/color_test.dir/__/cdogs/color.c.o: src/tests/CMakeFiles/color_test.dir/flags.make
src/tests/CMakeFiles/color_test.dir/__/cdogs/color.c.o: ../src/cdogs/color.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/anonymous/Documents/Dev/rs97-source/cdogs/gcw0build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object src/tests/CMakeFiles/color_test.dir/__/cdogs/color.c.o"
	cd /home/anonymous/Documents/Dev/rs97-source/cdogs/gcw0build/src/tests && /opt/rs97-toolchain/usr/bin/mipsel-buildroot-linux-musl-gcc --sysroot=/opt/rs97-toolchain/usr/mipsel-buildroot-linux-musl/sysroot $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/color_test.dir/__/cdogs/color.c.o   -c /home/anonymous/Documents/Dev/rs97-source/cdogs/src/cdogs/color.c

src/tests/CMakeFiles/color_test.dir/__/cdogs/color.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/color_test.dir/__/cdogs/color.c.i"
	cd /home/anonymous/Documents/Dev/rs97-source/cdogs/gcw0build/src/tests && /opt/rs97-toolchain/usr/bin/mipsel-buildroot-linux-musl-gcc --sysroot=/opt/rs97-toolchain/usr/mipsel-buildroot-linux-musl/sysroot $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/anonymous/Documents/Dev/rs97-source/cdogs/src/cdogs/color.c > CMakeFiles/color_test.dir/__/cdogs/color.c.i

src/tests/CMakeFiles/color_test.dir/__/cdogs/color.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/color_test.dir/__/cdogs/color.c.s"
	cd /home/anonymous/Documents/Dev/rs97-source/cdogs/gcw0build/src/tests && /opt/rs97-toolchain/usr/bin/mipsel-buildroot-linux-musl-gcc --sysroot=/opt/rs97-toolchain/usr/mipsel-buildroot-linux-musl/sysroot $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/anonymous/Documents/Dev/rs97-source/cdogs/src/cdogs/color.c -o CMakeFiles/color_test.dir/__/cdogs/color.c.s

# Object files for target color_test
color_test_OBJECTS = \
"CMakeFiles/color_test.dir/color_test.c.o" \
"CMakeFiles/color_test.dir/__/cdogs/color.c.o"

# External object files for target color_test
color_test_EXTERNAL_OBJECTS =

src/tests/color_test: src/tests/CMakeFiles/color_test.dir/color_test.c.o
src/tests/color_test: src/tests/CMakeFiles/color_test.dir/__/cdogs/color.c.o
src/tests/color_test: src/tests/CMakeFiles/color_test.dir/build.make
src/tests/color_test: src/tests/cbehave/libcbehave.a
src/tests/color_test: src/tests/CMakeFiles/color_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/anonymous/Documents/Dev/rs97-source/cdogs/gcw0build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable color_test"
	cd /home/anonymous/Documents/Dev/rs97-source/cdogs/gcw0build/src/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/color_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/tests/CMakeFiles/color_test.dir/build: src/tests/color_test

.PHONY : src/tests/CMakeFiles/color_test.dir/build

src/tests/CMakeFiles/color_test.dir/clean:
	cd /home/anonymous/Documents/Dev/rs97-source/cdogs/gcw0build/src/tests && $(CMAKE_COMMAND) -P CMakeFiles/color_test.dir/cmake_clean.cmake
.PHONY : src/tests/CMakeFiles/color_test.dir/clean

src/tests/CMakeFiles/color_test.dir/depend:
	cd /home/anonymous/Documents/Dev/rs97-source/cdogs/gcw0build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/anonymous/Documents/Dev/rs97-source/cdogs /home/anonymous/Documents/Dev/rs97-source/cdogs/src/tests /home/anonymous/Documents/Dev/rs97-source/cdogs/gcw0build /home/anonymous/Documents/Dev/rs97-source/cdogs/gcw0build/src/tests /home/anonymous/Documents/Dev/rs97-source/cdogs/gcw0build/src/tests/CMakeFiles/color_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/tests/CMakeFiles/color_test.dir/depend

