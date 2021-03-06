.. Common references to other documents

.. _Package Dependencies and Enable/Disable Logic: ../developers_guide/TribitsDevelopersGuide.html#package-dependencies-and-enable-disable-logic

.. _TriBITS Dependency Handling Behaviors: ../developers_guide/TribitsDevelopersGuide.html#tribits-dependency-handling-behaviors


Getting set up to use CMake
===========================

Before one can configure <Project> to be built, one must first obtain a
version of CMake on the system newer than <MinCMakeVer> This guide assumes
that once CMake is installed that it will be in the default path with the name
``cmake``.


Installing a binary release of CMake [casual users]
---------------------------------------------------

Download and install the binary (version <MinCMakeVer> or greater is
recommended) from:

  http://www.cmake.org/cmake/resources/software.html


Installing CMake from source [developers and experienced users]
---------------------------------------------------------------

If you have access to the <Project> git repositories, then install CMake with::

  $ $TRIBITS_BASE_DIR/python/install-cmake.py \
     --install-dir=<INSTALL_BASE_DIR> \
     --do-all

This will result in cmake and related CMake tools being installed in
<INSTALL_BASE_DIR>/bin.

Getting help for installing CMake with this script::

  $ $TRIBITS_BASE_DIR/python/install-cmake.py --help

NOTE: you will want to read the help message about how to use sudo to
install in a privileged location (like the default /usr/local/bin).


Getting CMake Help
==================


Finding CMake help at the website
---------------------------------

  http://www.cmake.org


Building CMake help locally
---------------------------

To get help on CMake input options, run::

  $ cmake --help

To get help on a single CMake function, run::

  $ cmake --help-command <command>

To generate the entire documentation at once, run::

  $ cmake --help-full cmake.help.html

(Open your web browser to the file cmake.help.html)


Configuring (Makefile Generator)
================================

While CMake supports a number of different build generators (e.g. Eclipes,
XCode, MS Visual Studio, etc.) the primary generator most people use on
Unix/Linix system is make and CMake generates exceptional Makefiles.  The
materila in this section, while not exclusing to the makefile generator this
should be assumed as the default.


Setting up a build directory
----------------------------

In order to configure, one must set up a build directory.  <Project> does
*not* support in-source builds so the build tree must be seprate from the
source tree.  The build tree can be created under the source tree such as
with::

  $ $SOURCE_DIR
  $ mkdir <SOME_BUILD_DIR>
  $ cd <SOME_BUILD_DIR>

but it is generally recommended to create a build directory parallel from the
source tree.

NOTE: If you mistakenly try to configure for an in-source build (e.g. with
'cmake .') you will get an error message and instructions on how to resolve
the problem by deleting the generated CMakeCache.txt file (and other generated
files) and then follow directions on how to create a different build directory
as shown above.


Basic configuration
-------------------

a) Create a 'do-configure' script such as [Recommended]::

    EXTRA_ARGS=$@
    
    cmake \
      -D CMAKE_BUILD_TYPE=DEBUG \
      -D <Project>_ENABLE_TESTS=ON \
      $EXTRA_ARGS \
      ${SOURCE_BASE}

  and then run it with::

    ./do-configure [OTHER OPTIONS] -D<Project>_ENABLE_<TRIBITS_PACKAGE>=ON

  where ``<TRIBITS_PACKAGE>`` is a valid SE Package name (see above), etc. and
  ``SOURCE_BASE`` is set to the <Project> source base directory (or your can
  just give it explicitly in the script).

  See `<Project>/sampleScripts/*cmake` for examples of real `do-configure`
  scripts for different platforms..

  NOTE: If one has already configured once and one needs to configure from
  scratch (needs to wipe clean defaults for cache variables, updates
  compilers, other types of changes) then one will want to delete the local
  CASL and other CMake-generated files before configuring again (see
  `Reconfiguring completely from scratch`_).

.. _<Project>_CONFIGURE_OPTIONS_FILE:

b) Create a CMake file fragment and point to it [Recommended].

  Create a do-configure script like::

    EXTRA_ARGS=$@
    
    cmake \
      -D <Project>_CONFIGURE_OPTIONS_FILE:FILEPATH=MyConfigureOptions.cmake \
      -D <Project>_ENABLE_TESTS=ON \
      $EXTRA_ARGS \
      ${SOURCE_BASE}
     
  where MyConfigureOptions.cmake might look like::

    SET(CMAKE_BUILD_TYPE DEBUG CACHE STRING "")
    SET(<Project>_ENABLE_CHECKED_STL ON CACHE BOOL "")
    SET(BUILD_SHARED_LIBS ON CACHE BOOL "")
    ...

  Using a configuration fragment file allows for better reuse of configure
  options across different configure scripts and better version control of
  configure options.  Also, when this file changes, CMake will automatically
  trigger a reconfgure during a make (because it knows about the file and will
  check its time stamp).

  NOTE: You can actually pass in a list of configuration fragment files
  which will be read in the order they are given.

  NOTE: You can use the ``FORCE`` option in the ``SET()`` shown above and that
  will override any value of the options that might already be set.  However,
  that will not allow the user to override the options on the CMake
  comamndline using ``-D<VAR>=<value>`` so it is generally desired to use
  ``FORCE``.

c) Using ccmake to configure

  ::

    $ ccmake $SOURCE_BASE

d) Using the QT CMake configuration GUI:

  On systems where the QT CMake GUI is installed (e.g. Windows) the CMake GUI
  can be a nice way to configure <Project> if you are a user.  To make your
  configuration easily repeatable, you might want to create a fragment file
  and just load it by setting `<Project>_CONFIGURE_OPTIONS_FILE`_ (see above)
  in the GUI.

Selecting the list of packages to enable
----------------------------------------

The <Project> project is broken up into a set of packages that can be enabled
(or disabled).  For details and generic examples, see `Package Dependencies and
Enable/Disable Logic`_ and `TriBITS Dependency Handling Behaviors`_.

See the following use cases:

* `Determine the list of packages that can be enabled`_
* `Print package dependencies`_
* `Enable a set of packages`_
* `Enable to test all effects of changing a given package(s)`_
* `Enable all packages with tests and examples`_
* `Disable a package and all its dependencies`_
* `Remove all package enables in the cache`_

Determine the list of packages that can be enabled
++++++++++++++++++++++++++++++++++++++++++++++++++

In order to see the list of available <Project> SE Packages to enable, just
run a basic CMake configure, enabling nothing, and then grep the output to see
what packages are available to enable.  The full set of defined packages is
contained the lines starting with ``'Final set of enabled SE packages'`` and
``'Final set of non-enabled SE packages'``.  If no SE packages are enabled by
default (which is base behavior), the full list of packages will be listed on
the line ``'Final set of non-enabled SE packages'``.  Therefore, to see the
full list of defined packages, run::

  ./do-configure 2>&1 | grep "Final set of .*enabled SE packages"

Any of the packages shown on those lines can potentially be enabled using ``-D
<Project>_ENABLE_<TRIBITS_PACKAGE>=ON`` (unless they are set to disabled
for some reason, see the CMake output for package disable warnings).

Another way to see the full list of SE packages that can be enabled is to
configure with `<Project>_DUMP_PACKAGE_DEPENDENCIES`_ = ``ON`` and then grep
for ``<Project>_SE_PACKAGES`` using, for example::

  ./do-configure 2>&1 | grep "<Project>_SE_PACKAGES: "

.. _<Project>_DUMP_PACKAGE_DEPENDENCIES:

Print package dependencies
++++++++++++++++++++++++++

The set of package dependencies in a project will be printed in the ``cmake``
STDOUT by setting::

  -D <Project>_DUMP_PACKAGE_DEPENDENCIES=ON

This will print the basic backward dependencies for each SE package.  To also
see the direct forward dependencies for each SE package, also include::

  -D <Project>_DUMP_FORWARD_PACKAGE_DEPENDENCIES=ON

Both of these variables are automatically enabled when
`<Project>_VERBOSE_CONFIGURE`_ = ``ON``.

Enable a set of packages
++++++++++++++++++++++++

To enable an SE package ``<TRIBITS_PACKAGE>`` (and optionally also its tests
and examples), configure with::

  -D <Project>_ENABLE_<TRIBITS_PACKAGE>=ON \
  -D <Project>_ENABLE_ALL_OPTIONAL_PACKAGES=ON \
  -D <Project>_ENABLE_TESTS=ON \

This set of arguments allows a user to turn on ``<TRIBITS_PACKAGE>`` as well
as all packages that ``<TRIBITS_PACKAGE>`` can use.  All of the package's
optional "can use" upstream dependent packages are enabled with
``-D<Project>_ENABLE_ALL_OPTIONAL_PACKAGES=ON``.  However,
``-D<Project>_ENABLE_TESTS=ON`` will only enable tests and examples for
``<TRIBITS_PACKAGE>`` (or any other packages specifically enabled).

If a TriBITS package ``<TRIBITS_PACKAGE>`` has subpackages (e.g. ``<A>``,
``<B>``, etc.), then enabling the package is equivalent to setting::

  -D <Project>_ENABLE_<TRIBITS_PACKAGE><A>=ON \
  -D <Project>_ENABLE_<TRIBITS_PACKAGE><B>=ON \
   ...

However, a TriBITS subpackage will only be enabled if it is not already
disabled either explicitly or implicitly.

NOTE: The CMake cache variable type for all ``XXX_ENABLE_YYY`` variables is
actually ``STRING`` and not ``BOOL``.  That is because these enable variables
take on the string enum values of ``"ON"``, ``"OFF"``, end empty ``""``.  An
empty enable means that the TriBITS dependency system is allowed to decide if
an enable should be turned on or off based on various logic.  The CMake GUI
will enforce the values of ``"ON"``, ``"OFF"``, and empty ``""`` but it will
not enforce this if you set the value on the command line or in a SET()
statement in an input ```*.cmake`` options files.  However, setting
``-DXXX_ENABLE_YYY=TRUE`` and ``-DXXX_ENABLE_YYY=FALSE`` is allowed and will
be interpreted correctly..

Enable to test all effects of changing a given package(s)
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++

To enable an SE package ``<TRIBITS_PACKAGE>`` to test it and all of its
down-stream packages, configure with::

  -D <Project>_ENABLE_<TRIBITS_PACKAGE>=ON \
  -D <Project>_ENABLE_ALL_FORWARD_DEP_PACKAGES=ON \
  -D <Project>_ENABLE_TESTS=ON \

The above set of arguments will result in package ``<TRIBITS_PACKAGE>`` and
all packages that depend on ``<TRIBITS_PACKAGE>`` to be enabled and have all
of their tests turned on.  Tests will not be enabled in packages that do not
depend on ``<TRIBITS_PACKAGE>`` in this case.  This speeds up and robustifies
pre-push testing.

Enable all packages with tests and examples
+++++++++++++++++++++++++++++++++++++++++++

To enable all SE packages (and optionally also their tests and examples), add
the configure options::

  -D <Project>_ENABLE_ALL_PACKAGES=ON \
  -D <Project>_ENABLE_TESTS=ON \

Specific packages can be disabled with
``<Project>_ENABLE_<TRIBITS_PACKAGE>=OFF``.  This will also disable all
packages that depend on ``<TRIBITS_PACKAGE>``.

All examples are also enabled by default when setting
``<Project>_ENABLE_TESTS=ON``.

By default, setting ``<Project>_ENABLE_ALL_PACKAGES=ON`` only enables primary
tested (PT) code.  To have this also enable all secondary tested (ST) code,
one must also set ``<Project>_ENABLE_SECONDARY_TESTED_CODE=ON``.

NOTE: If the project is a "meta-project", then
``<Project>_ENABLE_ALL_PACKAGES=ON`` may not enable *all* the SE packages
but only the project's primary meta-project packages.  See `Package
Dependencies and Enable/Disable Logic`_ and `TriBITS Dependency Handling
Behaviors`_ for details.

Disable a package and all its dependencies
++++++++++++++++++++++++++++++++++++++++++

To disable an SE package and all of the packages that depend on it, add the
configure options::

  -D <Project>_ENABLE_<TRIBITS_PACKAGE>=OFF

For example::

  -D <Project>_ENABLE_<PACKAGE_A>=ON \
  -D <Project>_ENABLE_ALL_OPTIONAL_PACKAGES=ON \
  -D <Project>_ENABLE_<PACKAGE_B>=ON \

will enable ``<PACKAGE_A>`` and all of the packages that it depends on except
for ``<PACKAGE_B>`` and all of its forward dependencies.

If a TriBITS package ``<TRIBITS_PACKAGE>`` has subpackages (e.g. ``<A>``,
``<B>``, etc.), then disabling the package is equivalent to setting::

  -D <Project>_ENABLE_<TRIBITS_PACKAGE><A>=OFF \
  -D <Project>_ENABLE_<TRIBITS_PACKAGE><B>=OFF \
  ...

The disable of the subpackage is this case will override any enables.

If a disabled package is a required dependency of some explicitly enabled
downstream package, then the configure will error out if
``<Project>_DISABLE_ENABLED_FORWARD_DEP_PACKAGES=OFF``.  Otherwise, a WARNING
will be printed and the downstream package will be disabled and configuration
will continue.


Remove all package enables in the cache
+++++++++++++++++++++++++++++++++++++++

To wipe the set of pakage enables in the CMakeCache.txt file so they can be
reset again from scratch, configure with::

  $ ./-do-confiugre -D <Project>_UNENABLE_ENABLED_PACKAGES=TRUE

This option will set to empty '' all package enables, leaving all other cache
variables as they are.  You can then reconfigure with a new set of package
enables for a different set of packages.  This allows you to avoid more
expensive configure time checks and to preserve other cache variables that you
have set and don't want to loose.  For example, one would want to do this to
avoid compiler and TPL checks.

Selecting compiler and linker options
-------------------------------------

The compilers for C, C++, and Fortran will be found by default by CMake if
they are not otherwise specified as described below (see standard CMake
documentation for how default compilers are found).  The most direct way to
set the compilers are to set the CMake cache variables::

  -D CMAKE_<LANG>_COMPILER=<path-to-compiler>

The path to the compiler can be just a name of the compiler
(e.g. ``-DCMAKE_C_COMPILER=gcc``) or can be an absolute path
(e.g. ``-DCMAKE_C_COMPILER=/usr/local/bin/cc``).  The safest and more direct
approach to determine the compilers is to set the absolute paths using, for
example, the cache variables::

  -D CMAKE_C_COMPILER=/opt/my_install/bin/gcc \
  -D CMAKE_CXX_COMPILER=/opt/my_install/bin/g++ \
  -D CMAKE_Fortran_COMPILER=/opt/my_install/bin/gfortran

or if ``TPL_ENABLE_MPI=ON`` (see `Configuring with MPI support`_) something
like::

  -D CMAKE_C_COMPILER=/opt/my_install/bin/mpicc \
  -D CMAKE_CXX_COMPILER=/opt/my_install/bin/mpicxx \
  -D CMAKE_Fortran_COMPILER=/opt/my_install/bin/mpif90

If these the CMake cache variables are not set, then CMake will use the
compilers specified in the environment variables ``CC``, ``CXX``, and ``FC``
for C, C++ and Fortran, respectively.  If one needs to drill down through
different layers of scripts, then it can be useful to set the compilers using
these environment variables.  But in general is it recommended to be explicit
and use the above CMake cache variables to set the absolute path to the
compilers to remove all ambiguity.

If absolute paths to the compilers are not specified using the CMake cache
variables or the environment variables as described above, then in MPI mode
(i.e. ``TPL_ENABLE_MPI=ON``) TriBITS performs its own search for the MPI
compiler wrappers that will find the correct compilers for most MPI
distributions (see `Configuring with MPI support`_).  However, if in serial
mode (i.e. ``TPL_ENABLE_MPI=OFF``), then CMake will do its own default
compiler search.  The algorithm by which raw CMake finds these compilers is
not precisely documented (and seems to change based on the platform).
However, on Linux systems, the observed algorithm appears to be:

1. Search for the C compiler first by looking in ``PATH`` (or the equivalent
   on Windows), starting with a compiler with the name ``cc`` and then moving
   on to other names like ``gcc``, etc.  This first compiler found is set to
   ``CMAKE_C_COMPILER``.

2. Search for the C++ compiler with names like ``c++``, ``g++``, etc., but
   restrict the search to the same directory specified by base path to the C
   compiler given in the variable ``CMAKE_C_COMPILER``.  The first compiler
   that is found is set to ``CMAKE_CXX_COMPILER``.

3. Search for the Fortran compiler with names like ``f90``, ``gfortran``,
   etc., but restrict the search to the same directory specified by base path
   to the C compiler given in the variable ``CMAKE_C_COMPILER``.  The first
   compiler that is found is set to ``CMAKE_CXX_COMPILER``.

**WARNING:** While this build-in CMake compiler search algorithm may seems
reasonable, it fails to find the correct compilers in many cases for a non-MPI
serial build.  For example, if a newer version of GCC is installed and is put
first in ``PATH``, then CMake will fail to find the updated ``gcc`` compiler
and will instead find the default system ``cc`` compiler (usually under
``/usr/bin/cc`` on Linux may systems) and will then only look for the C++ and
Fortran compilers under that directory.  This will fail to find the correct
updated compilers because GCC does not install a C compiler named ``cc``!
Therefore, if you want to use the default CMake compiler search to find the
updated GCC compilers, you can set the CMake cache variable::

  -D CMAKE_C_COMPILER=gcc

or can set the environment variable ``CC=gcc``.  Either one of these will
result in CMake finding the updated GCC compilers found first in ``PATH``.

Once one has specified the compilers, one can also set the compiler flags, but
the way that CMake does this is a little surprising to many people.  But the
<Project> TriBITS CMake build system offers the ability to tweak the built-in
CMake approach for setting compiler flags.  First some background is in order.
When CMake creates the object file build command for a given source file, it
passes in flags to the compiler in the order::

  ${CMAKE_<LANG>_FLAGS}  ${CMAKE_<LANG>_FLAGS_<CMAKE_BUILD_TYPE>}

where ``<LANG>`` = ``C``, ``CXX``, or ``Fortran`` and ``<CMAKE_BUILD_TYPE>`` =
``DEBUG`` or ``RELEASE``.  Note that the options in
``CMAKE_<LANG>_FLAGS_<CMAKE_BUILD_TYPE>`` come after and override those in
``CMAKE_<LANG>_FLAGS``!  The flags in ``CMAKE_<LANG>_FLAGS`` apply to all
build types.  Optimization, debug, and other build-type-specific flags are set
in ``CMAKE_<LANG>_FLAGS_<CMAKE_BUILD_TYPE>``.  CMake automatically provides a
default set of debug and release optimization flags for
``CMAKE_<LANG>_FLAGS_<CMAKE_BUILD_TYPE>`` (e.g. ``CMAKE_CXX_FLAGS_DEBUG`` is
typically ``"-g -O0"`` while ``CMAKE_CXX_FLAGS_RELEASE`` is typically
``"-O3"``).  This means that if you try to set the optimization level with
``-DCMAKE_CXX_FLAGS="-04"``, then this level gets overridden by the flags
specified in ``CMAKE_<LANG>_FLAGS_BUILD`` or ``CMAKE_<LANG>_FLAGS_RELEASE``.

Note that TriBITS will set defaults for ``CMAKE_<LANG>_FLAGS`` and
``CMAKE_<LANG>_FLAGS_<CMAKE_BUILD_TYPE>``, which may be different that what
raw CMake would set.  TriBITS provides a means for project and package
developers and users to set and override these compiler flag variables
globally and on a package-by-package basis.  Below, the facilities for
manipulating compiler flags is described.

Also, to see that the full set of compiler flags one has to actually build a
target with, for example ``make VERBOSE=1`` (see `Building with verbose output
without reconfiguring`_).  One can not just look at the cache variables for
``CMAKE_<LANG>_FLAGS`` and ``CMAKE_<LANG>_FLAGS_<CMAKE_BUILD_TYPE>`` in the
file ``CMakeCache.txt``.  These get overwritten and redefined by TriBITS in
development as described below (see `Overriding CMAKE_BUILD_TYPE debug/release
compiler options`_).

The <Project> TriBITS CMake build system will set up default compile flags for
GCC ('GNU') in development mode
(i.e. ``<Project>_ENABLE_DEVELOPMENT_MODE=ON``) on order to help produce
portable code.  These flags set up strong warning options and enforce language
standards.  In release mode (i.e. ``<Project>_ENABLE_DEVELOPMENT_MODE=ON``),
these flags are not set.  These flags get set internally into the variables
``CMAKE_<LANG>_FLAGS`` (when processing packages, not at the global cache
variable level) but the user can append flags that override these as described
below.

.. _CMAKE_BUILD_TYPE:

Configuring to build with default debug or release compiler flags
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

To build a debug version, pass into 'cmake'::

  -D CMAKE_BUILD_TYPE=DEBUG

This will result in debug flags getting passed to the compiler according to
what is set in ``CMAKE_<LANG>_FLAGS_DEBUG``.

To build a release (optimized) version, pass into 'cmake'::

  -D CMAKE_BUILD_TYPE=RELEASE

This will result in optimized flags getting passed to the compiler according
to what is in ``CMAKE_<LANG>_FLAGS_RELEASE``.

The default build type is typically ``CMAKE_BUILD_TYPE=RELEASE`` unless ``-D
USE_XSDK_DEFAULTS=TRUE`` is set in which case the default build type is
``CMAKE_BUILD_TYPE=DEBUG`` as per the xSDK configure standard.

Adding arbitrary compiler flags but keeping default build-type flags
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

To append arbitrary compiler flags to ``CMAKE_<LANG>_FLAGS`` (which may be
set internally by TriBITS) that apply to all build types, configure with::

  -D CMAKE_<LANG>_FLAGS="<EXTRA_COMPILER_OPTIONS>"

where ``<EXTRA_COMPILER_OPTIONS>`` are your extra compiler options like
``"-DSOME_MACRO_TO_DEFINE -funroll-loops"``.  These options will get
appended to (i.e. come after) other internally defined compiler option and
therefore override them.  The options are then pass to the compiler in the
order::

  <DEFAULT_TRIBITS_LANG_FLAGS> <EXTRA_COMPILER_OPTIONS> \
    ${CMAKE_<LANG>_FLAGS_<CMAKE_BUILD_TYPE>}

This that setting ``CMAKE_<LANG>_FLAGS`` can override the default flags that
TriBITS will set for ``CMAKE_<LANG>_FLAGS`` but will **not** override flags
specified in ``CMAKE_<LANG>_FLAGS_<CMAKE_BUILD_TYPE>``.

Instead of directly setting the CMake cache variables ``CMAKE_<LANG>_FLAGS``
one can instead set environment variables ``CFLAGS``, ``CXXFLAGS`` and
``FFLAGS`` for ``CMAKE_C_FLAGS``, ``CMAKE_CXX_FLAGS`` and
``CMAKE_Fortran_FLAGS``, respectively.

In addition, if ``-DUSE_XSDK_DEFAULTS=TRUE`` is set, then one can also pass
in Fortran flags using the environment variable ``FCFLAGS`` (raw CMake does
not recognize ``FCFLAGS``).  But if ``FFLAGS`` and ``FCFLAGS`` are both set,
then they must be the same or a configure error will occur.

Options can also be targeted to a specific TriBITS package using::

  -D <TRIBITS_PACKAGE>_<LANG>_FLAGS="<EXTRA_COMPILER_OPTIONS>"

The package-specific options get appended to those already in
``CMAKE_<LANG>_FLAGS`` and therefore override (but not replace) those set
globally in ``CMAKE_<LANG>_FLAGS`` (either internally or by the user in the
cache).

NOTES:

1) Setting ``CMAKE_<LANG>_FLAGS`` will override but will not replace any
other internally set flags in ``CMAKE_<LANG>_FLAGS`` defined by the
<Project> CMake system because these flags will come after those set
internally.  To get rid of these project/TriBITS default flags, see below.

2) Given that CMake passes in flags in
``CMAKE_<LANG>_FLAGS_<CMAKE_BUILD_TYPE>`` after those in
``CMAKE_<LANG>_FLAGS`` means that users setting the ``CMAKE_<LANG>_FLAGS``
and ``<TRIBITS_PACKAGE>_<LANG>_FLAGS`` will **not** override the flags in
``CMAKE_<LANG>_FLAGS_<CMAKE_BUILD_TYPE>`` which come after on the compile
line.  Therefore, setting ``CMAKE_<LANG>_FLAGS`` and
``<TRIBITS_PACKAGE>_<LANG>_FLAGS`` should only be used for options that will
not get overridden by the debug or release compiler flags in
``CMAKE_<LANG>_FLAGS_<CMAKE_BUILD_TYPE>``.  However, setting
``CMAKE_<LANG>_FLAGS`` will work well for adding extra compiler defines
(e.g. -DSOMETHING) for example.

WARNING: Any options that you set through the cache variable
``CMAKE_<LANG>_FLAGS_<CMAKE_BUILD_TYPE>`` will get overridden in the
<Project> CMake system for GNU compilers in development mode so don't try to
manually set ``CMAKE_<LANG>_FLAGS_<CMAKE_BUILD_TYPE>`` directly!  To
override those options, see
``CMAKE_<LANG>_FLAGS_<CMAKE_BUILD_TYPE>_OVERRIDE`` below.

Overriding CMAKE_BUILD_TYPE debug/release compiler options
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

To override the default CMake-set options in
``CMAKE_<LANG>_FLAGS_<CMAKE_BUILD_TYPE>``, use::

  -D CMAKE_<LANG>_FLAGS_<CMAKE_BUILD_TYPE>_OVERRIDE="<OPTIONS_TO_OVERRIDE>"

For example, to default debug options use::

  -D CMAKE_C_FLAGS_DEBUG_OVERRIDE="-g -O1" \
  -D CMAKE_CXX_FLAGS_DEBUG_OVERRIDE="-g -O1"
  -D CMAKE_Fortran_FLAGS_DEBUG_OVERRIDE="-g -O1"

and to override default release options use::

  -D CMAKE_C_FLAGS_RELEASE_OVERRIDE="-O3 -funroll-loops" \
  -D CMAKE_CXX_FLAGS_RELEASE_OVERRIDE="-03 -funroll-loops"
  -D CMAKE_Fortran_FLAGS_RELEASE_OVERRIDE="-03 -funroll-loops"

NOTES: The TriBITS CMake cache variable
``CMAKE_<LANG>_FLAGS_<CMAKE_BUILD_TYPE>_OVERRIDE`` is used and not
``CMAKE_<LANG>_FLAGS_<CMAKE_BUILD_TYPE>`` because is given a default
internally by CMake and the new varaible is needed to make the override
explicit.

Appending arbitrary libraries and link flags every executable
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

In order to append any set of arbitrary libraries and link flags to your
executables use::

  -D<Project>_EXTRA_LINK_FLAGS="<EXTRA_LINK_LIBRARIES>" \
  -DCMAKE_EXE_LINKER_FLAGS="<EXTRA_LINK_FLAGG>"

Above, you can pass any type of library and they will always be the last
libraries listed, even after all of the TPLs.

NOTE: This is how you must set extra libraries like Fortran libraries and
MPI libraries (when using raw compilers).  Please only use this variable
as a last resort.

NOTE: You must only pass in libraries in ``<Project>_EXTRA_LINK_FLAGS`` and
*not* arbitrary linker flags.  To pass in extra linker flags that are not
libraries, use the built-in CMake variable ``CMAKE_EXE_LINKER_FLAGS``
instead.  The TriBITS variable ``<Project>_EXTRA_LINK_FLAGS`` is badly named
in this respect but the name remains due to backward compatibility
requirements.

.. _<TRIBITS_PACKAGE>_DISABLE_STRONG_WARNINGS:

Turning off strong warnings for individual packages
+++++++++++++++++++++++++++++++++++++++++++++++++++

To turn off strong warnings (for all langauges) for a given TriBITS
package, set::

  -D <TRIBITS_PACKAGE>_DISABLE_STRONG_WARNINGS=ON

This will only affect the compilation of the sources for
``<TRIBITS_PACKAGES>``, not warnings generated from the header files in
downstream packages or client code.

Note that strong warnings are only enabled by default in development mode
(``<Project>_ENABLE_DEVELOPMENT_MODE==ON``) but not release mode
(``<Project>_ENABLE_DEVELOPMENT_MODE==ON``).  A release of <Project> should
therefore not have strong warning options enabled.

Overriding all (strong warnings and debug/release) compiler options
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

To override all compiler options, including both strong warning options
and debug/release options, configure with::

  -D CMAKE_C_FLAGS="-O3 -funroll-loops" \
  -D CMAKE_CXX_FLAGS="-03 -fexceptions" \
  -D CMAKE_BUILD_TYPE=NONE \
  -D <Project>_ENABLE_STRONG_C_COMPILE_WARNINGS=OFF \
  -D <Project>_ENABLE_STRONG_CXX_COMPILE_WARNINGS=OFF \
  -D <Project>_ENABLE_SHADOW_WARNINGS=OFF \
  -D <Project>_ENABLE_COVERAGE_TESTING=OFF \
  -D <Project>_ENABLE_CHECKED_STL=OFF \

NOTE: Options like ``<Project>_ENABLE_SHADOW_WARNINGS``,
``<Project>_ENABLE_COVERAGE_TESTING``, and ``<Project>_ENABLE_CHECKED_STL``
do not need to be turned off by default but they are shown above to make it
clear what other CMake cache variables can add compiler and link arguments.

NOTE: By setting ``CMAKE_BUILD_TYPE=NONE``, then ``CMAKE_<LANG>_FLAGS_NONE``
will be empty and therefore the options set in ``CMAKE_<LANG>_FLAGS`` will
be all that is passed in.

Enable and disable shadowing warnings for all <Project> packages
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

To enable shadowing warnings for all <Project> packages (that don't already
have them turned on) then use::

  -D <Project>_ENABLE_SHADOW_WARNINGS=ON

To disable shadowing warnings for all <Project> packages (even those that
have them turned on by default) then use::

  -D <Project>_ENABLE_SHADOW_WARNINGS=OFF

NOTE: The default value is empty '' which lets each <Project> package
decide for itself if shadowing warnings will be turned on or off for that
package.

Removing warnings as errors for CLEANED packages
++++++++++++++++++++++++++++++++++++++++++++++++

To remove the ``-Werror`` flag (or some other flag that is set) from being
applied to compile CLEANED packages like Teuchos, set the following when
configuring::

  -D <Project>_WARNINGS_AS_ERRORS_FLAGS=""

Adding debug symbols to the build
+++++++++++++++++++++++++++++++++

To get the compiler to add debug symbols to the build, configure with::

  -D <Project>_ENABLE_DEBUG_SYMBOLS=ON

This will add ``-g`` on most compilers.  NOTE: One does **not** generally
need to create a fully debug build to get debug symbols on most compilers.


Enabling support for C++11
--------------------------

To enable support for C++11 in packages that support C++11 (either optionally
or required), configure with::

  -D <Project>_ENABLE_CXX11=ON

By default, the system will try to automatically find compiler flags that will
enable C++11 features.  If it finds flags that allow a test C++11 program to
compile, then it will an additional set of configure-time tests to see if
several C++11 features are actually supported by the configured C++ compiler
and support will be disabled if all of these features are not supported.

In order to pre-set and/or override the C++11 compiler flags used, set the
cache variable::

  -D <Project>_CXX11_FLAGS="<compiler flags>"


Enabling explicit template instantiation for C++
------------------------------------------------

To enable explicit template instantiation for C++ code for packages that
support it, configure with::

  -D <Project>_ENABLE_EXPLICIT_INSTANTIATION=ON

When ``OFF``, all packages that have templated C++ code will use implicit
template instantiation.

Explicit template instantiation can be enabled (``ON``) or disabled (``OFF``)
for individual packages with::


  -D <TRIBITS_PACKAGE>_ENABLE_EXPLICIT_INSTANTIATION=[ON|OFF]

The default value for ``<TRIBITS_PACKAGE>_ENABLE_EXPLICIT_INSTANTIATION`` is
set by ``<Project>_ENABLE_EXPLICIT_INSTANTIATION``.

For packages that support it, explicit template instantation can massively
reduce the compile times for the C++ code involved.  To see what packages
support explicit instantation just search the CMakeCache.txt file for varibles
with ``ENABLE_EXPLICIT_INSTANTIATION`` in the name.


Disabling the Fortran compiler and all Fortran code
---------------------------------------------------

To disable the Fortran compiler and all <Project> code that depends on Fortran
set::

  -D <Project>_ENABLE_Fortran=OFF

NOTE: The fortran compiler may be disabled automatically by default on
systems like MS Windows.

NOTE: Most Apple Macs do not come with a compatible Fortran compiler by
default so you must turn off Fortran if you don't have a compatible Fortran
compiler.


Enabling runtime debug checking
-------------------------------
  
a) Enabling <Project> ifdefed runtime debug checking:

  To turn on optional ifdefed runtime debug checking, configure with::

    -D <Project>_ENABLE_DEBUG=ON

  This will result in a number of ifdefs to be enabled that will perform a
  number of runtime checks.  Nearly all of the debug checks in <Project> will
  get turned on by default by setting this option.  This option can be set
  independent of ``CMAKE_BUILD_TYPE`` (which sets the compiler debug/release
  options).

  NOTES:

  * The variable ``CMAKE_BUILD_TYPE`` controls what compiler options are
    passed to the compiler by default while ``<Project>_ENABLE_DEBUG``
    controls what defines are set in config.h files that control ifdefed debug
    checks.

  * Setting ``-DCMAKE_BUILD_TYPE=DEBUG`` will automatically set the
    default ``<Project>_ENABLE_DEBUG=ON``.

b) Enabling checked STL implementation:

  To turn on the checked STL implementation set::

    -D <Project>_ENABLE_CHECKED_STL=ON

  NOTES:

  * By default, this will set -D_GLIBCXX_DEBUG as a compile option for all C++
    code.  This only works with GCC currently.

  * This option is disabled by default because to enable it by default can
    cause runtime segfaults when linked against C++ code that was compiled
    without -D_GLIBCXX_DEBUG.


Configuring with MPI support
----------------------------

To enable MPI support you must minimally set::

  -D TPL_ENABLE_MPI=ON

There is built-in logic to try to find the various MPI components on your
system but you can override (or make suggestions) with::

  -D MPI_BASE_DIR="path"

(Base path of a standard MPI installation which has the subdirs 'bin', 'libs',
'include' etc.)

or::

  -D MPI_BIN_DIR="path1;path2;...;pathn"

which sets the paths where the MPI executables (e.g. mpiCC, mpicc, mpirun,
mpiexec) can be found.  By default this is set to ``${MPI_BASE_DIR}/bin`` if
``MPI_BASE_DIR`` is set.

**NOTE:** TriBITS uses the MPI compiler wrappers (e.g. mpiCC, mpicc, mpic++,
mpif90, etc.) which is more standard with other builds systems for HPC
computing using MPI (and the way that MPI implementations were meant to be
used).  But directly using the MPI compiler wrappers as the direct compilers
is inconsistent with the way that the standard CMake module ``FindMPI.cmake``
which tries to "unwrap" the compiler wrappers and grab out the raw underlying
compilers and the raw compiler and linker command-line arguments.  In this
way, TriBITS is more consistent with standard usage in the HPC community but
is less consistent with CMake (see "HISTORICAL NOTE" below).

There are several different different variations for configuring with MPI
support:

a) **Configuring build using MPI compiler wrappers:**

  The MPI compiler wrappers are turned on by default.  There is built-in logic
  in TriBITS that will try to find the right MPI compiler wrappers.  However,
  you can specifically select them by setting, for example::

    -D MPI_C_COMPILER:FILEPATH=mpicc \
    -D MPI_CXX_COMPILER:FILEPATH=mpic++ \
    -D MPI_Fortan_COMPILER:FILEPATH=mpif77

  which gives the name of the MPI C/C++/Fortran compiler wrapper executable.
  In this case, just the names of the programs are given and absolute path of
  the executables will be searched for under ``${MPI_BIN_DIR}/`` if the cache
  variable ``MPI_BIN_DIR`` is set, or in the default path otherwise.  The
  found programs will then be used to set the cache variables
  ``CMAKE_[C,CXX,Fortran]_COMPILER``.

  One can avoid the search and just use the absolute paths with, for example::

    -D MPI_C_COMPILER:FILEPATH=/opt/mpich/bin/mpicc \
    -D MPI_CXX_COMPILER:FILEPATH=/opt/mpich/bin/mpic++ \
    -D MPI_Fortan_COMPILER:FILEPATH=/opt/mpich/bin/mpif77

  However, you can also directly set the variables
  ``CMAKE_[C,CXX,Fortran]_COMPILER`` with, for example::

    -D CMAKE_C_COMPILER:FILEPATH=/opt/mpich/bin/mpicc \
    -D CMAKE_CXX_COMPILER:FILEPATH=/opt/mpich/bin/mpic++ \
    -D CMAKE_Fortan_COMPILER:FILEPATH=/opt/mpich/bin/mpif77

  **WARNING:** If you set just the compiler names and not the absolute paths
  with ``CMAKE_<LANG>_COMPILER`` in MPI mode, then a search will not be done
  and these will be expected to be in the path at build time. (Note that his
  is inconsistent the behavior of raw CMake in non-MPI mode described in
  `Selecting compiler and linker options`_).  If both
  ``CMAKE_<LANG>_COMPILER`` and ``MPI_<LANG>_COMPILER`` are set, however, then
  ``CMAKE_<LANG>_COMPILER`` will be used and ``MPI_<LANG>_COMPILER`` will be
  ignored.

  Note that when ``USE_XSDK_DEFAULTS=FALSE`` (see `xSDK Configuration
  Options`_), then the environment variables ``CC``, ``CXX`` and ``FC`` are
  ignored.  But when ``USE_XSDK_DEFAULTS=TRUE`` and the CMake cache variables
  ``CMAKE_[C,CXX,Fortran]_COMPILER`` are not set, then the environment
  variables ``CC``, ``CXX`` and ``FC`` will be used for
  ``CMAKE_[C,CXX,Fortran]_COMPILER``, even if the CMake cache variables
  ``MPI_[C,CXX,Fortran]_COMPILER`` are set!  So if one wants to make sure and
  set the MPI compilers irrespective of the xSDK mode, then one should set
  cmake cache variables ``CMAKE_[C,CXX,Fortran]_COMPILER`` to the absolute
  path of the MPI compiler wrappers.

  **HISTORICAL NOTE:** The TriBITS system has its own custom MPI integration
  support and does not (currently) use the standard CMake module
  ``FindMPI.cmake``.  This custom support for MPI was added to TriBITS in 2008
  when it was found the built-in ``FindMPI.cmake`` module was not sufficient
  for the needs of Trilinos and the approach taken by the module (still in use
  as of CMake 3.4.x) which tries to unwrap the raw compilers and grab the list
  of include directories, link libraries, etc, was not sufficiently portable
  for the systems where Trilinos needed to be used.  But earlier versions of
  TriBITS used the ``FindMPI.cmake`` module and that is why the CMake cache
  variables ``MPI_[C,CXX,Fortran]_COMPILER`` are defined and still supported.

b) **Configuring to build using raw compilers and flags/libraries:**

  While using the MPI compiler wrappers as described above is the preferred
  way to enable support for MPI, you can also just use the raw compilers and
  then pass in all of the other information that will be used to compile and
  link your code.

  To turn off the MPI compiler wrappers, set::

    -D MPI_USE_COMPILER_WRAPPERS=OFF

  You will then need to manually pass in the compile and link lines needed to
  compile and link MPI programs.  The compile flags can be set through::

    -D CMAKE_[C,CXX,Fortran]_FLAGS="$EXTRA_COMPILE_FLAGS"

  The link and library flags must be set through::

    -D <Project>_EXTRA_LINK_FLAGS="$EXTRA_LINK_FLAGS"

  Above, you can pass any type of library or other linker flags in and they
  will always be the last libraries listed, even after all of the TPLs.

  NOTE: A good way to determine the extra compile and link flags for MPI is to
  use::

    export EXTRA_COMPILE_FLAGS="`$MPI_BIN_DIR/mpiCC --showme:compile`"
    
    export EXTRA_LINK_FLAGS="`$MPI_BIN_DIR/mpiCC --showme:link`"
    
  where ``MPI_BIN_DIR`` is set to your MPI installations binary directory.

c) **Setting up to run MPI programs:**

  In order to use the ctest program to run MPI tests, you must set the mpi
  run command and the options it takes.  The built-in logic will try to find
  the right program and options but you will have to override them in many
  cases.

  MPI test and example executables are passed to CTest ``ADD_TEST()`` as::

    ADD_TEST(
      ${MPI_EXEC} ${MPI_EXEC_PRE_NUMPROCS_FLAGS}
      ${MPI_EXEC_NUMPROCS_FLAG} <NP>
      ${MPI_EXEC_POST_NUMPROCS_FLAGS}
      <TEST_EXECUTABLE_PATH> <TEST_ARGS> )

  where ``<TEST_EXECUTABLE_PATH>``, ``<TEST_ARGS>``, and ``<NP>`` are specific
  to the test being run.

  The test-independent MPI arguments are::

    -D MPI_EXEC:FILEPATH="exec_name"

  (The name of the MPI run command (e.g. mpirun, mpiexec) that is used to run
  the MPI program.  This can be just the name of the program in which case
  the full path will be looked for in ``${MPI_BIN_DIR}`` as described above.
  If it is an absolute path, it will be used without modification.)

  ::

    -D MPI_EXEC_DEFAULT_NUMPROCS=4

  (The default number of processes to use when setting up and running
  MPI test and example executables.  The default is set to '4' and only
  needs to be changed when needed or desired.)

  ::

    -D MPI_EXEC_MAX_NUMPROCS=4

  (The maximum number of processes to allow when setting up and running MPI
  test and example executables.  The default is set to '4' but should be set
  to the largest number that can be tolerated for the given machine.  Tests
  with more processes than this are excluded from the test suite at configure
  time.)

  ::

    -D MPI_EXEC_NUMPROCS_FLAG=-np

  (The command-line option just before the number of processes to use
  ``<NP>``.  The default value is based on the name of ``${MPI_EXEC}``, for
  example, which is ``-np`` for OpenMPI.)

  ::

    -D MPI_EXEC_PRE_NUMPROCS_FLAGS="arg1;arg2;...;argn"

  (Other command-line arguments that must come *before* the numprocs
  argument.  The default is empty "".)

  ::

    -D MPI_EXEC_POST_NUMPROCS_FLAGS="arg1;arg2;...;argn"

  (Other command-line arguments that must come *after* the numprocs
  argument.  The default is empty "".)

  NOTE: Multiple arguments listed in ``MPI_EXEC_PRE_NUMPROCS_FLAGS`` and
  ``MPI_EXEC_POST_NUMPROCS_FLAGS`` must be quoted and seprated by ``';'`` as
  these variables are interpreted as CMake arrays.

Configuring for OpenMP support
------------------------------

To enable OpenMP support, one must set::

  -D <Project>_ENABLE_OpenMP=ON

Note that if you enable OpenMP directly through a compiler option (e.g.,
``-fopenmp``), you will NOT enable OpenMP inside <Project> source code.

.. _BUILD_SHARED_LIBS:

Building shared libraries
-------------------------

To configure to build shared libraries, set::

  -D BUILD_SHARED_LIBS=ON

The above option will result in all shared libraries to be build on all
systems (i.e., ``.so`` on Unix/Linux systems, ``.dylib`` on Mac OS X, and
``.dll`` on Windows systems).

NOTE: If the project has ``USE_XSDK_DEFAULTS=ON`` set, then this will set
``BUILD_SHARED_LIBS=TRUE`` by default.  Otherwise, the default is
``BUILD_SHARED_LIBS=FALSE``


Building static libraries and executables
-----------------------------------------

To build static libraries, turn off the shared library support::

 -D BUILD_SHARED_LIBS=OFF

Some machines, such as the Cray XT5, require static executables.  To build
<Project> executables as static objects, a number of flags must be set::

 -D BUILD_SHARED_LIBS=OFF \
 -D TPL_FIND_SHARED_LIBS=OFF \
 -D <Project>_LINK_SEARCH_START_STATIC=ON

The first flag tells cmake to build static versions of the <Project>
libraries.  The second flag tells cmake to locate static library versions of
any required TPLs.  The third flag tells the autodetection routines that
search for extra required libraries (such as the mpi library and the gfortran
library for gnu compilers) to locate static versions.

NOTE: The flag ``<Project>_LINK_SEARCH_START_STATIC`` is only supported in
cmake version 2.8.5 or higher.  The variable will be ignored in prior releases
of cmake.


Enabling support for an optional Third-Party Library (TPL)
----------------------------------------------------------

To enable a given TPL, set::

  -D TPL_ENABLE_<TPLNAME>=ON

where ``<TPLNAME>`` = ``Boost``, ``ParMETIS``, etc.

The headers, libraries, and library directories can then be specified with
the input cache variables:

* ``<TPLNAME>_INCLUDE_DIRS:PATH``: List of paths to the header include
  directories.  For example::

    -D Boost_INCLUDE_DIRS=/usr/local/boost/include

* ``<TPLNAME>_LIBRARY_NAMES:STRING``: List of unadorned library names, in the
  order of the link line.  The platform-specific prefixes (e.g.. 'lib') and
  postfixes (e.g. '.a', '.lib', or '.dll') will be added automatically by
  CMake.  For example::

    -D BLAS_LIBRARY_NAMES="blas;gfortran"

* ``<TPLNAME>_LIBRARY_DIRS:PATH``: The list of directories where the library
  files can be found.  For example::

    -D BLAS_LIBRARY_DIRS=/usr/local/blas

The variables ``TPL_<TPLNAME>_INCLUDE_DIRS`` and ``TPL_<TPLNAME>_LIBRARIES``
are what are directly used by the TriBITS dependency infrastructure.  These
variables are normally set by the variables ``<TPLNAME>_INCLUDE_DIRS``,
``<TPLNAME>_LIBRARY_NAMES``, and ``<TPLNAME>_LIBRARY_DIRS`` using CMake
``find`` commands but one can always override these by directly setting these
cache variables ``TPL_<TPLNAME>_INCLUDE_DIRS`` and
``TPL_<TPLNAME>_LIBRARIES``, for example, as::

  -D TPL_Boost_INCLUDE_DIRS=/usr/local/boost/include \
  -D TPL_Boost_LIBRARIES="/user/local/boost/lib/libprogram_options.a;..."

This gives the user complete and direct control in specifying exactly what is
used in the build process.  The other variables that start with ``<TPLNAME>_``
are just a convenience to make it easier to specify the location of the
libraries.

In order to allow a TPL that normally requires one or more libraries to ignore
the libraries, one can set ``<TPLNAME>_LIBRARY_NAMES`` to empty, for example::

  -D BLAS_LIBRARY_NAMES=""

Optional package-specific support for a TPL can be turned off by setting::

  -D <TRIBITS_PACKAGE>_ENABLE_<TPLNAME>=OFF

This gives the user full control over what TPLs are supported by which package
independently.

Support for an optional TPL can also be turned on implicitly by setting::

  -D <TRIBITS_PACKAGE>_ENABLE_<TPLNAME>=ON

where ``<TRIBITS_PACKAGE>`` is a TriBITS package that has an optional
dependency on ``<TPLNAME>``.  That will result in setting
``TPL_ENABLE_<TPLNAME>=ON`` internally (but not set in the cache) if
``TPL_ENABLE_<TPLNAME>=OFF`` is not already set.

WARNING: Do *not* try to hack the system and set::

  TPL_BLAS_LIBRARIES="-L/some/dir -llib1 -llib2 ..."

This is not compatible with proper CMake usage and it not guaranteed
to be supported.

If all the parts of a TPL are not found on an initial configure, then one can
change the variables ``<TPLNAME>_INCLUDE_DIRS``, ``<TPLNAME>_LIBRARY_NAMES``,
and/or ``<TPLNAME>_LIBRARY_DIRS``.  One can do this over and over until the
TPL is found. By reconfiguring, one avoid a complete configure from scrath
which saves time.


Disabling support for a Third-Party Library (TPL)
--------------------------------------------------

Disabling a TPL explicitly can be done using::

  -D TPL_ENABLE_<TPLNAME>=OFF

NOTE: If a disabled TPL is a required dependency of some explicitly enabled
downstream package, then the configure will error out if
<Project>_DISABLE_ENABLED_FORWARD_DEP_PACKAGES=OFF.  Otherwise, a WARNING will
be printed and the downstream package will be disabled and configuration will
continue.


Disabling tentatively enabled TPLs
----------------------------------

To disable a tentatively enabled TPL, set::

  -D TPL_ENABLE_<TPLNAME>=OFF

where ``<TPLNAME>`` = ``BinUtils``, ``Boost``, etc.

NOTE: Some TPLs in <Project> are always tentatively enabled (e.g. BinUtils
for C++ stacktracing) and if all of the components for the TPL are found
(e.g. headers and libraries) then support for the TPL will be enabled,
otherwise it will be disabled.  This is to allow as much functionality as
possible to get automatically enabled without the user having to learn about
the TPL, explicitly enable the TPL, and then see if it is supported or not
on the given system.  However, if the TPL is not supported on a given
platform, then it may be better to explicitly disable the TPL (as shown
above) so as to avoid the output from the CMake configure process that shows
the tentatively enabled TPL being processes and then failing to be enabled.
Also, it is possible that the enable process for the TPL may pass, but the
TPL may not work correctly on the given platform.  In this case, one would
also want to explicitly disable the TPL as shown above.


Disable warnings from TPL header files
--------------------------------------

To disable warnings coming from included TPL header files for C and C++ code,
set::

  -D<Project>_TPL_SYSTEM_INCLUDE_DIRS=TRUE

On some systems and compilers (e.g. GNU), that will result is include
directories for all TPLs to be passed in to the compiler using ``-isystem``
instead of ``-I``.

WARNING: On some systems this will result in build failures involving gfortran
and module files.  Therefore, don't enable this if Fortran code in your
project is pulling in module files from TPLs.


xSDK Configuration Options
--------------------------

The configure of <Project> will adhere to the xSDK configuration standard
(todo: put in reference to final document) simply by setting the CMake cache
variable::

  -D USE_XSDK_DEFAULTS=TRUE

Setting this will have the following impact:

* ``BUILD_SHARED_LIBS`` will be set to ``TRUE`` by default instead of
  ``FALSE``, which is the default for raw CMake projects (see `Building shared
  libraries`_).

* ``CMAKE_BUILD_TYPE`` will be set to ``DEBUG`` by default instead of
  ``RELEASE`` which is the standard TriBITS default (see `CMAKE_BUILD_TYPE`_).

* The compilers in MPI mode ``TPL_ENABLE_MPI=ON`` or serial mode
  ``TPL_ENABLE_MPI=OFF`` will be read from the environment variables ``CC``,
  ``CXX`` and ``FC`` if they are set but the cmake cache variables
  ``CMAKE_C_COMPILER``, ``CMAKE_C_COMPILER`` and ``CMAKE_C_COMPILER`` are not
  set.  Otherwise, the TriBITS default behavior is to ignore these environment
  variables in MPI mode.

* The Fortran flags will be read from environment variable ``FCFLAGS`` if the
  environment variable ``FFLAGS`` and the CMake cache variable
  ``CMAKE_Fortran_FLAGS`` are empty.  Otherwise, raw CMake ignores ``FCFLAGS``
  (see `Adding arbitrary compiler flags but keeping default build-type
  flags`_).

The rest of the required xSDK configure standard is automatically satisfied by
every TriBITS CMake project, including the <Project> project.


Generating verbose output
-------------------------

There are several different ways to generate verbose output to debug problems
when they occur:

.. _<Project>_TRACE_FILE_PROCESSING:

a) **Trace file processing during configure:**

  ::

    -D <Project>_TRACE_FILE_PROCESSING=ON

  This will cause TriBITS to print out a trace for all of the project's,
  repositorie's, and package's files get processed on lines using the prefix
  ``File Trace:``.  This shows what files get processed and in what order they
  get processed.  To get a clean listing of all the files processed by TriBITS
  just grep out the lines starting with ``-- File Trace:``.  This can be
  helpful in debugging configure problems without generating too much extra
  output.

  Note that `<Project>_TRACE_FILE_PROCESSING`_ is set to ``ON`` automatically
  when `<Project>_VERBOSE_CONFIGURE`_  = ``ON``.

.. _<Project>_VERBOSE_CONFIGURE:

b) **Getting verbose output from TriBITS configure:**

  To do a complete debug dump for the TriBITS configure process, use::

    -D <Project>_VERBOSE_CONFIGURE=ON

  This produces a *lot* of output but can be very useful when debugging
  configuration problems.

  To just dump the package and TPL dependencies, use::

    -D <Project>_DUMP_PACKAGE_DEPENDENCIES=ON

  To just dump the link libraries for each library and executable created,
  use::

    -D <Project>_DUMP_LINK_LIBS=ON

  Of course ``<Project>_DUMP_PACKAGE_DEPENDENCIES`` and
  ``<Project>_DUMP_LINK_LIBS`` can be used together.  Also, note that
  ``<Project>_DUMP_PACKAGE_DEPENDENCIES`` and ``<Project>_DUMP_LINK_LIBS``
  both default t ``ON`` when ``<Project>_VERBOSE_CONFIGURE=ON`` on the first
  configure.
 

c) **Getting verbose output from the makefile:**

  ::

    -D CMAKE_VERBOSE_MAKEFILE=TRUE

  NOTE: It is generally better to just pass in ``VERBOSE=`` when directly
  calling ``make`` after configuration is finihsed.  See `Building with
  verbose output without reconfiguring`_.

d) **Getting very verbose output from configure:**

  ::

    -D <Project>_VERBOSE_CONFIGURE=ON --debug-output --trace

  NOTE: This will print a complete stack trace to show exactly where you are.


Enabling/disabling deprecated warnings
--------------------------------------

To turn off all deprecated warnings, set::

  -D <Project>_SHOW_DEPRECATED_WARNINGS=OFF

This will disable, by default, all deprecated warnings in packages in
<Project>.  By default, deprecated warnings are enabled.

To enable/disable deprecated warnings for a single <Project> package, set::

  -D <TRIBITS_PACKAGE>_SHOW_DEPRECATED_WARNINGS=OFF

This will override the global behavior set by
``<Project>_SHOW_DEPRECATED_WARNINGS`` for individual package
``<TRIBITS_PACKAGE>``.


Disabling deprecated code
-------------------------

To actually disable and remove deprecated code from being included in
compilation, set::

  -D <Project>_HIDE_DEPRECATED_CODE=ON

and a subset of deprecated code will actually be removed from the build.  This
is to allow testing of downstream client code that might otherwise ignore
deprecated warnings.  This allows one to certify that a downstream client code
is free of calling deprecated code.

To hide deprecated code for a single <Project> package set::

  -D <TRIBITS_PACKAGE>_HIDE_DEPRECATED_CODE=ON

This will override the global behavior set by
``<Project>_HIDE_DEPRECATED_CODE`` for individual package
``<TRIBITS_PACKAGE>``.


Outputting package dependency information
-----------------------------------------

To generate the various XML and HTML package dependency files, one can set the
output directory when configuring using::

  -D <Project>_DEPS_DEFAULT_OUTPUT_DIR:FILEPATH=<SOME_PATH>

This will generate, by default, the output files
<Project>PackageDependencies.xml, <Project>PackageDependenciesTable.html, and
CDashSubprojectDependencies.xml.

The filepath for <Project>PackageDependencies.xml can be overridden using::

  -D <Project>_DEPS_XML_OUTPUT_FILE:FILEPATH=<SOME_FILE_PATH>

The filepath for <Project>PackageDependenciesTable.html can be overridden
using::

  -D <Project>_DEPS_HTML_OUTPUT_FILE:FILEPATH=<SOME_FILE_PATH>

The filepath for CDashSubprojectDependencies.xml can be overridden using::

  -D <Project>_CDASH_DEPS_XML_OUTPUT_FILE:FILEPATH=<SOME_FILE_PATH>

NOTES:

* One must start with a clean CMake cache for all of these defaults to work.

* The files <Project>PackageDependenciesTable.html and
  CDashSubprojectDependencies.xml will only get generated if support for
  Python is enabled.


Enabling different test categories
----------------------------------

To turn on a set a given set of tests by test category, set::

  -D <Project>_TEST_CATEGORIES="<CATEGORY0>;<CATEGORY1>;..." 

Valid categories include ``BASIC``, ``CONTINUOUS``, ``NIGHTLY``, ``HEAVY`` and
``PERFORMANCE``.  ``BASIC`` tests get built and run for pre-push testing, CI
testing, and nightly testing.  ``CONTINUOUS`` tests are for post-push testing
and nightly testing.  ``NIGHTLY`` tests are for nightly testing only.
``HEAVY`` tests are for more expensive tests that require larger number of MPI
processes and llonger run times.  ``PERFORMANCE`` tests a special category
used only for performance testing.


Disabling specific tests
------------------------

Any TriBTS added ctest test (i.e. listed in ``ctest -N``) can be disabled at
configure time by setting::

  -D <fullTestName>_DISABLE=ON

where ``<fulltestName>`` must exactly match the test listed out by ``ctest
-N``.  Of course specific tests can also be excluded from ``ctest`` using the
``-E`` argument.


Trace test addition or exclusion
--------------------------------

To see what tests get added and see those that don't get added for various
reasons, configure with::

  -D <Project>_TRACE_ADD_TEST=ON

That will print one line per show that the test got added and if not then why
the test was not added (i.e. due to the test's ``COMM``, ``NUM_MPI_PROCS``,
``CATEGORIES``, ``HOST``, ``XHOST``, ``HOSTTYPE``, or ``XHOSTTYPE``
arguments).

.. _DART_TESTING_TIMEOUT:

Setting test timeouts at configure time
---------------------------------------

A maximum default time limit for all the tests can be set at configure time
using the cache variable::

  -D DART_TESTING_TIMEOUT=<maxSeconds>

where ``<maxSeconds>`` is the number of wall-clock seconds.  By default there
is no timeout limit set so it is a good idea to set some limit just so tests
don't hang and run forever.  For example, when an MPI program has a defect, it
can easily hang forever until it is manually killed.  If killed, CTest will
kill all of this child processes correctly.

NOTES:

* Be careful not set the timeout too low since if a machine becomes loaded
  tests can take longer to run and may result in timeouts that would not
  otherwise occur.
* Individual tests may have there timeout limit set on a test-by-test basis
  internally in the project's ``CMakeLists.txt`` files (see the ``TIMEOUT``
  argument for ``TRIBITS_ADD_TEST()`` and ``TRIBITS_ADD_ADVANCED_TEST()``).
  When this is the case, the global timeout set with ``DART_TESTING_TIMEOUT``
  has no impact on these individually set test timeouts.  To affect individual
  test timeouts set on a test-by-test basis, use
  `<Project>_SCALE_TEST_TIMEOUT_TESTING_TIMEOUT`_.
* To set or override the default global test timeout limit at runtime, see
  `Overridding test timeouts`_.

.. _<Project>_SCALE_TEST_TIMEOUT_TESTING_TIMEOUT:

Scaling test timeouts at configure time
---------------------------------------

The global default test timeout `DART_TESTING_TIMEOUT`_ as well as all of the
timeouts for the individual tests that have their own timeout set (through the
``TIMEOUT`` argument for each individual test) can be scaled by a constant
factor ``<testTimeoutScaleFactor>`` by configuring with::

  -D <Project>_SCALE_TEST_TIMEOUT=<testTimeoutScaleFactor>

Here, ``<testTimeoutScaleFactor>`` can be an integral number like ``5`` or can
be fractional number like ``1.5``.

This feature is generally used to compensate for slower machines or overloaded
test machines and therefore only scaling factors greater than 1 are to be
used.  The primary use case for this feature is to add large scale factors
(e.g. ``40`` to ``100``) to compensate for running tests using valgrind (see
`Running memory checking`_) but this can also be used for debug-mode builds
that create tests which run more slowly than for full release-mode optimized
builds.

NOTES:

* When scaling the timeouts, the timeout is first truncated to integral
  seconds so an original timeout like ``200.5`` will be truncated to ``200``
  before it gets scaled.

* Only the first fractional digit is used so ``1.57`` is truncated to ``1.5``
  before scaling the test timeouts.

* The cache value of the variable `DART_TESTING_TIMEOUT`_ is not changed in
  the ``CMakeCache.txt`` file.  Only the value of ``TimeOut`` written into the
  ``DartConfiguration.tcl`` file (which is directly read by ``ctest``) will be
  scaled.  (This ensures that running configure over and over again will not
  increase ``DART_TESTING_TIMEOUT`` each time.)


Enabling support for coverage testing
-------------------------------------

To turn on support for coverage testing set::

  -D <Project>_ENABLE_COVERAGE_TESTING=ON 

This will set compile and link options -fprofile-arcs -ftest-coverage for GCC.
Use 'make dashboard' (see below) to submit coverage results to CDash


Viewing configure options and documentation
-------------------------------------------
  
a) Viewing available configure-time options with documentation:

  ::

    $ cd $BUILD_DIR
    $ rm -rf CMakeCache.txt CMakeFiles/
    $ cmake -LAH -D <Project>_ENABLE_ALL_PACKAGES=ON \
      $SOURCE_BASE

  You can also just look at the text file CMakeCache.txt after configure which
  gets created in the build directory and has all of the cache variables and
  documentation.

b) Viewing available configure-time options without documentation:

  ::

    $ cd $BUILD_DIR
    $ rm -rf CMakeCache.txt CMakeFiles/
    $ cmake -LA <SAME_AS_ABOVE> $SOURCE_BASE

c) Viewing current values of cache variables:

  ::

    $ cmake -LA $SOURCE_BASE

  or just examine and grep the file CMakeCache.txt.


Enabling extra repositories with add-on packages:
-------------------------------------------------

.. _<Project>_EXTRA_REPOSITORIES:

To configure <Project> with an post extra set of packages in extra TriBITS
repositories, configure with::

  -D<Project>_EXTRA_REPOSITORIES="<REPO0>,<REPO1>,..."

Here, ``<REPOi>`` is the name of an extra repository that typically has been
cloned under the main <Project> source directory as::

  <Project>/<REPOi>/

For example, to add the packages from SomeExtraRepo one would configure as::

  $ cd $SOURCE_BASE_DIR
  $ git clone some_url.com/some/dir/SomeExtraRepo
  $ cd $BUILD_DIR
  $ ./do-configure -D<Project>_EXTRA_REPOSITORIES=SomeExtraRepo \
    [Other Options]

After that, all of the extra packages defined in ``SomeExtraRepo`` will appear
in the list of official <Project> packages (after all of the native packages)
and one is free to enable any of the defined add-on packages just like any
other native <Project> package.

NOTE: If ``<Project>_EXTRAREPOS_FILE`` and
``<Project>_ENABLE_KNOWN_EXTERNAL_REPOS_TYPE`` are specified, then the list of
extra repositories in ``<Project>_EXTRA_REPOSITORIES`` must be a subset and in
the same order as the list extra repos read in from the file specified by
`<Project>_EXTRAREPOS_FILE`_.  (Also see the variable
`<Project>_PRE_REPOSITORIES`_ as well.)


Enabling extra repositories through a file
------------------------------------------

.. _<Project>_EXTRAREPOS_FILE:

In order to provide the list of extra TriBIITS repositories containing add-on
packages from a file, configure with::

  -D<Project>_EXTRAREPOS_FILE:FILEPATH=<EXTRAREPOSFILE> \
  -D<Project>_ENABLE_KNOWN_EXTERNAL_REPOS_TYPE=Continuous

Specifing extra repositories through an extra repos file allows greater
flexibility in the specification of extra repos.  This is not helpful for a
basic configure of the project but is useful in automated testing using the
``TribitsCTestDriverCore.cmake`` script and the ``checkin-test.py`` script.

The valid values of ``<Project>_ENABLE_KNOWN_EXTERNAL_REPOS_TYPE`` include
``Continuous``, ``Nightly``, and ``Experimental``.  Only repositories listed
in the file ``<EXTRAREPOSFILE>`` that match this type will be included.  Note
that ``Nightly`` matches ``Continuous`` and ``Experimental`` matches
``Nightly`` and ``Continuous`` and therefore includes all repos by default.

If ``<Project>_IGNORE_MISSING_EXTRA_REPOSITORIES`` is set to ``TRUE``, then
any extra repositories selected who's directory is missing will be ignored.
This is useful when the list of extra repos that a given developer develops or
tests is variable and one just wants TriBITS to pick up the list of existing
repos automatically.

If the file ``<projectDir>/cmake/ExtraRepositoriesList.cmake`` exists, then it
is used as the default value for ``<Project>_EXTRAREPOS_FILE``.  However, the
default value for ``<Project>_ENABLE_KNOWN_EXTERNAL_REPOS_TYPE`` is empty so
no extra repostories are defined by default unless
``<Project>_ENABLE_KNOWN_EXTERNAL_REPOS_TYPE`` is specifically set to one of
the allowed values.

.. _<Project>_PRE_REPOSITORIES:

NOTE: The set of extra repositories listed in the file
``<Project>_EXTRAREPOS_FILE`` can be filtered down by setting the variables
``<Project>_PRE_REPOSITORIES`` if PRE extra repos are listed and/or
``<Project>_EXTRA_REPOSITORIES`` if POST extra repos are listed.

  
Reconfiguring completely from scratch
-------------------------------------

To reconfigure from scratch, one needs to delete the the ``CMakeCache.txt``
and base-level ``CMakeFiles/`` directory, for example, as::
  
  $ rm -rf CMakeCache.txt CMakeFiles/
  $ ./do-configure [options]
  
Removing the ``CMakeCache.txt`` file is often needed when removing variables
from the configure line since they are already in the cache.  Removing the
``CMakeFiles/`` directories is needed if there are changes in some CMake
modules or the CMake version itself.  However, usually removing just the
top-level ``CMakeCache.txt`` and ``CMakeFiles/`` directory is enough to
guarantee a clean reconfigure from a dirty build directory.

If one really wants a clean slate, then try::

  $ rm -rf `ls | grep -v do-configure`
  $ ./do-configure [options]

WARNING: Later versions of CMake (2.8.10.2+) require that you remove the
top-level ``CMakeFiles/`` directory whenever you remove the ``CMakeCache.txt``
file.


Viewing configure errors
-------------------------

To view various configure errors, read the file::

  $BUILD_BASE_DIR/CMakeFiles/CMakeError.log

This file contains detailed output from try-compile commands, Fortran/C name
managling determination, and other CMake-specific information.


Adding configure timers
-----------------------

To add timers to various configure steps, configure with::

  -D <Project>_ENABLE_CONFIGURE_TIMING=ON

This will do baulk timing for the major configure steps which is independent
of the number of packages in the project.

To additionally add timing for the configure of individual packages, configure
with::

  -D <Project>_ENABLE_CONFIGURE_TIMING=ON \
  -D <Project>_ENABLE_PACKAGE_CONFIGURE_TIMING=ON

If you are configuring a large number of packages (perhaps by including a lot
of add-on packages in extra repos) then you might not want to enable
package-by-package timing since it can add some significant overhead to the
configure times.

If you just want to time individual packages instead, you can enable that
with::

  -D <Project>_ENABLE_CONFIGURE_TIMING=ON \
  -D <TRIBITS_PACKAGE_0>_PACKAGE_CONFIGURE_TIMING=ON \
  -D <TRIBITS_PACKAGE_1>_PACKAGE_CONFIGURE_TIMING=ON \
  ...

NOTES:

* This requires that you are running on a Linux/Unix system that has the
  standard shell command ``date``.  CMake does not have built-in timing
  functions so this system command needs to be used instead.  This will report
  timings to 0.001 seconds but note that the overall configure time will go up
  due to the increased overhead of calling ``date`` as a process shell
  command.

* '''WARNING:''' Because this feature has to call the ``data`` using CMake's
  ``EXECUTE_PROCESS()`` command, it can be expensive.  Therefore, this should
  really only be turned on for large projects (where the extra overhead is
  small) or for smaller projects for extra informational purposes.

Generating export files
-----------------------

The project <Project> can generate export files for external CMake projects or
external Makefile projects.  These export files provide the lists of
libraries, include directories, compilers and compiler options, etc.

To configure to generate CMake export files for the project, configure with::

   -D <Project>_ENABLE_INSTALL_CMAKE_CONFIG_FILES=ON

This will generate the file ``<Project>Config.cmake`` for the project and the
files ``<Package>Config.cmake`` for each enabled package in the build tree.
In addition, this will install versions of these files into the install tree.

To confiugre Makefile export files, configure with::

  -D <Project>_ENABLE_EXPORT_MAKEFILES=ON

which will generate the file ``Makefile.export.<Project>`` for the project and
the files ``Makefile.export.<Package>`` for each enabled package in the build
tree.  In addition, this will install versions of these files into the install
tree.

The list of export files generated can be reduced by specifying the exact list
of packages the files are requested for with::

  -D <Project>_GENERATE_EXPORT_FILES_FOR_ONLY_LISTED_SE_PACKAGES="<pkg0>;<pkg1>"

NOTES:

* Only enabled packages will have their export files generated.

* One would only want to limit the export files generated for very large
  projects where the cost my be high for doing so.


Generating a project repo version file
--------------------------------------

In development mode working with local git repos for the project sources, on
can generate a <Project>RepoVersion.txt file which lists all of the repos and
their current versions using::

   -D <Project>_GENERATE_REPO_VERSION_FILE=ON

This will cause a <Project>RepoVersion.txt file to get created in the binary
directory, get installed in the install directory, and get included in the
source distribution tarball.


CMake configure-time development mode and debug checking
--------------------------------------------------------

To turn off CMake configure-time development-mode checking, set::

  -D <Project>_ENABLE_DEVELOPMENT_MODE=OFF

This turns off a number of CMake configure-time checks for the <Project>
TriBITS/CMake files including checking the package dependencies.  These checks
can be expensive and may also not be appropriate for a tarball release of the
software.  However, this also turns off strong compiler warnings so this is
not recommended by default (see `<TRIBITS_PACKAGE>_DISABLE_STRONG_WARNINGS`_).
For a release of <Project> this option is set OFF by default.

One of the CMake configure-time debug-mode checks performed as part of
``<Project>_ENABLE_DEVELOPMENT_MODE=ON`` is to assert the existence of TriBITS
package directories.  In development mode, the failure to find a package
directory is usually a programming error (i.e. a miss-spelled package
directory name).  But in a tarball release of the project, package directories
may be purposefully missing (see `Creating a tarball of the source tree`_) and
must be ignored.  When building from a reduced tarball created from the
development sources, set::

  -D <Project>_ASSERT_MISSING_PACKAGES=OFF

Setting this off will cause the TriBITS CMake configure to simply ignore any
missing packages and turn off all dependencies on these missing packages.

Another type of checking is for optional inserted/external packages
(e.g. packages who's source can optionally be included in and is flagged with
``TRIBITS_ALLOW_MISSING_EXTERNAL_PACKAGES()``).  Any of these package
directories that are missing result in the packages being silently ignored by
default.  However, notes on what missing packages are being ignored can
printed by configuring with::

  -D <Project>_WARN_ABOUT_MISSING_EXTERNAL_PACKAGES=TRUE

These warnings (starting with 'NOTE', not 'WARNING' that would otherwise
trigger warnings in CDash) about missing inserted/external packages will print
regardless of the setting for ``<Project>_ASSERT_MISSING_PACKAGES``.


Building (Makefile generator)
=============================

This section described building using the default CMake Makefile generator.
TriBITS supports other CMake generators such as Visual Studio on Windows,
XCode on Macs, and Eclipe project files but using those build systems are not
documented here.

Building all targets
--------------------

To build all targets use::

  $ make [-jN]

where ``N`` is the number of processes to use (i.e. 2, 4, 16, etc.) .


Discovering what targets are available to build
-----------------------------------------------

CMake generates Makefiles with a 'help' target!  To see the targets at the
current directory level type::

  $ make help

NOTE: In general, the ``help`` target only prints targets in the current
directory, not targets in subdirectories.  These targets can include object
files and all, anything that CMake defines a target for in the current
directory.  However, running ``make help`` it from the base build directory
will print all major targets in the project (i.e. libraries, executables,
etc.) but not minor targets like object files.  Any of the printed targets can
be used as a target for ``make <some-target>``.  This is super useful for just
building a single object file, for example.


Building all of the targets for a package
-----------------------------------------

To build only the targets for a given TriBITS package, one can use::

   $ make <TRIBITS_PACKAGE>_all

or::

   $ cd packages/<TRIBITS_PACKAGE>
   $ make

This will build only the targets for TriBITS package ``<TRIBITS_PACKAGE>`` and
its required upstream targets.


Building all of the libraries for a package
-------------------------------------------

To build only the libraries for given TriBITS package, use::

  $ make <TRIBITS_PACKAGE>_libs



Building all of the libraries for all enabled packages
------------------------------------------------------

To build only the libraries for all enabled TriBITS packages, use::

  $ make libs

NOTE: This target depends on the ``<PACKAGE>_libs`` targets for all of the
enabled ``<Project>`` packages.  You can also use the target name
``'<Project>_libs``.


Building a single object file
-----------------------------

To build just a single object file (i.e. to debug a compile problem), first,
look for the target name for the object file build based on the source file,
for example for the source file ``SomeSourceFile.cpp``, use::

  $ make help | grep SomeSourceFile

The above will return a target name like::

  ... SomeSourceFile.o

To find the name of the actual object file, do::

  $ find . -name "*SomeSourceFile*.o"

that will return something like::

  ./CMakeFiles/<source-dir-path>.dir/SomeSourceFile.cpp.o

(but this file location and name depends on the source directory structure,
the version of CMake, and other factors).  Use the returned name (exactly) for
the object file returned in the above find operation to remove the object file
first, for example, as::

  $ rm ./CMakeFiles/<source-dir-path>.dir/SomeSourceFile.cpp.o

and then build it again, for example, with::

  $ make SomeSourceFile.o

Again, the names of the target and the object file name an location depend on
the CMake version, the structure of your source directories and other factors
but the general process of using ``make help | grep <some-file-base-name>`` to
find the target name and then doing a find ``find . -name
"*<some-file-base-name>*"`` to find the actual object file path always works.

For this process to work correctly, you must be in the subdirectory where the
``TRIBITS_ADD_LIBRARY()`` or ``TRIBITS_ADD_EXECUTABLE()`` command is called
from its ``CMakeList.txt`` file, otherwise the object file targets will not be
listed by ``make help``.

NOTE: CMake does not seem to not check on dependencies when explicitly
building object files as shown above so you need to always delete the object
file first to make sure that it gets rebuilt correctly.


Building with verbose output without reconfiguring
--------------------------------------------------

One can get CMake to generate verbose make output at build type by just
setting the Makefile variable ``VERBOSE=1``, for example, as::

  $ make  VERBOSE=1 [<SOME_TARGET>]

Any number of compile or linking problem can be quickly debugged by seeing the
raw compile and link lines.  See `Building a single object file`_ for more
details.


Relink a target without considering dependencies
------------------------------------------------

CMake provides a way to rebuild a target without considering its dependencies
using::

  $ make <SOME_TARGET>/fast


Testing with CTest
==================

This section assumes one is using the CMake Makefile generator described
above.  Also, the ``ctest`` does not consider make dependencies when running
so the software must be completely built before running ``ctest`` as described
here.


Running all tests
-----------------

To run all of the defined tests (i.e. created using ``TRIBITS_ADD_TEST()`` or
``TRIBITS_ADD_ADVANCED_TEST()``) use::

  $ ctest -j<N>

(where ``<N>`` is an integer for the number of processes to try to run tests
in parallel).  A summary of what tests are run and their pass/fail status will
be printed to the screen.  Detailed output about each of the tests is archived
in the generate file::

  Testing/Temporary/LastTest.log

where CTest creates the ``Testing`` directory in the local directory where you
run it from.

NOTE: The ``-j<N>`` argument allows CTest to use more processes to run tests.
This will intelligently load ballance the defined tests with multiple
processes (i.e. MPI tests) and will try not exceed the number of processes
``<N>``.  However, if tests are defined that use more that ``<N>`` processes,
then CTest will still run the test but will not run any other tests while the
limit of ``<N>`` processes is exceeded.  To exclude tests that require more
than ``<N>`` processes, set the cache variable ``MPI_EXEC_MAX_NUMPROCS`` (see
`Configuring with MPI support`_).


Only running tests for a single package
---------------------------------------

Tests for just a single TriBITS package can be run with::

  $ ctest -j4 -L <TRIBITS_PACKAGE>

or::

  $ cd packages/<TRIBITS_PACKAGE>
  $ ctest -j4

This will run tests for packages and subpackages inside of the parent package
``<TRIBITS_PACKAGE>``.

NOTE: CTest has a number of ways to filter what tests get run.  You can use
the test name using ``-E``, you can exclude tests using ``-I``, and there are
other approaches as well.  See ``ctest --help`` and online documentation, and
experiment for more details.


Running a single test with full output to the console
-----------------------------------------------------

To run just a single test and send detailed output directly to the console,
one can run::

  $ ctest -R ^<FULL_TEST_NAME>$ -VV

However, when running just a single test, it is usally better to just run the
test command manually to allow passing in more options.  To see what the actual test command is, use::


  $ ctest -R ^<FULL_TEST_NAME>$ -VV -N

This will only print out the test command that ``ctest`` runs and show the
working directory.  To run the test exactly as ``ctest`` would, cd into the
shown working directory and run the shown command.


Overridding test timeouts
-------------------------

The configured glboal test timeout described in ``Setting test timeouts at
configure time`` can be overridden on the CTest command-line as::

  $ ctest --timeout <maxSeconds>

This will override the configured cache variable `DART_TESTING_TIMEOUT`_.
However, this will **not** override the test timesouts set on individual tests
on a test-by-test basis!

**WARNING:** Do not try to use ``--timeout=<maxSeconds>`` or CTest will just
ignore the argument!


Running memory checking
-----------------------

To configure for running memory testing with ``valgrind``, use::

 -D MEMORYCHECK_COMMAND=<abs-path-to-valgrind>/valgrind \
 -D MEMORYCHECK_SUPPRESSIONS_FILE=<abs-path-to-supp-file0> \
 -D MEMORYCHECK_COMMAND_OPTIONS="-q --trace-children=yes --tool=memcheck \
   --leak-check=yes --workaround-gcc296-bugs=yes \
   --num-callers=50 --suppressions=<abs-path-to-supp-file1> \
   ... --suppressions=<abs-path-to-supp-fileN>"

Above, you have to set the absolute path to the valgrind executable to run
using ``MEMORYCHECK_COMMAND`` as CMake will not find this for you by default.
To use a single valgrind suppression file, just set
``MEMORYCHECK_SUPPRESSIONS_FILE`` to the path of that suppression file as
shown above.  To add other suppression files, they have to be added as other
general valgrind arguments in ``MEMORYCHECK_COMMAND_OPTIONS`` as shown.

After configuring with the above options, to run the memory tests for all
enabled tests, from the **base** project build directory, do::

  $ ctest -T memcheck

This will run valgrind on **every** test command that is run by ctest.

To run valgrind on the tests for a single package, from the **base** project
directory, do::

  $ ctest -T memcheck -L <TRIBITS_PACKAGE>

To run valgrind on a specific test, from the **base** project directory, do::

  $ ctest -T memcheck -R ^<FULL_TEST_NAME>$

Detailed output from valgrind is printed in the file::

  Testing/Temporary/LastDynamicAnalysis_<DATE_TIME>.log

NOTE: If you try to run memory tests from any subdirectories, it will not
work.  You have to run them from the ***base** project build directory as
shown above.  A nice way to view valgrind results is to submit to CDash using
the ``dashboard`` target (see `Dashboard submissions`_).

NOTE: You have to use the valgrind option ``--trace-children=yes`` to trace
through child processes.  This is needed if you have tests that are given as
CMake -P scripts (such as advanced tests) or tests driven in bash, Perl,
Python, or other languages.


Installing
==========

After a build and test of the software is complete, the software can be
installed.  Actually, to get ready for the install, the install directory must
be specified at configure time by setting the variable
``CMAKE_INSTALL_PREFIX``.  The other commands described below can all be run
after the build and testing is complete.

**WARNING:** When using shared libraries, one must be careful to avoid the
error **"RegularExpression::compile(): Expression too big."** when using RPATH
and when RPATH gets stripped out on install.  To avoid this error, use the
shortest build directory you can, like::

  $HOME/<Project>_BUILD/

This has been shown to allow even the most complex TriBITS-based CMake
projects to successfully install and avoid this error.


Setting the install prefix at configure time
--------------------------------------------

In order to set up for the install, the install prefix should be set up at
configure time by setting, for example::

  -D CMAKE_INSTALL_PREFIX=$HOME/install/<Project>/mpi/opt

The default location for the installation of libraries, headers, and
executables is given by the variables (with defaults)::

  -D <Project>_INSTALL_INCLUDE_DIR="include" \
  -D <Project>_INSTALL_LIB_DIR="lib" \
  -D <Project>_INSTALL_RUNTIME_DIR="bin" \
  -D <Project>_INSTALL_EXAMPLE_DIR="example"

If these paths are relative (i.e. don't start with "/" and use type
``STRING``) then they are relative to ``${CMAKE_INSTALL_PREFIX}``.  Otherwise
the paths can be absolute (use type ``PATH``) and don't have to be under
``${CMAKE_INSTALL_PREFIX}``.  For example, to install each part in any
abritrary location use::

  -D <Project>_INSTALL_INCLUDE_DIR="/usr/trilinos_include" \
  -D <Project>_INSTALL_LIB_DIR="/usr/trilinos_lib" \
  -D <Project>_INSTALL_RUNTIME_DIR="/usr/trilinos_bin" \
  -D <Project>_INSTALL_EXAMPLE_DIR="/usr/share/trilinos/examples"

NOTE: The defaults for the above include paths will be set by the standard
CMake module ``GNUInstallDirs`` if ``<Project>_USE_GNUINSTALLDIRS=TRUE`` is
set.  Some projects have this set by default (see the ``CMakeCache.txt`` after
configuring to see default being used by this project).

WARNING: To overwrite default relative paths, you must use the data type
``STRING`` for the cache variables.  If you don't, then CMake will use the
current binary directory for the base path.  Otherwise, if you want to specify
absolute paths, use the data type ``PATH`` as shown above.


Avoiding installing libraries and headers
-----------------------------------------

By default, any libraries and header files defined by in the TriBITS project
<Project> will get installed into the installation directories specified by
``CMAKE_INSTALL_PREFIX``, ``<Project>_INSTALL_INCLUDE_DIR`` and
``<Project>_INSTALL_LIB_DIR``.  However, if the primary desire is to install
executables only, then the user can set::

   -D <Project>_INSTALL_LIBRARIES_AND_HEADERS=OFF

which, if in addition static libraries are being built
(i.e. ``BUILD_SHARED_LIBS=OFF``), this this option will result in no libraries
or headers being installed into the ``<install>/include/`` and
``<install>/lib/`` directories, respectively.  However, if shared libraries
are being built (i.e. ``BUILD_SHARED_LIBS=ON``), they the libraries will be
installed in ``<install>/lib/`` along with the executables because the
executables can't run without the shared libraries being installed.


Installing the software
-----------------------

To install the software, type::

  $ make install

Note that CMake actually puts in the build dependencies for installed targets
so in some cases you can just type ``make -j<N> install`` and it will also
build the software.  However, it is advanced to always build and test the
software first before installing with::

  $ make -j<N> && ctest -j<N> && make -j<N> install

This will ensure that everything is built correctly and all tests pass before
installing.

**NOTE:** If while installing you see errors stating
"RegularExpression::compile(): Expression too big.", then try using a shorter
build directory path like::

  $HOME/<Project>_BUILD/

See the above warning for more details.


Packaging
=========

Packaged source and binary distributions can also be created using CMake and
CPack.


Creating a tarball of the source tree
-------------------------------------

To create a source tarball of the project, first configure with the list of
desired packages (see `Selecting the list of packages to enable`_) and pass in
::

  -D <Project>_ENABLE_CPACK_PACKAGING=ON

To actually generate the distribution files, use::

  $ make package_source

The above command will tar up *everything* in the source tree except for files
explicitly excluded in the CMakeLists.txt files and packages that are not
enabled so make sure that you start with a totally clean source tree before
you do this.  You can clean the source tree first to remove all ignored files
using::

  $ git clean -fd -x

You can include generated files in the tarball, such as Doxygen output files,
by creating them first, then running ``make package_source`` and they will be
included in the distribution (unless there is an internal exclude set).

Disabled subpackages can be included or excluded from the tarball by setting
``<Project>_EXCLUDE_DISABLED_SUBPACKAGES_FROM_DISTRIBUTION`` (the TriBITS
project has its own default, check ``CMakeCache.txt`` to see what the default
is).  If ``<Project>_EXCLUDE_DISABLED_SUBPACKAGES_FROM_DISTRIBUTION=ON`` and
but one wants to include some subpackages that are otherwise excluded, just
enable them or their outer package so they will be included in the source
tarball.  To get a printout of set regular expresions that will be used to
match files to exclude, set::

  -D <Project>_DUMP_CPACK_SOURCE_IGNORE_FILES=ON

While a set of default CPack source generator types is defined for this
project (see the ``CMakeCache.txt`` file), it can be overridden using, for
example::

  -D <Project>_CPACK_SOURCE_GENERATOR="TGZ;TBZ2"

(see CMake documentation to find out the types of supported CPack source
generators on your system).

NOTE: When configuring from an untarred source tree that has missing packages,
one must configure with::

  -D <Project>_ASSERT_MISSING_PACKAGES=OFF

Otherwise, TriBITS will error out complaining about missing packages.  (Note
that ``<Project>_ASSERT_MISSING_PACKAGES`` will default to ```OFF``` in
release mode, i.e. ``<Project>_ENABLE_DEVELOPMENT_MODE==OFF``.)


Dashboard submissions
=====================

You can use the TriBITS scripting code to submit package-by-package build,
test, coverage, memcheck results to the project's CDash dashboard.

First, configure as normal but add the build and test parallel levels with::

  -DCTEST_BUILD_FLAGS=-j4 -DCTEST_PARALLEL_LEVEL=4

(or with some other ``-j<N>``).  Then, invoke the build, test and submit
with::

  $ make dashboard

This invokes the advanced TriBITS CTest scripts to do an experimental build
for all of the packages that you have explicitly enabled.  The packages that
are implicitly enabled due to package dependencies are not directly processed
by the experimental_build_test.cmake script.

There are a number of options that you can set in the environment to control
what this script does.  This set of options can be found by doing::

  $ grep 'SET_DEFAULT_AND_FROM_ENV(' \
      $TRIBITS_DIR/ctest_driver/TribitsCTestDriverCore.cmake

Currently, the variables can be set include::

  SET_DEFAULT_AND_FROM_ENV(<Project>_IGNORE_MISSING_EXTRA_REPOSITORIES ...)
  SET_DEFAULT_AND_FROM_ENV(<Project>_PRE_REPOSITORIES ...)
  SET_DEFAULT_AND_FROM_ENV(<Project>_EXTRA_REPOSITORIES ...)
  SET_DEFAULT_AND_FROM_ENV(CTEST_SOURCE_NAME <Project>)
  SET_DEFAULT_AND_FROM_ENV(CTEST_CONFIGURATION_UNIT_TESTING OFF)
  SET_DEFAULT_AND_FROM_ENV(CTEST_TEST_TYPE Experimental)
  SET_DEFAULT_AND_FROM_ENV(<Project>_TRACK "${<Project>_TRACK_DEFAULT}")
  SET_DEFAULT_AND_FROM_ENV(CTEST_SITE ${CTEST_SITE_DEFAULT})
  SET_DEFAULT_AND_FROM_ENV(CTEST_DASHBOARD_ROOT "")
  SET_DEFAULT_AND_FROM_ENV(BUILD_TYPE NONE)
  SET_DEFAULT_AND_FROM_ENV(<Project>_VERBOSE_CONFIGURE OFF)
  SET_DEFAULT_AND_FROM_ENV(COMPILER_VERSION UNKNOWN)
  SET_DEFAULT_AND_FROM_ENV(CTEST_BUILD_NAME
  SET_DEFAULT_AND_FROM_ENV(CTEST_START_WITH_EMPTY_BINARY_DIRECTORY TRUE)
  SET_DEFAULT_AND_FROM_ENV(CTEST_WIPE_CACHE TRUE)
  SET_DEFAULT_AND_FROM_ENV(CTEST_CMAKE_GENERATOR ${DEFAULT_GENERATOR})
  SET_DEFAULT_AND_FROM_ENV(CTEST_DO_UPDATES TRUE)
  SET_DEFAULT_AND_FROM_ENV(CTEST_GENERATE_DEPS_XML_OUTPUT_FILE FALSE)
  SET_DEFAULT_AND_FROM_ENV(CTEST_UPDATE_ARGS "")
  SET_DEFAULT_AND_FROM_ENV(CTEST_UPDATE_OPTIONS "")
  SET_DEFAULT_AND_FROM_ENV(CTEST_BUILD_FLAGS "-j2")
  SET_DEFAULT_AND_FROM_ENV(CTEST_BUILD_FLAGS "")
  SET_DEFAULT_AND_FROM_ENV(CTEST_DO_BUILD TRUE)
  SET_DEFAULT_AND_FROM_ENV(CTEST_DO_TEST TRUE)
  SET_DEFAULT_AND_FROM_ENV(MPI_EXEC_MAX_NUMPROCS 0)
  SET_DEFAULT_AND_FROM_ENV(CTEST_PARALLEL_LEVEL 1)
  SET_DEFAULT_AND_FROM_ENV(<Project>_WARNINGS_AS_ERRORS_FLAGS "")
  SET_DEFAULT_AND_FROM_ENV(CTEST_DO_COVERAGE_TESTING FALSE)
  SET_DEFAULT_AND_FROM_ENV(CTEST_COVERAGE_COMMAND gcov)
  SET_DEFAULT_AND_FROM_ENV(CTEST_DO_MEMORY_TESTING FALSE)
  SET_DEFAULT_AND_FROM_ENV(CTEST_MEMORYCHECK_COMMAND
    "${CTEST_MEMORYCHECK_COMMAND_DEFAULT}")
  SET_DEFAULT_AND_FROM_ENV(CTEST_MEMORYCHECK_COMMAND_OPTIONS "")
  SET_DEFAULT_AND_FROM_ENV(CTEST_GENERATE_OUTER_DEPS_XML_OUTPUT_FILE TRUE)
  SET_DEFAULT_AND_FROM_ENV(CTEST_SUBMIT_CDASH_SUBPROJECTS_DEPS_FILE TRUE)
  SET_DEFAULT_AND_FROM_ENV(CTEST_DO_SUBMIT TRUE)
  SET_DEFAULT_AND_FROM_ENV(<Project>_ENABLE_SECONDARY_TESTED_CODE OFF)
  SET_DEFAULT_AND_FROM_ENV(<Project>_ENABLE_SECONDARY_STABLE_CODE OFF)
  SET_DEFAULT_AND_FROM_ENV(<Project>_ADDITIONAL_PACKAGES "")
  SET_DEFAULT_AND_FROM_ENV(<Project>_EXCLUDE_PACKAGES "")
  SET_DEFAULT_AND_FROM_ENV(<Project>_BRANCH "${<Project>_BRANCH_DEFAULT}")
  SET_DEFAULT_AND_FROM_ENV(<Project>_ENABLE_DEVELOPMENT_MODE
    "${<Project>_ENABLE_DEVELOPMENT_MODE_DEFAULT}")
  SET_DEFAULT_AND_FROM_ENV(<Project>_REPOSITORY_LOCATION ...)
  SET_DEFAULT_AND_FROM_ENV(<Project>_PACKAGES "")
  SET_DEFAULT_AND_FROM_ENV(<Project>_EXTRAREPOS_FILE ...)
  SET_DEFAULT_AND_FROM_ENV(<Project>_ENABLE_KNOWN_EXTERNAL_REPOS_TYPE ...)
  SET_DEFAULT_AND_FROM_ENV(CTEST_ENABLE_MODIFIED_PACKAGES_ONLY OFF)
  SET_DEFAULT_AND_FROM_ENV(CTEST_EXPLICITLY_ENABLE_IMPLICITLY_ENABLED_PACKAGES ...)
  SET_DEFAULT_AND_FROM_ENV(<Project>_DISABLE_ENABLED_FORWARD_DEP_PACKAGES ...)
  SET_DEFAULT_AND_FROM_ENV(TRIBITS_2ND_CTEST_DROP_SITE "")
  SET_DEFAULT_AND_FROM_ENV(TRIBITS_2ND_CTEST_DROP_LOCATION "")
  SET_DEFAULT_AND_FROM_ENV(CTEST_DEPENDENCY_HANDLING_UNIT_TESTING FALSE)
  SET_DEFAULT_AND_FROM_ENV(CTEST_UPDATE_RETURN_VAL 0)
  SET_DEFAULT_AND_FROM_ENV(<Project>_SOURCE_DIRECTORY ${CTEST_SOURCE_DIRECTORY})
  SET_DEFAULT_AND_FROM_ENV(CTEST_DROP_SITE_COVERAGE ...)
  SET_DEFAULT_AND_FROM_ENV(CTEST_DROP_LOCATION_COVERAGE ...)

For details on what all of these options do, look at the file
``TribitsCTestDriverCore.cmake`` (more detailed documentation will come in
time).  The value of all of these variables is printed out when the ``make
dashboard`` target is run.  From there, one can change and tweak these
options.  To see the defaults, just run with ``CTEST_DO_SUBMIT=FALSE`` and one
can see the values used without actually doing the submit.  Just grep for the
names of the variables in the STDOUT for ``make dashboard``.

For an example of variables one might want to tweak, to run an experimental
build and in the process change the build, use::

  $ env CTEST_BUILD_NAME=MyBuild make dashboard

After this finishes running, look for the build 'MyBuild' (or whatever build
name you used above) in the <Project> CDash dashboard (the CDash URL is
printed at the end of STDOUT).

It is useful to set ``CTEST_BUILD_NAME`` to some unique name to make it easier
to find your results in the CDash dashboard.

A number of the defaults set in ``TribitsCTestDriverCore.cmake`` are
overridden from ``experimental_build_test.cmake`` (such as
``CTEST_TEST_TYPE=Experimental``) so you will want to look at
``experimental_build_test.cmake`` to see how these are changed.  The script
``experimental_build_test.cmake`` sets reasonable values for these options in
order to use the `make dashboard` target in iterative development for
experimental builds.

The target ``dashboard`` is not directly related to the built-in CMake targets
'Experimental*' that run standard dashboards with CTest without the custom
package-by-package driver in ``TribitsCTestDriverCore.cmake``.  The
package-by-package extended CTest driver is more appropriate for the
TriBITS-based project <Project>.

Once you configure with ``-D<Project>_ENABLE_COVERAGE_TESTING=ON``, the
environment variable ``CTEST_DO_COVERAGE_TESTING=TRUE`` is automatically set
by the target 'dashboard' so you don't have to set this yourself.

Doing a memory check with Valgrind requires that you set
``CTEST_DO_MEMORY_TESTING=TRUE`` with the 'env' command as::

  $ env CTEST_DO_MEMORY_TESTING=TRUE make dashboard

but also note that you may also need to set the valgrind command and options
with::

  $ env CTEST_DO_MEMORY_TESTING=TRUE \
    CTEST_MEMORYCHECK_COMMAND=<abs-path-to-valgrind> \
    CTEST_MEMORYCHECK_COMMAND_OPTIONS="-q --trace-children=yes --tool=memcheck \
     --leak-check=yes --workaround-gcc296-bugs=yes \
     --num-callers=50 --suppressions=<abs-path-to-supp-file1> \
     ... --suppressions=<abs-path-to-supp-fileN>" \
    make dashboard

The CMake cache variable ``<Project>_DASHBOARD_CTEST_ARGS`` can be set on the
cmake configure line in order to pass additional arguments to 'ctest -S' when
invoking the package-by-package CTest driver.  For example::

  -D <Project>_DASHBOARD_CTEST_ARGS="-VV"

will set verbose output with CTest.

.. _TRIBITS_2ND_CTEST_DROP_SITE:
.. _TRIBITS_2ND_CTEST_DROP_LOCATION:

Also note that one can submit results to a second CDash site as well by
setting::

  $ env TRIBITS_2ND_CTEST_DROP_SITE=<second-site> \
    TRIBITS_2ND_CTEST_DROP_LOCATION=<second-location> \
    ... \
    make dashboard

If left the same as ``CTEST_DROP_SITE`` or ``CTEST_DROP_LOCATION``, then
``TRIBITS_2ND_CTEST_DROP_SITE`` and ``TRIBITS_2ND_CTEST_DROP_LOCATION`` can be
left empty "" and the defaults will be used.  However, the user must set at
least one of these variables to non-empty in order to trigger the second
submit.  For example, to submit to an experimental CDash site on the same
machine, one would run::

  $ env TRIBITS_2ND_CTEST_DROP_LOCATION="/testing/cdash/submit.php?project=<Project>" \
    ... \
    make dashboard

and ``TRIBITS_2ND_CTEST_DROP_SITE`` would be used for ``CTEST_DROP_SITE``.
This is a common use case when upgrading to a new CDash installation or
testing new features for CDash before impacting the existing CDash site.

Note that if one kills the ``make dashboard`` target before it completes, then
one must reconfigure from scratch in order to get the build directory back
into the same state before the command was run.  This is because the
``dashboard`` target must first reconfigure the project with no enabled
packages before it does the package-by-package configure/build/test/submit
which enables each package one at a time.  After the package-by-package
configure/build/test/submit cycles are complete, then the project is
reconfigured with the original set of package enables and returning to the
original configure state.
