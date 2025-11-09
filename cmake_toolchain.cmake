# MinGW Toolchain for Omega DAW
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_C_COMPILER "C:/msys64/mingw64/bin/gcc.exe")
set(CMAKE_CXX_COMPILER "C:/msys64/mingw64/bin/g++.exe")
set(CMAKE_RC_COMPILER "C:/msys64/mingw64/bin/windres.exe")

# Explicitly set the make program
find_program(CMAKE_MAKE_PROGRAM NAMES mingw32-make PATHS "C:/msys64/mingw64/bin" NO_DEFAULT_PATH)
if(NOT CMAKE_MAKE_PROGRAM)
    set(CMAKE_MAKE_PROGRAM "C:/msys64/mingw64/bin/mingw32-make.exe" CACHE FILEPATH "Make program" FORCE)
endif()

# Set library search paths
set(CMAKE_PREFIX_PATH "C:/msys64/mingw64/bin/..")
set(CMAKE_FIND_ROOT_PATH "C:/msys64/mingw64/bin/..")

# Adjust search behavior
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Set proper library suffixes
set(CMAKE_FIND_LIBRARY_SUFFIXES .dll.a .a .lib)
