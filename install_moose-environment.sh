#! /usr/bin/env bash
###### Darwin Installer ######
#
# Comment any of the following to prevent that library from being built
#
# HDF5: a high-performance software library and
# data format that has been adopted across multiple industries and has become a
# de facto standard in scientific and research communities.
#
# More information about The HDF Group, the HDF5 Community and the HDF5 software
# project, tools and services can be found at the Group's website.
#
#    https://www.hdfgroup.org/
HDF5=true
#
#
# netCDF: The Unidata network Common Data Form (netCDF) is an interface for
# scientific data access and a freely-distributed software library that
# provides an implementation of the interface.
#
# For more information about netCDF, see the netCDF Web page at
#
#    http://www.unidata.ucar.edu/netcdf/
NETCDF=true
#
#
# SEACAS Tools
SEACAS=true
#
#
# Build quietly
VERBOSE=${VERBOSE:-1}
###### End user config  ######

# register ACCESS directory with executor script location
DOWNLOAD_TOOLS=(wget curl)

download_args() {
    case $1 in
        wget) echo '--quiet --no-check-certificate'; return;;
        curl) echo '--silent --insecure --location --remote-name'; return;;
        *) echo 1>&2 Invalid download tool; exit 1
    esac
}

download_file() {
    if [ -f $(basename $1) ]; then rm -f $(basename $1); fi
    for tool in ${DOWNLOAD_TOOLS[*]}; do
        which $tool 2>&1 > /dev/null
        if [[ $? == 0 ]]; then
            echo "Downloading $(basename $1)..."
            `$(which $tool) $(download_args $tool) $1`
            if [[ $? == 0 ]] && [ `file $(basename $1) | grep -c -i "text\|UTF-8\|HTML"` -eq 0 ]; then
                echo "done"
            else
                rm -f $(basename $1)
                echo 1>&2 Download failed
                exit 1
            fi
            return
        fi
    done
    echo "No available downloading tool (wget, curl, etc). Exiting..."
    exit 1
}

# =================== Environment Setup ==========
if [ `uname -s` == 'Darwin' ]; then
  export JOBS=`/usr/sbin/sysctl -n hw.ncpu`
else
  export JOBS=`cat /proc/cpuinfo | grep processor | wc -l`
fi
unset MODULEPATH
source $PACKAGES_DIR/Modules/3.2.10/init/bash
module purge
module load advanced_modules autotools gcc cmake
export ACCESS="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
export CC=gcc CXX=g++ FC=gfortran
CFLAGS="-I$ACCESS/include"; export CFLAGS
LDFLAGS="-L$ACCESS/lib "; export LDFLAGS

# =================== INSTALL HDF5 ===============
if [ -n "$HDF5" ]; then
    echo Installing HDF5
    hdf_version="1.10.2"
    url=https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.10/hdf5-${hdf_version}/src/hdf5-${hdf_version}.tar.bz2
    cd "$ACCESS/TPL/hdf5"
    download_file $url
    rm -rf hdf5-${hdf_version}
    tar -jxf $(basename $url)
    cd hdf5-${hdf_version}
    ./configure --prefix=${ACCESS} \
                --libdir=${ACCESS}/lib \
                --enable-shared \
                --enable-build-mode=production \
                --enable-static-exec
    if [ $? -ne 0 ]; then
        echo "Error during HDF5 configure"
        exit 1
    fi
    make -j${JOBS} && make "V=${VERBOSE}" install
    if [ $? != 0 ]
    then
        echo 1>&2 couldn\'t make hdf5. exiting.
        exit 1
    fi
fi

# =================== INSTALL NETCDF ===============
if [ -n "$NETCDF" ]; then
    echo Installing netCDF
    netcdf_version="4.6.1"
    url=https://github.com/Unidata/netcdf-c/archive/v${netcdf_version}.tar.gz
    cd "$ACCESS/TPL/netcdf"
    download_file $url
    tar -xf $(basename $url)
    perl -pi -e 's/#define NC_MAX_DIMS\s*\d*/#define NC_MAX_DIMS     65536/g' netcdf-c-${netcdf_version}/include/netcdf.h
    perl -pi -e 's/#define NC_MAX_VARS\s*\d*/#define NC_MAX_VARS     524288/g' netcdf-c-${netcdf_version}/include/netcdf.h
    perl -pi -e 's/#define NC_MAX_VAR_DIMS\s*\d*/#define NC_MAX_VAR_DIMS     8/g' netcdf-c-${netcdf_version}/include/netcdf.h
    mkdir build; cd build
    ../netcdf-c-${netcdf_version}/configure --enable-netcdf-4  \
                --enable-shared \
                --disable-fsync --prefix ${ACCESS} \
                --libdir=${ACCESS}/lib \
                --disable-dap --disable-cdmremote

    if [ $? != 0 ]; then
        echo 1>&2 couldn\'t configure netcdf. exiting.
        exit 1
    fi
    make -j $JOBS && make "VERBOSE=${VERBOSE}" install
    if [ $? != 0 ]; then
        echo 1>&2 couldn\'t configure make netcdf. exiting.
        exit 1
    fi
fi

# =================== INSTALL SEACAS ===============
if [ -n "$SEACAS" ]; then
    echo "Step 1. Build SEACAS in static mode"
    if [ -d "$ACCESS/build" ]; then rm -rf "$ACCESS/build"; fi
    mkdir "$ACCESS/build"; cd "$ACCESS/build"
    cmake  \
        -D CMAKE_BUILD_TYPE=Release \
        -D SEACASProj_ENABLE_ALL_PACKAGES:BOOL=ON \
        -D SEACASProj_ENABLE_ALL_OPTIONAL_PACKAGES:BOOL=ON \
        -D SEACASProj_ENABLE_SECONDARY_TESTED_CODE:BOOL=ON \
        -D SEACASProj_ENABLE_TESTS=ON \
        -D CMAKE_INSTALL_PREFIX:PATH="$ACCESS" \
        -D CMAKE_INSTALL_LIBDIR:PATH="$ACCESS/lib" \
        -D BUILD_SHARED_LIBS:BOOL=OFF \
        -D CMAKE_CXX_COMPILER:FILEPATH=g++ \
        -D CMAKE_C_COMPILER:FILEPATH=gcc \
        -D CMAKE_Fortran_COMPILER:FILEPATH=gfortran \
        -D SEACASProj_SKIP_FORTRANCINTERFACE_VERIFY_TEST:BOOL=ON \
        -D SEACASProj_ENABLE_Kokkos:BOOL=OFF \
        -D TPL_ENABLE_HDF5:BOOL=ON \
        -D TPL_ENABLE_Matio:BOOL=OFF \
        -D TPL_ENABLE_Netcdf:BOOL=ON \
        -D TPL_ENABLE_MPI=OFF \
        -D TPL_ENABLE_Pamgen=OFF \
        -D TPL_ENABLE_CGNS:BOOL=OFF \
        -D NetCDF_DIR:PATH="$ACCESS" \
        -D HDF5_ROOT:PATH="$ACCESS" \
        -D HDF5_NO_SYSTEM_PATHS=ON \
        -D TPL_ENABLE_X11=ON \
        "$EXTRA_ARGS" \
        ..
    if [[ $? != 0 ]]; then
        echo 1>&2 "error configuring seacas"
        exit 1
    fi
    make -j $JOBS && make "VERBOSE=${VERBOSE}" install
    if [[ $? != 0 ]]; then
        echo 1>&2 "error building seacas"
        exit 1
    fi
    echo "Step 2. Build SEACAS in Shared mode"
    if [ -d "$ACCESS/build-exodus" ]; then rm -rf "$ACCESS/build-exodus"; fi
    mkdir "$ACCESS/build-exodus"; cd "$ACCESS/build-exodus"
    cmake  \
        -D CMAKE_BUILD_TYPE=Release \
        -D SEACASProj_ENABLE_SEACASExodus=ON \
        -D SEACASProj_ENABLE_TESTS=ON \
        -D CMAKE_INSTALL_PREFIX:PATH="$ACCESS" \
        -D BUILD_SHARED_LIBS:BOOL=ON \
        -D CMAKE_C_COMPILER:FILEPATH=gcc \
        -D TPL_ENABLE_HDF5:BOOL=ON \
        -D TPL_ENABLE_Netcdf:BOOL=ON \
        -D TPL_ENABLE_MPI=OFF \
        -D SEACASProj_ENABLE_Kokkos:BOOL=OFF \
        -D NetCDF_DIR:PATH="$ACCESS" \
        -D HDF5_ROOT:PATH="$ACCESS" \
        -D HDF5_NO_SYSTEM_PATHS=ON \
        -D SEACASProj_SKIP_FORTRANCINTERFACE_VERIFY_TEST:BOOL=ON \
        -D TPL_ENABLE_X11=ON \
        ..
    if [[ $? != 0 ]]; then
        echo 1>&2 "error configuring seacas"
        exit 1
    fi
    make -j $JOBS && make "VERBOSE=${VERBOSE}" install
    if [[ $? != 0 ]]; then
        echo 1>&2 "error building seacas"
        exit 1
    fi
fi
=================== CLEAN UP =====================
cd $ACCESS
good_list=(bin lib include share LICENSE . ..)
for item in `ls -a`; do
  DELETE='TRUE'
  for good_item in ${good_list[*]}; do
    if [ $item == $good_item ]; then
      DELETE="FALSE"
      break
    fi
  done
  if [ $DELETE == "TRUE" ]; then
    echo "deleting: " $item
    rm -rf $item
  fi
done
