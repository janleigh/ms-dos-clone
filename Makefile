.PHONY: all clean run iso debug

all:
	$(MAKE) -C src all

clean:
	$(MAKE) -C src clean

run:
	$(MAKE) -C src run

iso:
	$(MAKE) -C src iso

debug:
	$(MAKE) -C src debug