# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.18

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
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
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /tmp/tmp.q136TOGVfu

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /tmp/tmp.q136TOGVfu/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/klient.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/klient.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/klient.dir/flags.make

CMakeFiles/klient.dir/klient/klient.cpp.o: CMakeFiles/klient.dir/flags.make
CMakeFiles/klient.dir/klient/klient.cpp.o: ../klient/klient.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/tmp/tmp.q136TOGVfu/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/klient.dir/klient/klient.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/klient.dir/klient/klient.cpp.o -c /tmp/tmp.q136TOGVfu/klient/klient.cpp

CMakeFiles/klient.dir/klient/klient.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/klient.dir/klient/klient.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /tmp/tmp.q136TOGVfu/klient/klient.cpp > CMakeFiles/klient.dir/klient/klient.cpp.i

CMakeFiles/klient.dir/klient/klient.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/klient.dir/klient/klient.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /tmp/tmp.q136TOGVfu/klient/klient.cpp -o CMakeFiles/klient.dir/klient/klient.cpp.s

# Object files for target klient
klient_OBJECTS = \
"CMakeFiles/klient.dir/klient/klient.cpp.o"

# External object files for target klient
klient_EXTERNAL_OBJECTS =

klient: CMakeFiles/klient.dir/klient/klient.cpp.o
klient: CMakeFiles/klient.dir/build.make
klient: CMakeFiles/klient.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/tmp/tmp.q136TOGVfu/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable klient"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/klient.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/klient.dir/build: klient

.PHONY : CMakeFiles/klient.dir/build

CMakeFiles/klient.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/klient.dir/cmake_clean.cmake
.PHONY : CMakeFiles/klient.dir/clean

CMakeFiles/klient.dir/depend:
	cd /tmp/tmp.q136TOGVfu/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /tmp/tmp.q136TOGVfu /tmp/tmp.q136TOGVfu /tmp/tmp.q136TOGVfu/cmake-build-debug /tmp/tmp.q136TOGVfu/cmake-build-debug /tmp/tmp.q136TOGVfu/cmake-build-debug/CMakeFiles/klient.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/klient.dir/depend
