get glibc version
```sh
ldd --version
```

update GCC
```sh
cd $HOME/Downloads &&
wget https://ftp.gnu.org/gnu/gcc/gcc-14.2.0/gcc-14.2.0.tar.gz &&
tar -xvf gcc-14.2.0.tar.gz &&
cd gcc-14.2.0 &&
sudo apt install build-essential libgmp-dev libmpfr-dev libmpc-dev gnat && 

# full potential of the Ryzen 7 5800H // --with-multilib-list=m64
./configure --prefix=/usr --enable-languages=c \
--with-arch=znver3 --with-tune=znver3 --with-multilib-list=m32 \
--enable-offload-targets=amdgcn-amdhsa --enable-lto --enable-threads=posix \
--enable-libstdcxx-threads --enable-libstdcxx-time \
--enable-checking=release --enable-multilib --disable-werror &&

make -j$(nproc) &&
sudo make install
```

[Part 1: To bigger things in GCC-14](https://community.arm.com/arm-community-blogs/b/tools-software-ides-blog/posts/p1-gcc-14)
[Auto-vectorization in GCC](https://gcc.gnu.org/projects/tree-ssa/vectorization.html)
[Vectorization optimization in GCC](https://developers.redhat.com/articles/2023/12/08/vectorization-optimization-gcc#)


Using built-in specs.
COLLECT_GCC=gcc
COLLECT_LTO_WRAPPER=/usr/libexec/gcc/x86_64-linux-gnu/13/lto-wrapper
OFFLOAD_TARGET_NAMES=nvptx-none:amdgcn-amdhsa
OFFLOAD_TARGET_DEFAULT=1
Target: x86_64-linux-gnu
Configured with: ../src/configure -v --with-pkgversion='Ubuntu 13.2.0-23ubuntu4' --with-bugurl=file:///usr/share/doc/gcc-13/README.Bugs --enable-languages=c,ada,c++,go,d,fortran,objc,obj-c++,m2 --prefix=/usr --with-gcc-major-version-only --program-suffix=-13 --program-prefix=x86_64-linux-gnu- --enable-shared --enable-linker-build-id --libexecdir=/usr/libexec --without-included-gettext --enable-threads=posix --libdir=/usr/lib --enable-nls --enable-clocale=gnu --enable-libstdcxx-debug --enable-libstdcxx-time=yes --with-default-libstdcxx-abi=new --enable-libstdcxx-backtrace --enable-gnu-unique-object --disable-vtable-verify --enable-plugin --enable-default-pie --with-system-zlib --enable-libphobos-checking=release --with-target-system-zlib=auto --enable-objc-gc=auto --enable-multiarch --disable-werror --enable-cet --with-arch-32=i686 --with-abi=m64 --with-multilib-list=m32,m64,mx32 --enable-multilib --with-tune=generic --enable-offload-targets=nvptx-none=/build/gcc-13-uJ7kn6/gcc-13-13.2.0/debian/tmp-nvptx/usr,amdgcn-amdhsa=/build/gcc-13-uJ7kn6/gcc-13-13.2.0/debian/tmp-gcn/usr --enable-offload-defaulted --without-cuda-driver --enable-checking=release --build=x86_64-linux-gnu --host=x86_64-linux-gnu --target=x86_64-linux-gnu
Thread model: posix
Supported LTO compression algorithms: zlib zstd
gcc version 13.2.0 (Ubuntu 13.2.0-23ubuntu4) 