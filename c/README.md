# TStorage Client C library
TStorage client provides communication with TStorage database.

# Prerequisites

* CMake >= 3.10
* A C99-compliant C compiler

# Building the library

1. Create a directory for building - it may be out of the source directory:

```sh
mkdir build
```


2. Enter the build directory:

```sh
cd build
```

3. Run cmake, pointig it to the source package's root directory:

```sh
cmake <opts> path/to/tstorage_clients/c
```

where <opts> may be one of:

* -DBUILD_SHARED_LIBS=[0|1]
  Build a static (0, the default) or shared (1) library.
* -DBUILD_EXAMPLE=[0|1]
  Build the example program (1, the default) or skip building it (0).
  See README.example.

4. Run make to build.

```shell
make
```

# Installing the library

Typically you will need superuser privileges to do these actions.

When being in the build directory, run:

```sh
make install
```

This will install the library and its public header in the system, typicaly in

/usr/local/lib
/usr/local/include

"make install" creates an "install_manifest.txt" file in the build directory, that
contains names of all installed files. To uninstall them, enter:

```shell
xargs rm < install_manifest.txt
```
