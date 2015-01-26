all: environode
MIST=../..
APPS+=mqtt

include $(MIST)/Makefile.mist

ifndef COMPORT
COMPORT=/dev/ttyACM0
endif # COMPORT

$(CONTIKI)/tools/tunslip6:	$(CONTIKI)/tools/tunslip6.c
	(cd $(CONTIKI)/tools && $(MAKE) tunslip6)
connect-slip-router:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 -s $(COMPORT) aaaa::1/64