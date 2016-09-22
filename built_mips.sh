OUT_TAR=/opt/buildroot-gcc342/lib/
make clean
TAR_LIB=libbase342.so
make CROSS_COMPILE=mipsel-linux-  TAR_V=$TAR_LIB
cp $TAR_LIB $OUT_TAR

