ifdef CROSS_COMPILE
 ifndef BASE
 $(error specify BASE)
 endif
else
RELEASE = $(shell uname -r)
BASE = /lib/modules/$(RELEASE)/build
endif
#INCLUDE = -I$(BASE)/include/

obj-m += mmuhack.o
#modtest-objs += mmuhack.o


all:
	make -C $(BASE) M=$(PWD) modules

clean:
	make -C $(BASE) M=$(PWD) clean
	$(RM) modules.order Module.markers

