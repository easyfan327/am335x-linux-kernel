DEST="/mnt/hgfs/work/date/uImage"
SRC="arch/arm/boot/uImage"

if ! [ -f ".config" ]; then
    make am335x_evm_defconfig
    [ $? != 0 ] && exit 1
fi

make uImage -j2
[ $? != 0 ] && exit 1

echo "INFO: COPY $SRC -> $DEST"
cp -f $SRC $DEST
