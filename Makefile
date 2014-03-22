.PHONY: all sim fw clean run

all:
	$(MAKE) -C firmware
	$(MAKE) -C simulation

sim:
	$(MAKE) -C simulation

fw:
	$(MAKE) -C firmware

clean:
	$(MAKE) -C firmware clean
	$(MAKE) -C simulation clean

run: sim fw
	$(MAKE) -C simulation run
