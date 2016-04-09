#!/bin/bash

export CC=gcc-5
export CXX=g++-5
export LOGFILE=gcc5_sdl2_tests.log

source ./tools/ci/scripts/init.sh

aptget_install gcc-5 g++-5 \
    make autoconf automake autopoint gettext libphysfs-dev \
    libxml2-dev libcurl4-gnutls-dev libpng-dev \
    libsdl2-dev libsdl2-gfx-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-net-dev libsdl2-ttf-dev

export CXXFLAGS="-ggdb3 -O2 -pipe -ffast-math \
-fno-omit-frame-pointer -funswitch-loops \
-Wvariadic-macros -Wvla -Wredundant-decls \
-Wpacked-bitfield-compat -Wtrampolines \
-Wsuggest-attribute=noreturn -Wunused -Wstrict-aliasing=2 \
-fstrict-aliasing -Wunreachable-code \
-Wvolatile-register-var -Winvalid-pch -Wredundant-decls \
-Wnormalized=nfkc -Wmissing-format-attribute -Wmissing-noreturn \
-Wswitch-default -Wsign-promo -Waddress -Wmissing-declarations \
-Wctor-dtor-privacy -Wstrict-null-sentinel -Wlogical-op \
-Wcast-align -Wpointer-arith -Wundef \
-Wmissing-include-dirs -Winit-self -pedantic -Wall \
-Wpacked -Wextra -fstrict-overflow -Wstrict-overflow=1 -Wunknown-pragmas \
-Wwrite-strings -Wstack-protector -Wshadow -Wunused-macros -Wsynth \
-Wbuiltin-macro-redefined -Wctor-dtor-privacy -Wdeprecated \
-Wendif-labels -Wformat=1 -Wimport -Wnon-virtual-dtor -Wpsabi \
-Wsign-promo -Wwrite-strings -D_FORTIFY_SOURCE=2 -std=gnu++1y \
-Wdelete-non-virtual-dtor -Wmaybe-uninitialized -Wunused-local-typedefs \
-Wvector-operation-performance -Wfree-nonheap-object -Winvalid-memory-model \
-Wnarrowing -Wzero-as-null-pointer-constant -funsafe-loop-optimizations \
-Waggressive-loop-optimizations -Wclobbered -Wempty-body \
-Wignored-qualifiers -Wliteral-suffix -Wmissing-field-initializers \
-Woverlength-strings -Wpedantic -Wsign-compare -Wsizeof-pointer-memaccess \
-Wsuggest-attribute=format -Wtype-limits -Wuninitialized \
-Wunused-but-set-parameter -Wunused-but-set-variable -Wunused-function \
-Wunused-label -Wunused-parameter -Wunused-value -Wunused-variable -ftrapv \
-fsched-pressure \
-Wconditionally-supported -Wdate-time \
-fno-var-tracking \
-Wopenmp-simd -Wformat-signedness \
-Wswitch-bool -Wmemset-transposed-args -Wchkp -Wc++14-compat -Wsized-deallocation \
-Wlogical-not-parentheses -Woverloaded-virtual"

run_configure --with-sdl2 --enable-unittests=yes
export SDL_VIDEODRIVER=dummy
run_make check
#run_check_warnings

source ./tools/ci/scripts/exit.sh

exit 0
