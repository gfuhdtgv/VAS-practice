# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

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
CMAKE_SOURCE_DIR = /home/bormoley/Документы/VAS-practice/dop3

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/bormoley/Документы/VAS-practice/dop3/build

# Include any dependencies generated for this target.
include CMakeFiles/dop3.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/dop3.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/dop3.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/dop3.dir/flags.make

CMakeFiles/dop3.dir/massive.cpp.o: CMakeFiles/dop3.dir/flags.make
CMakeFiles/dop3.dir/massive.cpp.o: ../massive.cpp
CMakeFiles/dop3.dir/massive.cpp.o: CMakeFiles/dop3.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/bormoley/Документы/VAS-practice/dop3/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/dop3.dir/massive.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/dop3.dir/massive.cpp.o -MF CMakeFiles/dop3.dir/massive.cpp.o.d -o CMakeFiles/dop3.dir/massive.cpp.o -c /home/bormoley/Документы/VAS-practice/dop3/massive.cpp

CMakeFiles/dop3.dir/massive.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/dop3.dir/massive.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/bormoley/Документы/VAS-practice/dop3/massive.cpp > CMakeFiles/dop3.dir/massive.cpp.i

CMakeFiles/dop3.dir/massive.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/dop3.dir/massive.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/bormoley/Документы/VAS-practice/dop3/massive.cpp -o CMakeFiles/dop3.dir/massive.cpp.s

# Object files for target dop3
dop3_OBJECTS = \
"CMakeFiles/dop3.dir/massive.cpp.o"

# External object files for target dop3
dop3_EXTERNAL_OBJECTS =

dop3: CMakeFiles/dop3.dir/massive.cpp.o
dop3: CMakeFiles/dop3.dir/build.make
dop3: CMakeFiles/dop3.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/bormoley/Документы/VAS-practice/dop3/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable dop3"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/dop3.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/dop3.dir/build: dop3
.PHONY : CMakeFiles/dop3.dir/build

CMakeFiles/dop3.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/dop3.dir/cmake_clean.cmake
.PHONY : CMakeFiles/dop3.dir/clean

CMakeFiles/dop3.dir/depend:
	cd /home/bormoley/Документы/VAS-practice/dop3/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/bormoley/Документы/VAS-practice/dop3 /home/bormoley/Документы/VAS-practice/dop3 /home/bormoley/Документы/VAS-practice/dop3/build /home/bormoley/Документы/VAS-practice/dop3/build /home/bormoley/Документы/VAS-practice/dop3/build/CMakeFiles/dop3.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/dop3.dir/depend
