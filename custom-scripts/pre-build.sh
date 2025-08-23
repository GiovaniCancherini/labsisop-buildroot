#!/bin/sh

# if folder BASE_DIR exists 
if [ -d "$BASE_DIR" ]; then
    cp $BASE_DIR/../custom-scripts/S41network-config $BASE_DIR/target/etc/init.d
    chmod +x $BASE_DIR/target/etc/init.d/S41network-config

    cp $BASE_DIR/../custom-scripts/S50hello-config $BASE_DIR/target/etc/init.d
    chmod +x $BASE_DIR/target/etc/init.d/S50hello-config
fi