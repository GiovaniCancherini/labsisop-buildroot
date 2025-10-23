#!/bin/sh

# if folder BASE_DIR exists 
if [ -d "$BASE_DIR" ]; then
    # network-config
    cp $BASE_DIR/../custom-scripts/S41network-config $BASE_DIR/target/etc/init.d
    chmod +x $BASE_DIR/target/etc/init.d/S41network-config

    # hello
    cp $BASE_DIR/../custom-scripts/S50hello-config $BASE_DIR/target/etc/init.d
    chmod +x $BASE_DIR/target/etc/init.d/S50hello-config

    # tp1
    cp $BASE_DIR/../custom-scripts/S60tp1systeminfo-config $BASE_DIR/target/etc/init.d
    chmod +x $BASE_DIR/target/etc/init.d/S60tp1systeminfo-config
    cp $BASE_DIR/../tp1-systeminfo/systeminfo.py $BASE_DIR/target/usr/bin/
    chmod +x $BASE_DIR/target/usr/bin/systeminfo.py

    # 2.3 simple_driver
    make -C $BASE_DIR/../modules/simple_driver/
    # 2.3 simple_driver atividade 1
    make -C $BASE_DIR/../modules/simple_driver_atividade1/
    # 2.3 simple_driver atividade 2
    make -C $BASE_DIR/../modules/simple_driver_atividade2/
    # 2.3 simple_driver desafio
    make -C $BASE_DIR/../modules/simple_driver_desafio/

    # 2.4 disk-test: compilação cruzada e inclusão do binário
    /workspaces/labsisop-buildroot/output/host/bin/i686-buildroot-linux-gnu-gcc -o $BASE_DIR/../disk-test/write_hello_bin $BASE_DIR/../disk-test/write_hello.c
    # Copia o binário para o target do Buildroot e o torna executável
    cp $BASE_DIR/../disk-test/write_hello_bin $BASE_DIR/target/usr/bin/write_hello
    chmod +x $BASE_DIR/target/usr/bin/write_hello

    # tp2
    make -C $BASE_DIR/../modules/sstf-iosched-skeleton-master/

    # mount tracefs on boot (ftrace)
    cp $BASE_DIR/../custom-scripts/S70mount-tracefs-config $BASE_DIR/target/etc/init.d/
    chmod +x $BASE_DIR/target/etc/init.d/S70mount-tracefs-config

fi