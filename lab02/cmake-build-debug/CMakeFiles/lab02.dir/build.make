# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.14

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

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files (x86)\JetBrains\CLion 2019.1.4\bin\cmake\win\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files (x86)\JetBrains\CLion 2019.1.4\bin\cmake\win\bin\cmake.exe" -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = Z:\CSCI441\lab02

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = Z:\CSCI441\lab02\cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/lab02.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/lab02.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/lab02.dir/flags.make

CMakeFiles/lab02.dir/main.cpp.obj: CMakeFiles/lab02.dir/flags.make
CMakeFiles/lab02.dir/main.cpp.obj: CMakeFiles/lab02.dir/includes_CXX.rsp
CMakeFiles/lab02.dir/main.cpp.obj: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=Z:\CSCI441\lab02\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/lab02.dir/main.cpp.obj"
	C:\mingw-w64\mingw64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\lab02.dir\main.cpp.obj -c Z:\CSCI441\lab02\main.cpp

CMakeFiles/lab02.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/lab02.dir/main.cpp.i"
	C:\mingw-w64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E Z:\CSCI441\lab02\main.cpp > CMakeFiles\lab02.dir\main.cpp.i

CMakeFiles/lab02.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/lab02.dir/main.cpp.s"
	C:\mingw-w64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S Z:\CSCI441\lab02\main.cpp -o CMakeFiles\lab02.dir\main.cpp.s

# Object files for target lab02
lab02_OBJECTS = \
"CMakeFiles/lab02.dir/main.cpp.obj"

# External object files for target lab02
lab02_EXTERNAL_OBJECTS =

lab02.exe: CMakeFiles/lab02.dir/main.cpp.obj
lab02.exe: CMakeFiles/lab02.dir/build.make
lab02.exe: CMakeFiles/lab02.dir/linklibs.rsp
lab02.exe: CMakeFiles/lab02.dir/objects1.rsp
lab02.exe: CMakeFiles/lab02.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=Z:\CSCI441\lab02\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable lab02.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\lab02.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/lab02.dir/build: lab02.exe

.PHONY : CMakeFiles/lab02.dir/build

CMakeFiles/lab02.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\lab02.dir\cmake_clean.cmake
.PHONY : CMakeFiles/lab02.dir/clean

CMakeFiles/lab02.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" Z:\CSCI441\lab02 Z:\CSCI441\lab02 Z:\CSCI441\lab02\cmake-build-debug Z:\CSCI441\lab02\cmake-build-debug Z:\CSCI441\lab02\cmake-build-debug\CMakeFiles\lab02.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/lab02.dir/depend

