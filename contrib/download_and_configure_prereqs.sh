#!/bin/bash
##########
# Verify Environment
#
if [ -z $PACKAGES_DIR ]; then
  echo "No PACKAGES_DIR environment set. Exiting"
  exit 1
fi
unset MODULEPATH
source $PACKAGES_DIR/Modules/3.2.10/init/bash
module purge
module load advanced_modules gcc cmake
export ACCESS=$PACKAGES_DIR/seacas
export CC=gcc CXX=g++ FC=gfortran

##########
# Download Prereqs
#
download_list=(http://mooseframework.org/source_packages/hdf5-1.8.15.tar.gz \
http://mooseframework.org/source_packages/netcdf-4.3.3.1.tar.gz)
download_file() {
  echo "Downloading "$1"..."
  curl --insecure -c -s -L -O $1
  if [ $? -ne 0 ]; then
    echo "Error downloading file. Exiting..."
    exit 1
  fi
}
if ! [ -d $ACCESS/downloads ]; then mkdir $ACCESS/downloads; fi
cd $ACCESS/downloads
for URL in ${download_list[*]}; do
  download_file $URL
done

##########
# Extract Files
#
cd $ACCESS/TPL
for directory in `find . ! -path . -type d -maxdepth 1 | cut -d/  -f 2`; do
  for downloaded_file in `ls $ACCESS/downloads`; do
    if [ `echo $downloaded_file | tr '[:lower:]' '[:upper:]' | grep -i -c $(echo $directory | tr '[:lower:]' '[:upper:]')` -ge 1 ]; then
       echo "extract: "$ACCESS'/downloads/'$downloaded_file" to "$directory"..."
       tar -xf $ACCESS'/downloads/'$downloaded_file -C $directory/
       if [ $? -ne 0 ]; then
         echo "Error extracing file: "$downloaded_file
       fi
    fi
  done
done

##########
# Build HDF5
#
echo "Build HDF5"
cd $ACCESS/TPL/hdf5/hdf5-*
./configure --prefix=${ACCESS} --libdir=${ACCESS}/lib --enable-shared --enable-production --enable-debug=no --enable-static-exec
if [ $? -ne 0 ]; then
  echo "Error during HDF5 configure"
  exit 1
fi
make -j $MOOSE_JOBS
if [ $? -ne 0 ]; then
  echo "Error during HDF5 make"
  exit 1
fi
make install
if [ $? -ne 0 ]; then
  echo "Error during HDF5 install"
  exit 1
fi

##########
# Build NETCDF
#
echo "Build NETCDF"
cd $ACCESS/TPL/netcdf
perl -pi -e 's/#define NC_MAX_DIMS\s*\d*/#define NC_MAX_DIMS     65536/g' netcdf-*/include/netcdf.h
perl -pi -e 's/#define NC_MAX_VARS\s*\d*/#define NC_MAX_VARS     524288/g' netcdf-*/include/netcdf.h
perl -pi -e 's/#define NC_MAX_VAR_DIMS\s*\d*/#define NC_MAX_VAR_DIMS     8/g' netcdf-*/include/netcdf.h
cd netcdf-*/
CFLAGS="-I$ACCESS/include"; export CFLAGS
LDFLAGS="-L$ACCESS/lib "; export LDFLAGS
./configure --enable-netcdf-4  --enable-shared \
  --disable-fsync --prefix ${ACCESS} \
  --libdir=${ACCESS}/lib \
  --disable-dap --disable-cdmremote
if [ $? -ne 0 ]; then
  echo "Error during configure. Exiting"
  exit 1
fi
make -j $MOOSE_JOBS
if [ $? -ne 0 ]; then
  echo "Error during make. Exiting"
  exit 1
fi
make install
if [ $? -ne 0 ]; then
  echo "Error during make install. Exiting"
  exit 1
fi

##########
# Build SEACAS
#
mkdir $ACCESS/build
cd $ACCESS/build
cmake  \
-D CMAKE_BUILD_TYPE=Release \
-D SEACASProj_ENABLE_ALL_PACKAGES:BOOL=ON \
-D SEACASProj_ENABLE_ALL_OPTIONAL_PACKAGES:BOOL=ON \
-D SEACASProj_ENABLE_SECONDARY_TESTED_CODE:BOOL=ON \
-D SEACASProj_ENABLE_TESTS=ON \
-D CMAKE_INSTALL_PREFIX:PATH=$ACCESS \
-D CMAKE_INSTALL_LIBDIR:PATH=$ACCESS/lib \
-D BUILD_SHARED_LIBS:BOOL=OFF \
-D CMAKE_CXX_COMPILER:FILEPATH=g++ \
-D CMAKE_C_COMPILER:FILEPATH=gcc \
-D CMAKE_Fortran_COMPILER:FILEPATH=gfortran \
-D SEACASProj_SKIP_FORTRANCINTERFACE_VERIFY_TEST:BOOL=ON \
-D TPL_ENABLE_HDF5:BOOL=ON \
-D TPL_ENABLE_Matio:BOOL=OFF \
-D TPL_ENABLE_Netcdf:BOOL=ON \
-D TPL_ENABLE_MPI=OFF \
-D TPL_ENABLE_Pamgen=OFF \
-D TPL_ENABLE_CGNS:BOOL=OFF \
-D NetCDF_DIR:PATH=$ACCESS \
-D HDF5_ROOT:PATH=$ACCESS \
-D HDF5_NO_SYSTEM_PATHS=ON \
-D TPL_ENABLE_X11=OFF \
$EXTRA_ARGS \
..

if [ $? -ne 0 ]; then
  echo "Error during SEACAS configure. Exiting..."
  exit 1
fi
make -j $MOOSE_JOBS
if [ $? -ne 0 ]; then
  echo "Error during SEACAS make. Exiting..."
  exit 1
fi
make install
if [ $? -ne 0 ]; then
  echo "Error during SEACAS install. Exiting..."
  exit 1
fi

##########
# Build Exodus in shared mode. We have to do this
# separately, as some other tools seem to fail
# to build in shared mode (blot for example)
#
mkdir $ACCESS/build-exodus
cd $ACCESS/build-exodus
cmake  \
-D CMAKE_BUILD_TYPE=Release \
-D SEACASProj_ENABLE_SEACASExodus=ON \
-D SEACASProj_ENABLE_TESTS=ON \
-D CMAKE_INSTALL_PREFIX:PATH=$ACCESS \
-D BUILD_SHARED_LIBS:BOOL=ON \
-D CMAKE_C_COMPILER:FILEPATH=gcc \
-D TPL_ENABLE_HDF5:BOOL=ON \
-D TPL_ENABLE_Netcdf:BOOL=ON \
-D TPL_ENABLE_MPI=OFF \
-D NetCDF_DIR:PATH=$ACCESS \
-D HDF5_ROOT:PATH=$ACCESS \
-D HDF5_NO_SYSTEM_PATHS=ON \
-D SEACASProj_SKIP_FORTRANCINTERFACE_VERIFY_TEST:BOOL=ON \
-D TPL_ENABLE_X11=OFF \
..

if [ $? -ne 0 ]; then
  echo "Error during Exodus configure. Exiting..."
  exit 1
fi
make -j $MOOSE_JOBS
if [ $? -ne 0 ]; then
  echo "Error during Exodus make. Exiting..."
  exit 1
fi
make install
if [ $? -ne 0 ]; then
  echo "Error during Exodus install. Exiting..."
  exit 1
fi

##########
# Clean up
#
echo -e "I am about to delete the source for building SEACAS. ctrl-c now, or panic later..."
sleep 10
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
