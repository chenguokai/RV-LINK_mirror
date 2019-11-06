OUTPUT_DIR = output

all:
	$(MAKE) gd32vf103c-start
	$(MAKE) longan-nano
	$(MAKE) longan-nano-alt

gd32vf103c-start:
	@echo "building for gd32vf103c-start"
	$(MAKE) build elf=$(OUTPUT_DIR)/gd32vf103c-start+gd32vf103+gdb-server.elf
longan-nano:
	@echo "building for longan nano"
	$(MAKE) build elf=$(OUTPUT_DIR)/longan-nano+gd32vf103+gdb-server.elf
longan-nano-alt:
	@echo "building for longan nano alt"
	$(MAKE) build elf=$(OUTPUT_DIR)/longan-nano-alt+gd32vf103+gdb-server.elf
#elf += $(OUTPUT_DIR)/gd32vf103c-start+riscv-debug-spec-v0p13+riscv-prober.elf
#elf += $(OUTPUT_DIR)/gd32vf103c-start+riscv-debug-spec-v0p11+riscv-prober.elf 
test-usb-serial:
	@echo "building for test-usb-serial"
	$(MAKE) build elf=$(OUTPUT_DIR)/gd32vf103c-start+gd32vf103+test-usb-serial.elf

build: $(elf)

get_link = $(shell echo $@ | awk -F'.' '{print $$1}' | awk -F'/' '{print $$2}' | awk -F'+' '{print $$1}')
get_target = $(shell echo $@ | awk -F'.' '{print $$1}' | awk -F'/' '{print $$2}' | awk -F'+' '{print $$2}')
get_app = $(shell echo $@ | awk -F'.' '{print $$1}' | awk -F'/' '{print $$2}' | awk -F'+' '{print $$3}')

$(OUTPUT_DIR)/%.elf:
	make -C src clean
	make -C src LINK=$(get_link) TARGET=$(get_target) APP=$(get_app)

clean:
	rm -rf output
