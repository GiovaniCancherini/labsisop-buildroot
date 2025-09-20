obj-m := simple_driver_atividade1.o
BUILDROOT_DIR := ../..
KDIR := $(BUILDROOT_DIR)/output/build/linux-custom
COMPILER := $(BUILDROOT_DIR)/output/host/bin/i686-buildroot-linux-gnu-gcc

all:
	$(MAKE) -C $(KDIR) M=$$PWD
	$(MAKE) -C $(KDIR) M=$$PWD modules_install INSTALL_MOD_PATH=../../target
	$(COMPILER) -o test_simple_driver_atividade1 test_simple_driver_atividade1.c
	cp test_simple_driver_atividade1 $(BUILDROOT_DIR)/output/target/bin
	
clean:
	rm -f *.o *.ko .*.cmd
	rm -f modules.order
	rm -f Module.symvers
	rm -f simple_driver_atividade1.mod.c
	rm -f test_simple_driver_atividade1
