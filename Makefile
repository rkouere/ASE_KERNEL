# If KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language.
ifneq ($(KERNELRELEASE),)
	obj-m := ase_cmd.o
# Otherwise we were called directly from the command
# line; invoke the kernel build system.
else
	KERNELDIR ?= /gfs/echallier/build/linux-3.19.1
	PWD := $(shell pwd)
default:
	# on est oblige de rajouter 0=... car le make va chercher le .config que l'on peut trouver dans kvm32
	$(MAKE) -C $(KERNELDIR) O=/gfs/echallier/build/kvm32/ M=$(PWD) modules
endif