
OUT_TAR=/opt/buildroot-gcc463/usr/mipsel-buildroot-linux-uclibc/lib/
make clean
TAR_LIB=libbase463.so
make CROSS_COMPILE=/opt/buildroot-gcc463/usr/bin/mipsel-linux-  TAR_V=$TAR_LIB
cp $TAR_LIB $OUT_TAR

