#include "dtm-jtag.h"
#include "rvl-tap.h"
#include "rvl-assert.h"

typedef struct rvl_dtm_s {
    struct pt pt;
    uint8_t abits;
    uint8_t version;
    uint32_t in[2];
    uint32_t out[2];
    uint8_t last_jtag_reg;
}rvl_dtm_t;

static rvl_dtm_t rvl_dtm_i;
#define self rvl_dtm_i


#define RISCV_DTM_JTAG_REG_IDCODE           0x01
#define RISCV_DTM_JTAG_REG_DTMCS            0x10
#define RISCV_DTM_JTAG_REG_DMI              0x11
#define RISCV_DTM_JTAG_REG_BYPASS           0x1f


#define RISCV_DTMCS_DMI_RESET           (1 << 16)
#define RISCV_DTMCS_DMI_HARD_RESET      (1 << 17)


void rvl_dtm_init(void)
{
    PT_INIT(&self.pt);
    self.last_jtag_reg = 0;

    rvl_tap_init();
    rvl_tap_go_idle();
}


PT_THREAD(rvl_dtm_idcode(uint32_t* idcode))
{
    PT_BEGIN(&self.pt);

    if(self.last_jtag_reg != RISCV_DTM_JTAG_REG_IDCODE) {
        self.in[0] = RISCV_DTM_JTAG_REG_IDCODE;
        self.last_jtag_reg = RISCV_DTM_JTAG_REG_IDCODE;
        rvl_tap_shift_ir(self.out, self.in, 5);
        PT_YIELD(&self.pt);
    }

    self.in[0] = 0;
    rvl_tap_shift_dr(self.out, self.in, 32);
    PT_YIELD(&self.pt);

    *idcode = self.out[0];

    PT_END(&self.pt);
}


PT_THREAD(rvl_dtm_dtmcs(uint32_t* dtmcs))
{
    PT_BEGIN(&self.pt);

    if(self.last_jtag_reg != RISCV_DTM_JTAG_REG_DTMCS) {
        self.in[0] = RISCV_DTM_JTAG_REG_DTMCS;
        self.last_jtag_reg = RISCV_DTM_JTAG_REG_DTMCS;
        rvl_tap_shift_ir(self.out, self.in, 5);
        PT_YIELD(&self.pt);
    }

    self.in[0] = 0;
    rvl_tap_shift_dr(self.out, self.in, 32);
    PT_YIELD(&self.pt);

    *dtmcs = self.out[0];

    self.abits = (self.out[0] >> 4) & 0x3f;
    rvl_assert(self.abits <= 32);

    self.version = (self.out[0]) & 0xf;

    PT_END(&self.pt);
}


PT_THREAD(rvl_dtm_dtmcs_dmireset(void))
{
    PT_BEGIN(&self.pt);

    if(self.last_jtag_reg != RISCV_DTM_JTAG_REG_DTMCS) {
        self.in[0] = RISCV_DTM_JTAG_REG_DTMCS;
        self.last_jtag_reg = RISCV_DTM_JTAG_REG_DTMCS;
        rvl_tap_shift_ir(self.out, self.in, 5);
        PT_YIELD(&self.pt);
    }

    self.in[0] = RISCV_DTMCS_DMI_RESET;
    rvl_tap_shift_dr(self.out, self.in, 32);
    PT_YIELD(&self.pt);

    PT_END(&self.pt);
}


PT_THREAD(rvl_dtm_dtmcs_dmihardreset(void))
{
    PT_BEGIN(&self.pt);

    if(self.last_jtag_reg != RISCV_DTM_JTAG_REG_DTMCS) {
        self.in[0] = RISCV_DTM_JTAG_REG_DTMCS;
        self.last_jtag_reg = RISCV_DTM_JTAG_REG_DTMCS;
        rvl_tap_shift_ir(self.out, self.in, 5);
        PT_YIELD(&self.pt);
    }

    self.in[0] = RISCV_DTMCS_DMI_HARD_RESET;
    rvl_tap_shift_dr(self.out, self.in, 32);
    PT_YIELD(&self.pt);

    PT_END(&self.pt);
}


PT_THREAD(rvl_dtm_dmi(uint32_t addr, uint32_t* data, uint32_t* op))
{
    PT_BEGIN(&self.pt);

    rvl_assert(addr <= ((1 << self.abits) - 1));
    rvl_assert(data != NULL);
    rvl_assert(op != NULL && *op <= 3);

    if(self.last_jtag_reg != RISCV_DTM_JTAG_REG_DMI) {
        self.in[0] = RISCV_DTM_JTAG_REG_DMI;
        self.last_jtag_reg = RISCV_DTM_JTAG_REG_DMI;
        rvl_tap_shift_ir(self.out, self.in, 5);
        PT_YIELD(&self.pt);
    }

    self.in[0] = (*data << 2) | (*op & 0x3);
    self.in[1] = (*data >> 30) | (addr << 2);
    rvl_tap_shift_dr(self.out, self.in, 32 + 2 + self.abits);
    PT_YIELD(&self.pt);

    *op = self.out[0] & 0x3;
    *data = (self.out[0] >> 2) | (self.out[1] << 30);

    PT_END(&self.pt);
}