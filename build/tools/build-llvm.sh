#!/bin/sh
#
# Copyright (C) 2012 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#  This shell script is used to rebuild the llvm and clang binaries
#  for the Android NDK.
#

# include common function and variable definitions
. `dirname $0`/prebuilt-common.sh

PROGRAM_PARAMETERS="<src-dir> <ndk-dir> <toolchain>"

PROGRAM_DESCRIPTION=\
"Rebuild the LLVM prebuilt binaries for the Android NDK.

Where <src-dir> is the location of toolchain sources, <ndk-dir> is
the top-level NDK installation path and <toolchain> is the name of
the toolchain to use (e.g. llvm-3.1)."

RELEASE=`date +%Y%m%d`
BUILD_OUT=/tmp/ndk-$USER/build/toolchain
OPTION_BUILD_OUT=
register_var_option "--build-out=<path>" OPTION_BUILD_OUT "Set temporary build directory"

# Note: platform API level 9 or higher is needed for proper C++ support
PLATFORM=$DEFAULT_PLATFORM
register_var_option "--platform=<name>"  PLATFORM "Specify platform name"

GMP_VERSION=$DEFAULT_GMP_VERSION
register_var_option "--gmp-version=<version>" GMP_VERSION "Specify gmp version"

PACKAGE_DIR=
register_var_option "--package-dir=<path>" PACKAGE_DIR "Create archive tarball in specific directory"

POLLY=no
do_polly_option () { POLLY=yes; }
register_option "--with-polly" do_polly_option "Enable Polyhedral optimizations for LLVM"

CHECK=no
do_check_option () { CHECK=yes; }
register_option "--check" do_check_option "Check LLVM"

register_jobs_option
register_mingw_option
register_try64_option

extract_parameters "$@"

prepare_mingw_toolchain /tmp/ndk-$USER/build

fix_option BUILD_OUT "$OPTION_BUILD_OUT" "build directory"
setup_default_log_file $BUILD_OUT/config.log

set_parameters ()
{
    SRC_DIR="$1"
    NDK_DIR="$2"
    TOOLCHAIN="$3"

    # Check source directory
    #
    if [ -z "$SRC_DIR" ] ; then
        echo "ERROR: Missing source directory parameter. See --help for details."
        exit 1
    fi

    if [ ! -d "$SRC_DIR/$TOOLCHAIN/llvm" ] ; then
        echo "ERROR: Source directory does not contain llvm sources: $SRC_DIR/$TOOLCHAIN/llvm"
        exit 1
    fi

    if [ -e "$SRC_DIR/$TOOLCHAIN/llvm/tools/polly" -a ! -h "$SRC_DIR/$TOOLCHAIN/llvm/tools/polly" ] ; then
        echo "ERROR: polly, if exist, needs to be a symbolic link: $SRC_DIR/$TOOLCHAIN/llvm/tools/polly"
        exit 1
    fi

    GMP_SOURCE=$SRC_DIR/gmp/gmp-$GMP_VERSION.tar.bz2
    if [ ! -f "$GMP_SOURCE" ] ; then
        echo "ERROR: Source directory does not contain gmp: $GMP_SOURCE"
        exit 1
    fi

    log "Using source directory: $SRC_DIR"

    # Check NDK installation directory
    #
    if [ -z "$NDK_DIR" ] ; then
        echo "ERROR: Missing NDK directory parameter. See --help for details."
        exit 1
    fi

    if [ ! -d "$NDK_DIR" ] ; then
        mkdir -p $NDK_DIR
        if [ $? != 0 ] ; then
            echo "ERROR: Could not create target NDK installation path: $NDK_DIR"
            exit 1
        fi
    fi

    log "Using NDK directory: $NDK_DIR"

    # Check toolchain name
    #
    if [ -z "$TOOLCHAIN" ] ; then
        echo "ERROR: Missing toolchain name parameter. See --help for details."
        exit 1
    fi
}

set_parameters $PARAMETERS

prepare_target_build

if [ "$PACKAGE_DIR" ]; then
    mkdir -p "$PACKAGE_DIR"
    fail_panic "Could not create package directory: $PACKAGE_DIR"
fi

set_toolchain_ndk $NDK_DIR $TOOLCHAIN

dump "Using C compiler: $CC"
dump "Using C++ compiler: $CXX"

rm -rf $BUILD_OUT
mkdir -p $BUILD_OUT

TOOLCHAIN_BUILD_PREFIX=$BUILD_OUT/prefix

CFLAGS="$HOST_CFLAGS $CFLAGS -I$TOOLCHAIN_BUILD_PREFIX/include"
CXXFLAGS="$CXXFLAGS -I$TOOLCHAIN_BUILD_PREFIX/include"  # polly doesn't look at CFLAGS !
LDFLAGS="$LDFLAGS -L$TOOLCHAIN_BUILD_PREFIX/lib"
export CC CXX CFLAGS CXXFLAGS LDFLAGS REQUIRES_RTTI=1

EXTRA_CONFIG_FLAGS=
rm -rf $SRC_DIR/$TOOLCHAIN/llvm/tools/polly
if [ "$POLLY" = "yes" ]; then
    # crate symbolic link
    ln -s ../../polly $SRC_DIR/$TOOLCHAIN/llvm/tools

    # build polly dependencies
    unpack_archive "$GMP_SOURCE" "$BUILD_OUT"
    fail_panic "Couldn't unpack $SRC_DIR/gmp/gmp-$GMP_VERSION to $BUILD_OUT"

    GMP_BUILD_OUT=$BUILD_OUT/gmp-$GMP_VERSION
    cd $GMP_BUILD_OUT
    fail_panic "Couldn't cd into gmp build path: $GMP_BUILD_OUT"

    OLD_ABI="${ABI}"
    export ABI=$HOST_GMP_ABI  # needed to build 32-bit on 64-bit host
    run ./configure \
        --prefix=$TOOLCHAIN_BUILD_PREFIX \
        --host=$ABI_CONFIGURE_HOST \
        --build=$ABI_CONFIGURE_BUILD \
        --disable-shared \
        --disable-nls \
        --enable-cxx
    fail_panic "Couldn't configure gmp"
    run make -j$NUM_JOBS
    fail_panic "Couldn't compile gmp"
    run make install
    fail_panic "Couldn't install gmp to $TOOLCHAIN_BUILD_PREFIX"
    ABI="$OLD_ABI"

    CLOOG_BUILD_OUT=$BUILD_OUT/cloog
    mkdir -p $CLOOG_BUILD_OUT && cd $CLOOG_BUILD_OUT
    fail_panic "Couldn't create cloog build path: $CLOOG_BUILD_OUT"

    run $SRC_DIR/$TOOLCHAIN/llvm/tools/polly/utils/cloog_src/configure \
        --prefix=$TOOLCHAIN_BUILD_PREFIX \
        --host=$ABI_CONFIGURE_HOST \
        --build=$ABI_CONFIGURE_BUILD \
        --with-gmp-prefix=$TOOLCHAIN_BUILD_PREFIX \
        --disable-shared \
        --disable-nls
    fail_panic "Couldn't configure cloog"
    run make -j$NUM_JOBS
    fail_panic "Couldn't compile cloog"
    run make install
    fail_panic "Couldn't install cloog to $TOOLCHAIN_BUILD_PREFIX"

    EXTRA_CONFIG_FLAGS="--with-cloog=$TOOLCHAIN_BUILD_PREFIX --with-isl=$TOOLCHAIN_BUILD_PREFIX"
fi # POLLY = yes

# configure the toolchain
dump "Configure: $TOOLCHAIN toolchain build"
LLVM_BUILD_OUT=$BUILD_OUT/llvm
mkdir -p $LLVM_BUILD_OUT && cd $LLVM_BUILD_OUT
fail_panic "Couldn't cd into llvm build path: $LLVM_BUILD_OUT"

run $SRC_DIR/$TOOLCHAIN/llvm/configure \
    --prefix=$TOOLCHAIN_BUILD_PREFIX \
    --host=$ABI_CONFIGURE_HOST \
    --build=$ABI_CONFIGURE_BUILD \
    --with-bug-report-url=$DEFAULT_ISSUE_TRACKER_URL \
    --enable-targets=arm,mips,x86 \
    --enable-optimized \
    $EXTRA_CONFIG_FLAGS
fail_panic "Couldn't configure llvm toolchain"


# build the toolchain
dump "Building : llvm toolchain [this can take a long time]."
cd $LLVM_BUILD_OUT
run make -j$NUM_JOBS
fail_panic "Couldn't compile llvm toolchain"

if [ "$CHECK" = "yes" -a "$MINGW" != "yes" ] ; then
    # run the regression test
    dump "Running  : llvm toolchain regression test"
    cd $LLVM_BUILD_OUT
    run make check-all
    fail_warning "Couldn't pass all llvm regression test"  # change to fail_panic later
    if [ "$POLLY" = "yes" ]; then
        dump "Running  : polly toolchain regression test"
        cd $LLVM_BUILD_OUT
        run make polly-test -C tools/polly/test
        fail_warning "Couldn't pass all polly regression test"  # change to fail_panic later
    fi
fi

# install the toolchain to its final location
dump "Install  : llvm toolchain binaries."
cd $LLVM_BUILD_OUT && run make install
fail_panic "Couldn't install llvm toolchain to $TOOLCHAIN_BUILD_PREFIX"

# clean static or shared libraries
rm -rf $TOOLCHAIN_BUILD_PREFIX/docs
rm -rf $TOOLCHAIN_BUILD_PREFIX/include
rm -rf $TOOLCHAIN_BUILD_PREFIX/lib/*.a
rm -rf $TOOLCHAIN_BUILD_PREFIX/lib/*.la
rm -rf $TOOLCHAIN_BUILD_PREFIX/lib/pkgconfig
rm -rf $TOOLCHAIN_BUILD_PREFIX/lib/lib*.so
rm -rf $TOOLCHAIN_BUILD_PREFIX/lib/lib*.dylib
rm -rf $TOOLCHAIN_BUILD_PREFIX/lib/B*.so
rm -rf $TOOLCHAIN_BUILD_PREFIX/lib/B*.dylib
rm -rf $TOOLCHAIN_BUILD_PREFIX/lib/LLVMH*.so
rm -rf $TOOLCHAIN_BUILD_PREFIX/lib/LLVMH*.dylib
rm -rf $TOOLCHAIN_BUILD_PREFIX/share

UNUSED_LLVM_EXECUTABLES="
bugpoint c-index-test clang-tblgen lli llvm-ar llvm-as llvm-bcanalyzer
llvm-config llvm-cov llvm-diff llvm-dwarfdump llvm-extract llvm-ld llvm-mc
llvm-nm llvm-objdump llvm-prof llvm-ranlib llvm-readobj llvm-rtdyld llvm-size
llvm-stress llvm-stub llvm-tblgen macho-dump cloog"

for i in $UNUSED_LLVM_EXECUTABLES; do
    rm -f $TOOLCHAIN_BUILD_PREFIX/bin/$i
    rm -f $TOOLCHAIN_BUILD_PREFIX/bin/$i.exe
done

# copy to toolchain path
run copy_directory "$TOOLCHAIN_BUILD_PREFIX" "$TOOLCHAIN_PATH"

# create analyzer/++ scripts
for ABI in $PREBUILT_ABIS; do
    ANALYZER_PATH="$TOOLCHAIN_PATH/bin/$ABI"
    ANALYZER="$ANALYZER_PATH/analyzer"
    mkdir -p "$ANALYZER_PATH"
    case "$ABI" in
      armeabi)
          LLVM_TARGET=armv5te-none-linux-androideabi
          ;;
      armeabi-v7a)
          LLVM_TARGET=armv7-none-linux-androideabi
          ;;
      x86)
          LLVM_TARGET=i686-none-linux-android
          ;;
      mips)
          LLVM_TARGET=mipsel-none-linux-android
          ;;
      *)
        dump "ERROR: Unsupported NDK ABI: $ABI"
        exit 1
    esac

    cat > "${ANALYZER}" <<EOF
if [ "\$1" != "-cc1" ]; then
    \`dirname \$0\`/../clang -target $LLVM_TARGET "\$@"
else
    # target/triple already spelled out.
    \`dirname \$0\`/../clang "\$@"
fi
EOF
    cat > "${ANALYZER}++" <<EOF
if [ "\$1" != "-cc1" ]; then
    \`dirname \$0\`/../clang++ -target $LLVM_TARGET "\$@"
else
    # target/triple already spelled out.
    \`dirname \$0\`/../clang++ "\$@"
fi
EOF
    chmod 0755 "${ANALYZER}" "${ANALYZER}++"

    if [ -n "$HOST_EXE" ] ; then
        cat > "${ANALYZER}.cmd" <<EOF
@echo off
if "%1" == "-cc1" goto :L
%~dp0\\..\\clang${HOST_EXE} -target $LLVM_TARGET %*
if ERRORLEVEL 1 exit /b 1
goto :done
:L
rem target/triple already spelled out.
%~dp0\\..\\clang${HOST_EXE} %*
if ERRORLEVEL 1 exit /b 1
:done
EOF
        cat > "${ANALYZER}++.cmd" <<EOF
@echo off
if "%1" == "-cc1" goto :L
%~dp0\\..\\clang++${HOST_EXE} -target $LLVM_TARGET %*
if ERRORLEVEL 1 exit /b 1
goto :done
:L
rem target/triple already spelled out.
%~dp0\\..\\clang++${HOST_EXE} %*
if ERRORLEVEL 1 exit /b 1
:done
EOF
    fi
done

if [ "$PACKAGE_DIR" ]; then
    ARCHIVE="$TOOLCHAIN-$HOST_TAG.tar.bz2"
    SUBDIR=$(get_toolchain_install_subdir $TOOLCHAIN $HOST_TAG)
    dump "Packaging $ARCHIVE"
    pack_archive "$PACKAGE_DIR/$ARCHIVE" "$NDK_DIR" "$SUBDIR"
fi

dump "Done."
if [ -z "$OPTION_BUILD_OUT" ] ; then
    rm -rf $BUILD_OUT
fi
