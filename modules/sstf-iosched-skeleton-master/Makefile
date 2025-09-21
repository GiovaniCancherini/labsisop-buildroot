obj-m := sstf-iosched.o
BUILDROOT_DIR := ../..
KDIR := $(BUILDROOT_DIR)/output/build/linux-custom
CROSS_COMPILE := $(firstword $(wildcard $(BUILDROOT_DIR)/output/host/bin/*-gcc))
COMPILER := $(if $(CROSS_COMPILE),$(CROSS_COMPILE),gcc)

all:
	$(MAKE) -C $(KDIR) M=$$PWD
	$(MAKE) -C $(KDIR) M=$$PWD modules_install INSTALL_MOD_PATH=../../target
	$(COMPILER) -o sector_read sector_read.c
	cp sector_read $(BUILDROOT_DIR)/output/target/bin
	
	
	
clean:
	rm -f *.o *.ko .*.cmd
	rm -f modules.order
	rm -f Module.symvers
	rm -f sstf-iosched.mod.c
	rm -f sector_read

