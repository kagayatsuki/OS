# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.17

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

# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "G:\Extra\JetBrains\CLion 2020.2.4\bin\cmake\win\bin\cmake.exe"

# The command to remove a file.
RM = "G:\Extra\JetBrains\CLion 2020.2.4\bin\cmake\win\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = G:\repo\OS\OS_core

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = G:\repo\OS\OS_core\cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/ebug_vfs.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/ebug_vfs.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/ebug_vfs.dir/flags.make

CMakeFiles/ebug_vfs.dir/vfs/vfs_debug.cpp.obj: CMakeFiles/ebug_vfs.dir/flags.make
CMakeFiles/ebug_vfs.dir/vfs/vfs_debug.cpp.obj: CMakeFiles/ebug_vfs.dir/includes_CXX.rsp
CMakeFiles/ebug_vfs.dir/vfs/vfs_debug.cpp.obj: ../vfs/vfs_debug.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=G:\repo\OS\OS_core\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/ebug_vfs.dir/vfs/vfs_debug.cpp.obj"
	F:\LocalLibrary\MinGW\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\ebug_vfs.dir\vfs\vfs_debug.cpp.obj -c G:\repo\OS\OS_core\vfs\vfs_debug.cpp

CMakeFiles/ebug_vfs.dir/vfs/vfs_debug.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ebug_vfs.dir/vfs/vfs_debug.cpp.i"
	F:\LocalLibrary\MinGW\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E G:\repo\OS\OS_core\vfs\vfs_debug.cpp > CMakeFiles\ebug_vfs.dir\vfs\vfs_debug.cpp.i

CMakeFiles/ebug_vfs.dir/vfs/vfs_debug.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ebug_vfs.dir/vfs/vfs_debug.cpp.s"
	F:\LocalLibrary\MinGW\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S G:\repo\OS\OS_core\vfs\vfs_debug.cpp -o CMakeFiles\ebug_vfs.dir\vfs\vfs_debug.cpp.s

# Object files for target ebug_vfs
ebug_vfs_OBJECTS = \
"CMakeFiles/ebug_vfs.dir/vfs/vfs_debug.cpp.obj"

# External object files for target ebug_vfs
ebug_vfs_EXTERNAL_OBJECTS =

ebug_vfs.exe: CMakeFiles/ebug_vfs.dir/vfs/vfs_debug.cpp.obj
ebug_vfs.exe: CMakeFiles/ebug_vfs.dir/build.make
ebug_vfs.exe: CMakeFiles/ebug_vfs.dir/linklibs.rsp
ebug_vfs.exe: CMakeFiles/ebug_vfs.dir/objects1.rsp
ebug_vfs.exe: CMakeFiles/ebug_vfs.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=G:\repo\OS\OS_core\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ebug_vfs.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\ebug_vfs.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/ebug_vfs.dir/build: ebug_vfs.exe

.PHONY : CMakeFiles/ebug_vfs.dir/build

CMakeFiles/ebug_vfs.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\ebug_vfs.dir\cmake_clean.cmake
.PHONY : CMakeFiles/ebug_vfs.dir/clean

CMakeFiles/ebug_vfs.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" G:\repo\OS\OS_core G:\repo\OS\OS_core G:\repo\OS\OS_core\cmake-build-debug G:\repo\OS\OS_core\cmake-build-debug G:\repo\OS\OS_core\cmake-build-debug\CMakeFiles\ebug_vfs.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/ebug_vfs.dir/depend
