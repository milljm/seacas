#! /usr/bin/env bash

# Which compiler to use?
export COMPILER=${COMPILER:-gnu}

#By default, download and then install.
DOWNLOAD=${DOWNLOAD:-YES}
BUILD=${BUILD:-YES}

# Force downloading and installation even if the TPL already exists in lib/include
FORCE=${FORCE:-NO}

# Shared libraries or static libraries?
SHARED=${SHARED:-YES}

# Enable Burst-Buffer support in PNetCDF?
BB=${BB:-NO}

# Which TPLS? (HDF5 and NetCDF always, PNetCDF if MPI=ON)
CGNS=${CGNS:-ON}
MATIO=${MATIO:-ON}
GNU_PARALLEL=${GNU_PARALLEL:-ON}
NEEDS_ZLIB=${NEEDS_ZLIB:-NO}
H5VERSION=${H5VERSION:-V110}

SUDO=${SUDO:-}
JOBS=${JOBS:-2}
VERBOSE=${VERBOSE:-1}

pwd
export ACCESS=`pwd`

# Text color variables
if [[ $TERM != *"xterm"* ]]; then
    export TERM=dumb
fi
txtred=$(tput setaf 1)    # Red
txtgrn=$(tput setaf 2)    # Green
txtylw=$(tput setaf 3)    # Yellow
txtblu=$(tput setaf 4)    # Blue
txtpur=$(tput setaf 5)    # Purple
txtcyn=$(tput setaf 6)    # Cyan
txtwht=$(tput setaf 7)    # White
txtrst=$(tput sgr0)       # Text reset

if [ "$MPI" == "ON" ] && [ "$CRAY" == "ON" ]
then
    CC=cc; export CC
    CFLAGS=-static; export CFLAGS
    SHARED=NO
elif [ "$MPI" == "ON" ]
then
    CC=mpicc; export CC
fi

OS=`uname -s`
if [ "$SHARED" == "YES" ]
then
    if [ "$OS" == "Darwin" ] ; then
	LD_EXT="dylib"
    else
	LD_EXT="so"
    fi
else
    LD_EXT="a"
fi

if [ $# -gt 0 ]; then
    if [ "$1" == "--help" ]; then
	echo "${txtcyn}Environment Variables used in the script and their default values:"
	echo ""
	echo "   ACCESS       = ${txtgrn}${ACCESS}${txtcyn} (Automatically set to current directory)"
	echo "   OS           = ${txtgrn}${OS}${txtcyn} (Automatically set)"
	echo "   COMPILER     = ${COMPILER}"
	echo ""
	echo "   DOWNLOAD     = ${DOWNLOAD}"
	echo "   BUILD        = ${BUILD}"
	echo "   FORCE        = ${FORCE}"
	echo "   SHARED       = ${SHARED}"
	ecoh "   BB           = ${BB}"
	echo ""
	echo "   H5VERSION    = ${H5VERSION}"
	echo "   CGNS         = ${CGNS}"
	echo "   MATIO        = ${MATIO}"
	echo "   GNU_PARALLEL = ${GNU_PARALLEL}"
	echo "   NEEDS_ZLIB   = ${NEEDS_ZLIB}"
	echo ""
	echo "   SUDO         = ${SUDO}"
	echo "   JOBS         = ${JOBS}"
	echo "   VERBOSE      = ${VERBOSE}"
	echo "${txtrst}"
	exit 0
    fi
fi

if [ "$NEEDS_ZLIB" == "YES" ]
then
    if [ "$FORCE" == "YES" ] || ! [ -e $ACCESS/lib/libz.${LD_EXT} ]
    then
	echo "${txtgrn}+++ ZLIB${txtrst}"
        zlib_version="1.2.11"
					 
	cd $ACCESS
	cd TPL
	if [ "$DOWNLOAD" == "YES" ]
	then
	    echo "${txtgrn}+++ Downloading...${txtrst}"
            rm -rf zlib-${zlib_version}
            rm -rf zlib-${zlib_version}.tar.gz
            curl --insecure -L -O https://zlib.net/zlib-${zlib_version}.tar.gz
            tar -xzf zlib-${zlib_version}.tar.gz
            rm -rf zlib-${zlib_version}.tar.gz
	fi
	
	if [ "$BUILD" == "YES" ]
	then
	    echo "${txtgrn}+++ Configuring, Building, and Installing...${txtrst}"
            cd zlib-${zlib_version}
	    if [[ "$SHARED" == "ON" || "$SHARED" == "YES" ]]
	    then
		USE_SHARED=""
	    else
		USE_SHARED="--static"
	    fi
            ./configure --prefix=${ACCESS}
            if [[ $? != 0 ]]
            then
		echo 1>&2 ${txtred}couldn\'t configure zlib. exiting.${txtrst}
		exit 1
            fi
            make -j${JOBS} && ${SUDO} make install
            if [[ $? != 0 ]]
            then
		echo 1>&2 ${txtred}couldn\'t build zlib. exiting.${txtrst}
		exit 1
            fi
	fi
    else
	echo "${txtylw}+++ ZLIB already installed.  Skipping download and installation.${txtrst}"
    fi
fi

# =================== BUILD HDF5 ===============
if [ "$FORCE" == "YES" ] || ! [ -e $ACCESS/lib/libhdf5.${LD_EXT} ]
then
    echo "${txtgrn}+++ HDF5${txtrst}"
    if [ "${H5VERSION}" == "V18" ]
    then
	hdf_version="1.8.20"
    else
	hdf_version="1.10.3"
    fi

    cd $ACCESS
    cd TPL/hdf5
    if [ "$DOWNLOAD" == "YES" ]
    then
	echo "${txtgrn}+++ Downloading...${txtrst}"
        rm -rf hdf5-${hdf_version}
        rm -f hdf5-${hdf_version}.tar.bz2
	if [ "${H5VERSION}" == "V18" ]
	then
	    curl --insecure -L -O https://support.hdfgroup.org/ftp/HDF5/current18/src/hdf5-${hdf_version}.tar.bz2
	else
	    curl --insecure -L -O https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.10/hdf5-${hdf_version}/src/hdf5-${hdf_version}.tar.bz2
	fi
        tar -jxf hdf5-${hdf_version}.tar.bz2
        rm -f hdf5-${hdf_version}.tar.bz2
    fi

    if [ "$BUILD" == "YES" ]
    then
	echo "${txtgrn}+++ Configuring, Building, and Installing...${txtrst}"
        cd hdf5-${hdf_version}
        H5VERSION=${H5VERSION} SHARED=${SHARED} NEEDS_ZLIB=${NEEDS_ZLIB} MPI=${MPI} bash ../runconfigure.sh
        if [[ $? != 0 ]]
        then
            echo 1>&2 ${txtred}couldn\'t configure hdf5. exiting.${txtrst}
            exit 1
        fi
        make -j${JOBS} && ${SUDO} make "V=${VERBOSE}" install
        if [[ $? != 0 ]]
        then
            echo 1>&2 ${txtred}couldn\'t build hdf5. exiting.${txtrst}
            exit 1
        fi
    fi
else
    echo "${txtylw}+++ HDF5 already installed.  Skipping download and installation.${txtrst}"
fi
# =================== INSTALL PNETCDF if parallel build ===============
if [ "$MPI" == "ON" ]
then
    # PnetCDF currently only builds static library...
    if [ "$FORCE" == "YES" ] || ! [ -e $ACCESS/lib/libpnetcdf.a ]
    then
        echo "${txtgrn}+++ PNetCDF${txtrst}"
        pnet_version="1.10.0"

        cd $ACCESS
        cd TPL/pnetcdf
        if [ "$DOWNLOAD" == "YES" ]
        then
	    echo "${txtgrn}+++ Downloading...${txtrst}"
            rm -rf parallel-netcdf-${pnet_version}
            rm -f parallel-netcdf-${pnet_version}.tar.gz
            curl --insecure -L -O http://cucis.ece.northwestern.edu/projects/PnetCDF/Release/parallel-netcdf-${pnet_version}.tar.gz
            tar -xzf parallel-netcdf-${pnet_version}.tar.gz
            rm -f parallel-netcdf-${pnet_version}.tar.gz
        fi

        if [ "$BUILD" == "YES" ]
        then
	    echo "${txtgrn}+++ Configuring, Building, and Installing...${txtrst}"
            cd parallel-netcdf-${pnet_version}
            BB=${BB} SHARED=${SHARED} bash ../runconfigure.sh
            if [[ $? != 0 ]]
            then
                echo 1>&2 ${txtred}couldn\'t configure PnetCDF. exiting.${txtrst}
                exit 1
            fi


            if [ "$CRAY" == "ON" ]
            then
                make -j${JOBS} LDFLAGS=-all-static && ${SUDO} make install
            else
                make -j${JOBS} && ${SUDO} make install
            fi
            if [[ $? != 0 ]]
            then
                echo 1>&2 ${txtred}couldn\'t build PnetCDF. exiting.${txtrst}
                exit 1
            fi
        fi
    else
	echo "${txtylw}+++ PNetCDF already installed.  Skipping download and installation.${txtrst}"
    fi
fi

# =================== INSTALL NETCDF ===============
if [ "$FORCE" == "YES" ] || ! [ -e $ACCESS/lib/libnetcdf.${LD_EXT} ]
then
    echo "${txtgrn}+++ NetCDF${txtrst}"
    cd $ACCESS
    cd TPL/netcdf
    if [ "$DOWNLOAD" == "YES" ]
    then
	echo "${txtgrn}+++ Downloading...${txtrst}"
        rm -rf netcdf-c
        git clone https://github.com/Unidata/netcdf-c netcdf-c
    fi

    if [ "$BUILD" == "YES" ]
    then
	echo "${txtgrn}+++ Configuring, Building, and Installing...${txtrst}"
        cd netcdf-c
        if [ -d build ]
        then
            rm -rf build
        fi
        mkdir build
        cd build
        SHARED=${SHARED} NEEDS_ZLIB=${NEEDS_ZLIB} MPI=${MPI} bash ../../runcmake.sh
        if [[ $? != 0 ]]
        then
            echo 1>&2 ${txtred}couldn\'t configure cmake for NetCDF. exiting.${txtrst}
            exit 1
        fi

        make -j${JOBS} && ${SUDO} make "VERBOSE=${VERBOSE}" install
        if [[ $? != 0 ]]
        then
            echo 1>&2 ${txtred}couldn\'t build NetCDF. exiting.${txtrst}
            exit 1
        fi
    fi
else
    echo "${txtylw}+++ NetCDF already installed.  Skipping download and installation.${txtrst}"
fi
# =================== INSTALL CGNS ===============
if [ "$CGNS" == "ON" ]
then
    if [ "$FORCE" == "YES" ] || ! [ -e $ACCESS/lib/libcgns.${LD_EXT} ]
    then
        echo "${txtgrn}+++ CGNS${txtrst}"
        cd $ACCESS
        cd TPL/cgns
        if [ "$DOWNLOAD" == "YES" ]
        then
	    echo "${txtgrn}+++ Downloading...${txtrst}"
            rm -rf CGNS
            git clone https://github.com/cgns/CGNS
            cd CGNS; git am ../CGNS-parallel.patch; cd ..
        fi

        if [ "$BUILD" == "YES" ]
        then
	    echo "${txtgrn}+++ Configuring, Building, and Installing...${txtrst}"
            cd CGNS
            if ! [ -d build ]
            then
                mkdir build
            fi
            cd build
            SHARED=${SHARED} NEEDS_ZLIB=${NEEDS_ZLIB} MPI=${MPI} bash ../../runcmake.sh
            if [[ $? != 0 ]]
            then
                echo 1>&2 ${txtred}couldn\'t configure CGNS. exiting.${txtrst}
                exit 1
            fi

            make -j${JOBS} && ${SUDO} make install
            if [[ $? != 0 ]]
            then
                echo 1>&2 ${txtred}couldn\'t build CGNS. exiting.${txtrst}
                exit 1
            fi
        fi
    else
	echo "${txtylw}+++ CGNS already installed.  Skipping download and installation.${txtrst}"
    fi
fi

# =================== INSTALL MATIO  ===============
if [ "$MATIO" == "ON" ]
then
    if [ "$FORCE" == "YES" ] || ! [ -e $ACCESS/lib/libmatio.${LD_EXT} ]
    then
	echo "${txtgrn}+++ MatIO${txtrst}"
	cd $ACCESS
	cd TPL/matio
	if [ "$DOWNLOAD" == "YES" ]
	then
	    echo "${txtgrn}+++ Downloading...${txtrst}"
            rm -rf matio
            git clone https://github.com/tbeu/matio.git
	fi
	
	if [ "$BUILD" == "YES" ]
	then
	    echo "${txtgrn}+++ Configuring, Building, and Installing...${txtrst}"
            cd matio
            ./autogen.sh
            SHARED=${SHARED} bash ../runconfigure.sh
            if [[ $? != 0 ]]
            then
                echo 1>&2 ${txtred}couldn\'t configure MatIO. exiting.${txtrst}
                exit 1
            fi
            make -j${JOBS} && ${SUDO} make install
            if [[ $? != 0 ]]
            then
                echo 1>&2 ${txtred}couldn\'t build MatIO. exiting.${txtrst}
                exit 1
            fi
	fi
    else
	echo "${txtylw}+++ MatIO already installed.  Skipping download and installation.${txtrst}"
    fi
fi

# =================== INSTALL PARALLEL  ===============
if [ "$GNU_PARALLEL" == "ON" ]
then
    if [ "$FORCE" == "YES" ] || ! [ -e $ACCESS/bin/env_parallel ]
    then
	echo "${txtgrn}+++ GNU Parallel${txtrst}"
        cd $ACCESS
        cd TPL/parallel
        if [ "$DOWNLOAD" == "YES" ]
        then
	    echo "${txtgrn}+++ Downloading...${txtrst}"
            rm -rf parallel-*
            curl --insecure -L -O http://ftp.gnu.org/gnu/parallel/parallel-latest.tar.bz2
            tar -jxf parallel-latest.tar.bz2
            rm -rf parallel-latest.tar.bz2
        fi

        if [ "$BUILD" == "YES" ]
        then
	    echo "${txtgrn}+++ Configuring, Building, and Installing...${txtrst}"
            cd parallel-*
            bash ../runconfigure.sh
            if [[ $? != 0 ]]
            then
                echo 1>&2 ${txtred}couldn\'t configure parallel. exiting.${txtrst}
                exit 1
            fi
            make -j${JOBS} && ${SUDO} make install
            if [[ $? != 0 ]]
            then
                echo 1>&2 ${txtred}couldn\'t build parallel. exiting.${txtrst}
                exit 1
            fi
        fi
    else
	echo "${txtylw}+++ Parallel already installed.  Skipping download and installation.${txtrst}"
    fi
fi

# ==================================
cd $ACCESS
#ls -l include
#ls -l bin
#ls -l lib
