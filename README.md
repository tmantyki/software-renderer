A software-based renderer for real-time 3D graphics.

# GNU/Linux instructions
Dependencies are satisfied in Arch Linux -based distributions by installing the following software packages. Notice that the package installation command and package names may vary between other GNU/Linux distributions.

    # pacman -S eigen gtest scons sdl2

 - **Eigen** is the industry standard C++ template library for linear algebra. It is a header-only library, thus, no linking with dynamic or static libraries is required.
 - **Google Test** is a unit test framework for C++. It is modeled after the xUnit architecture.
 - **SCons** is a software build tool which allows build configurations to be defined in a Python file. This essentially works similarly as `make`.
 - **Simple DirectMedia Layer (SDL)** is a cross-platform multimedia library suited for building 2D and 3D game engines.

Build the unit test executable and main program with:

    $ scons -Q
You may use the `-j n` option to specify the number of threads, `n`, to use for parallel compilation. This will greatly reduce the build time on multi-core systems.