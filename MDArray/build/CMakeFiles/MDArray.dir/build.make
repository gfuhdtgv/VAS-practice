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
CMAKE_SOURCE_DIR = /home/bormoley/Документы/VAS-practice/MDArray

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/bormoley/Документы/VAS-practice/MDArray/build

# Include any dependencies generated for this target.
include CMakeFiles/MDArray.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/MDArray.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/MDArray.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/MDArray.dir/flags.make

CMakeFiles/MDArray.dir/MDArray.c.o: CMakeFiles/MDArray.dir/flags.make
CMakeFiles/MDArray.dir/MDArray.c.o: ../MDArray.c
CMakeFiles/MDArray.dir/MDArray.c.o: CMakeFiles/MDArray.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/bormoley/Документы/VAS-practice/MDArray/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/MDArray.dir/MDArray.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/MDArray.dir/MDArray.c.o -MF CMakeFiles/MDArray.dir/MDArray.c.o.d -o CMakeFiles/MDArray.dir/MDArray.c.o -c /home/bormoley/Документы/VAS-practice/MDArray/MDArray.c

CMakeFiles/MDArray.dir/MDArray.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/MDArray.dir/MDArray.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/bormoley/Документы/VAS-practice/MDArray/MDArray.c > CMakeFiles/MDArray.dir/MDArray.c.i

CMakeFiles/MDArray.dir/MDArray.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/MDArray.dir/MDArray.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/bormoley/Документы/VAS-practice/MDArray/MDArray.c -o CMakeFiles/MDArray.dir/MDArray.c.s

# Object files for target MDArray
MDArray_OBJECTS = \
"CMakeFiles/MDArray.dir/MDArray.c.o"

# External object files for target MDArray
MDArray_EXTERNAL_OBJECTS =

MDArray: CMakeFiles/MDArray.dir/MDArray.c.o
MDArray: CMakeFiles/MDArray.dir/build.make
MDArray: CMakeFiles/MDArray.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/bormoley/Документы/VAS-practice/MDArray/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable MDArray"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/MDArray.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/MDArray.dir/build: MDArray
.PHONY : CMakeFiles/MDArray.dir/build

CMakeFiles/MDArray.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/MDArray.dir/cmake_clean.cmake
.PHONY : CMakeFiles/MDArray.dir/clean

CMakeFiles/MDArray.dir/depend:
	cd /home/bormoley/Документы/VAS-practice/MDArray/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/bormoley/Документы/VAS-practice/MDArray /home/bormoley/Документы/VAS-practice/MDArray /home/bormoley/Документы/VAS-practice/MDArray/build /home/bormoley/Документы/VAS-practice/MDArray/build /home/bormoley/Документы/VAS-practice/MDArray/build/CMakeFiles/MDArray.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/MDArray.dir/depend

