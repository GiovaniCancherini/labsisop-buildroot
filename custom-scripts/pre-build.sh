#!/bin/sh

# if folder BASE_DIR exists 
if [ -d "$BASE_DIR" ]; then
    cp $BASE_DIR/../custom-scripts/S41network-config $BASE_DIR/target/etc/init.d
    chmod +x $BASE_DIR/target/etc/init.d/S41network-config

    cp $BASE_DIR/../custom-scripts/S50hello-config $BASE_DIR/target/etc/init.d
    chmod +x $BASE_DIR/target/etc/init.d/S50hello-config

    # tp1
    cp $BASE_DIR/../custom-scripts/S60tp1systeminfo-config $BASE_DIR/target/etc/init.d
    chmod +x $BASE_DIR/target/etc/init.d/S60tp1systeminfo-config
    cp $BASE_DIR/../tp1-systeminfo/systeminfo.py $BASE_DIR/target/usr/bin/
    chmod +x $BASE_DIR/target/usr/bin/systeminfo.py

    # 2.3
    make -C $BASE_DIR/../modules/simple_driver/
fi