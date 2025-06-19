/*
    Copyright (c) 2023 Raspberry Pi (Trading) Ltd.

    SPDX-License-Identifier: BSD-3-Clause
*/

#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/error.h"

#include "hardware/dma.h"
#include "hardware/clocks.h"

#include "wiznet_pio_qspi.h"
#include "wiznet_pio_qspi.pio.h"

#ifndef PIO_QSPI_PREFERRED_PIO
#define PIO_QSPI_PREFERRED_PIO 1
#endif

#define PADS_DRIVE_STRENGTH PADS_BANK0_GPIO0_DRIVE_VALUE_12MA
#define IRQ_SAMPLE_DELAY_NS 100

#define WIZNET_PIO_QSPI_PROGRAM_NAME wizchip_pio_spi_quad_write_read
#define WIZNET_PIO_QSPI_PROGRAM_FUNC __CONCAT(WIZNET_PIO_QSPI_PROGRAM_NAME, _program)
#define WIZNET_PIO_QSPI_PROGRAM_GET_DEFAULT_CONFIG_FUNC __CONCAT(WIZNET_PIO_QSPI_PROGRAM_NAME, _program_get_default_config)
#define WIZNET_PIO_QSPI_OFFSET_WRITE_BITS __CONCAT(WIZNET_PIO_QSPI_PROGRAM_NAME, _offset_write_bits)
#define WIZNET_PIO_QSPI_OFFSET_WRITE_BITS_END __CONCAT(WIZNET_PIO_QSPI_PROGRAM_NAME, _offset_write_bits_end)
#define WIZNET_PIO_QSPI_OFFSET_READ_END __CONCAT(WIZNET_PIO_QSPI_PROGRAM_NAME, _offset_read_bits_end)

// All wiznet spi operations must start with writing a 3 byte header
#define WIZNET_PIO_QSPI_HEADER_LEN 3

#ifndef WIZNET_PICO_PIO_QSPI_INSTANCE_COUNT
#define WIZNET_PICO_PIO_QSPI_INSTANCE_COUNT 1
#endif

typedef struct wiznet_pio_qspi_state {
    wiznet_pio_qspi_funcs_t *funcs;
    const wiznet_pio_qspi_config_t *qspi_config;
    pio_hw_t *pio;
    uint8_t pio_func_sel;
    int8_t pio_offset;
    int8_t pio_sm;
    int8_t dma_out;
    int8_t dma_in;
    uint8_t qspi_header[WIZNET_PIO_QSPI_HEADER_LEN];
    uint8_t qspi_header_count;
} wiznet_pio_qspi_state_t;

static wiznet_pio_qspi_state_t wiznet_pio_qspi_state[WIZNET_PICO_PIO_QSPI_INSTANCE_COUNT];
static wiznet_pio_qspi_state_t *active_state;

static wiznet_pio_qspi_funcs_t *get_wiznet_pio_qspi_impl(void);

static uint16_t mk_cmd_buf(uint8_t *pdst, uint8_t opcode, uint16_t addr) {
    pdst[0] = ((opcode >> 7 & 0x01) << 4) | ((opcode >> 6 & 0x01) << 0);
    pdst[1] = ((opcode >> 5 & 0x01) << 4) | ((opcode >> 4 & 0x01) << 0);
    pdst[2] = ((opcode >> 3 & 0x01) << 4) | ((opcode >> 2 & 0x01) << 0);
    pdst[3] = ((opcode >> 1 & 0x01) << 4) | ((opcode >> 0 & 0x01) << 0);

    pdst[4] = ((uint8_t)(addr >> 8) & 0xFF);
    pdst[5] = ((uint8_t)(addr >> 0) & 0xFF);

    pdst[6] = 0;

    return 6 + 1;
}

// Initialise our gpios
static void pio_qspi_gpio_setup(wiznet_pio_qspi_state_t *state) {
    // Setup DO and DI
    gpio_init(state->qspi_config->data_io0_pin);
    gpio_init(state->qspi_config->data_io1_pin);
    gpio_init(state->qspi_config->data_io2_pin);
    gpio_init(state->qspi_config->data_io3_pin);
    gpio_set_dir(state->qspi_config->data_io0_pin, GPIO_OUT);
    gpio_set_dir(state->qspi_config->data_io1_pin, GPIO_OUT);
    gpio_set_dir(state->qspi_config->data_io2_pin, GPIO_OUT);
    gpio_set_dir(state->qspi_config->data_io3_pin, GPIO_OUT);
    gpio_put(state->qspi_config->data_io0_pin, false);
    gpio_put(state->qspi_config->data_io1_pin, false);
    gpio_put(state->qspi_config->data_io2_pin, false);
    gpio_put(state->qspi_config->data_io3_pin, false);

    // Setup CS
    gpio_init(state->qspi_config->cs_pin);
    gpio_set_dir(state->qspi_config->cs_pin, GPIO_OUT);
    gpio_put(state->qspi_config->cs_pin, true);

    // Setup reset
    gpio_init(state->qspi_config->irq_pin);
    gpio_set_dir(state->qspi_config->irq_pin, GPIO_IN);
    gpio_set_pulls(state->qspi_config->irq_pin, false, false);
}

wiznet_pio_qspi_handle_t wiznet_pio_qspi_open(const wiznet_pio_qspi_config_t *qspi_config) {

    wiznet_pio_qspi_state_t *state;
    for (unsigned int i = 0; i < count_of(wiznet_pio_qspi_state); i++) {
        if (!wiznet_pio_qspi_state[i].funcs) {
            state = &wiznet_pio_qspi_state[i];
            break;
        }
    }
    assert(state);
    if (!state) {
        return NULL;
    }
    state->qspi_config = qspi_config;
    state->funcs = get_wiznet_pio_qspi_impl();

    pio_qspi_gpio_setup(state);

    pio_hw_t *pios[2] = {pio0, pio1};
    uint pio_index = PIO_QSPI_PREFERRED_PIO;

    if (!pio_can_add_program(pios[pio_index], &WIZNET_PIO_QSPI_PROGRAM_FUNC)) {
        pio_index ^= 1;
        if (!pio_can_add_program(pios[pio_index], &WIZNET_PIO_QSPI_PROGRAM_FUNC)) {
            return NULL;
        }
    }

    state->pio = pios[pio_index];
    state->dma_in = -1;
    state->dma_out = -1;

    static_assert(GPIO_FUNC_PIO1 == GPIO_FUNC_PIO0 + 1, "");
    state->pio_func_sel = GPIO_FUNC_PIO0 + pio_index;
    state->pio_sm = (int8_t)pio_claim_unused_sm(state->pio, false);
    if (state->pio_sm < 0) {
        wiznet_pio_qspi_close(&state->funcs);
        return NULL;
    }

    state->pio_offset = pio_add_program(state->pio, &WIZNET_PIO_QSPI_PROGRAM_FUNC);
    printf("[QSPI CLOCK SPEED : %.2lf MHz]\r\n\r\n", 66.5 / (state->qspi_config->clock_div_major + ((double)state->qspi_config->clock_div_minor / 256)));
    pio_sm_config sm_config = WIZNET_PIO_QSPI_PROGRAM_GET_DEFAULT_CONFIG_FUNC(state->pio_offset);

    sm_config_set_clkdiv_int_frac(&sm_config, state->qspi_config->clock_div_major, state->qspi_config->clock_div_minor);
    hw_write_masked(&pads_bank0_hw->io[state->qspi_config->clock_pin],
                    (uint)PADS_DRIVE_STRENGTH << PADS_BANK0_GPIO0_DRIVE_LSB,
                    PADS_BANK0_GPIO0_DRIVE_BITS
                   );
    hw_write_masked(&pads_bank0_hw->io[state->qspi_config->clock_pin],
                    (uint)1 << PADS_BANK0_GPIO0_SLEWFAST_LSB,
                    PADS_BANK0_GPIO0_SLEWFAST_BITS
                   );

    printf("\r\n[QSPI QUAD MODE]\r\n");
    sm_config_set_out_pins(&sm_config, state->qspi_config->data_io0_pin, 4);
    sm_config_set_in_pins(&sm_config, state->qspi_config->data_io0_pin);
    sm_config_set_set_pins(&sm_config, state->qspi_config->data_io0_pin, 4);
    sm_config_set_sideset(&sm_config, 1, false, false);
    sm_config_set_sideset_pins(&sm_config, state->qspi_config->clock_pin);

    sm_config_set_in_shift(&sm_config, false, true, 8);
    sm_config_set_out_shift(&sm_config, false, true, 8);

    hw_set_bits(&state->pio->input_sync_bypass,
                (1u << state->qspi_config->data_io0_pin) | (1u << state->qspi_config->data_io1_pin) | (1u << state->qspi_config->data_io2_pin) | (1u << state->qspi_config->data_io3_pin));
    pio_sm_set_config(state->pio, state->pio_sm, &sm_config);
    pio_sm_set_consecutive_pindirs(state->pio, state->pio_sm, state->qspi_config->clock_pin, 1, true);

    gpio_set_function(state->qspi_config->data_io0_pin, state->pio_func_sel);
    gpio_set_function(state->qspi_config->data_io1_pin, state->pio_func_sel);
    gpio_set_function(state->qspi_config->data_io2_pin, state->pio_func_sel);
    gpio_set_function(state->qspi_config->data_io3_pin, state->pio_func_sel);

    // Set data pin to pull down and schmitt
    gpio_set_pulls(state->qspi_config->data_io0_pin, false, true);
    gpio_set_pulls(state->qspi_config->data_io1_pin, false, true);
    gpio_set_pulls(state->qspi_config->data_io2_pin, false, true);
    gpio_set_pulls(state->qspi_config->data_io3_pin, false, true);
    gpio_set_input_hysteresis_enabled(state->qspi_config->data_io0_pin, true);
    gpio_set_input_hysteresis_enabled(state->qspi_config->data_io1_pin, true);
    gpio_set_input_hysteresis_enabled(state->qspi_config->data_io2_pin, true);
    gpio_set_input_hysteresis_enabled(state->qspi_config->data_io3_pin, true);
    /* @todo: Implement to use. */

    pio_sm_exec(state->pio, state->pio_sm, pio_encode_set(pio_pins, 1));

    state->dma_out = (int8_t)dma_claim_unused_channel(false); // todo: Should be able to use one dma channel?
    state->dma_in = (int8_t)dma_claim_unused_channel(false);
    if (state->dma_out < 0 || state->dma_in < 0) {
        wiznet_pio_qspi_close(&state->funcs);
        return NULL;
    }

    return &state->funcs;

}

void wiznet_pio_qspi_close(wiznet_pio_qspi_handle_t handle) {

    wiznet_pio_qspi_state_t *state = (wiznet_pio_qspi_state_t *)handle;
    if (state) {
        if (state->pio_sm >= 0) {
            if (state->pio_offset != -1) {
                pio_remove_program(state->pio, &WIZNET_PIO_QSPI_PROGRAM_FUNC, state->pio_offset);
            }

            pio_sm_unclaim(state->pio, state->pio_sm);
        }
        if (state->dma_out >= 0) {
            dma_channel_unclaim(state->dma_out);
            state->dma_out = -1;
        }
        if (state->dma_in >= 0) {
            dma_channel_unclaim(state->dma_in);
            state->dma_in = -1;
        }
        state->funcs = NULL;
    }
}

static void cs_set(wiznet_pio_qspi_state_t *state, bool value) {
    gpio_put(state->qspi_config->cs_pin, value);
}

static __noinline void ns_delay(uint32_t ns) {
    // cycles = ns * clk_sys_hz / 1,000,000,000
    uint32_t cycles = ns * (clock_get_hz(clk_sys) >> 16u) / (1000000000u >> 16u);
    busy_wait_at_least_cycles(cycles);
}

void wiznet_pio_qspi_frame_start(void) {
    assert(active_state);

    gpio_set_function(active_state->qspi_config->data_io0_pin, active_state->pio_func_sel);
    gpio_set_function(active_state->qspi_config->data_io1_pin, active_state->pio_func_sel);
    gpio_set_function(active_state->qspi_config->data_io2_pin, active_state->pio_func_sel);
    gpio_set_function(active_state->qspi_config->data_io3_pin, active_state->pio_func_sel);

    gpio_set_function(active_state->qspi_config->clock_pin, active_state->pio_func_sel);
    gpio_pull_down(active_state->qspi_config->clock_pin);

    // Pull CS low
    cs_set(active_state, false);
}

void wiznet_pio_qspi_frame_end(void) {
    assert(active_state);
    // from this point a positive edge will cause an IRQ to be pending
    cs_set(active_state, true);
    // we need to wait a bit in case the irq line is incorrectly high
#ifdef IRQ_SAMPLE_DELAY_NS
    ns_delay(IRQ_SAMPLE_DELAY_NS);
#endif
}

// To read a byte we must first have been asked to write a 3 byte spi header
void wiznet_pio_qspi_read_byte(uint8_t op_code, uint16_t AddrSel, uint8_t *rx, uint16_t rx_length) {
    uint8_t command_buf[8] = {0,};
    uint16_t command_len = mk_cmd_buf(command_buf, op_code, AddrSel);
    uint32_t loop_cnt = 0;

    pio_sm_set_enabled(active_state->pio, active_state->pio_sm, false);
    pio_sm_set_wrap(active_state->pio, active_state->pio_sm, active_state->pio_offset, active_state->pio_offset + WIZNET_PIO_QSPI_OFFSET_READ_END - 1);
    //pio_sm_set_wrap(active_state->pio, active_state->pio_sm, active_state->pio_offset + PIO_SPI_OFFSET_WRITE_BITS, active_state->pio_offset + PIO_SPI_OFFSET_READ_BITS_END - 1);
    pio_sm_clear_fifos(active_state->pio, active_state->pio_sm);

    loop_cnt = 2;
    pio_sm_set_pindirs_with_mask(active_state->pio,
                                 active_state->pio_sm,
                                 (1u << active_state->qspi_config->data_io0_pin) | (1u << active_state->qspi_config->data_io1_pin) | (1u << active_state->qspi_config->data_io2_pin) | (1u << active_state->qspi_config->data_io3_pin),
                                 (1u << active_state->qspi_config->data_io0_pin) | (1u << active_state->qspi_config->data_io1_pin) | (1u << active_state->qspi_config->data_io2_pin) | (1u << active_state->qspi_config->data_io3_pin));

    /* @todo: Implement to use. */

    pio_sm_restart(active_state->pio, active_state->pio_sm);
    pio_sm_clkdiv_restart(active_state->pio, active_state->pio_sm);

    pio_sm_put(active_state->pio, active_state->pio_sm, command_len * loop_cnt - 1);
    pio_sm_exec(active_state->pio, active_state->pio_sm, pio_encode_out(pio_x, 32));

    pio_sm_put(active_state->pio, active_state->pio_sm, rx_length - 1);
    pio_sm_exec(active_state->pio, active_state->pio_sm, pio_encode_out(pio_y, 32));

    pio_sm_exec(active_state->pio, active_state->pio_sm, pio_encode_jmp(active_state->pio_offset));

    dma_channel_abort(active_state->dma_out);
    dma_channel_abort(active_state->dma_in);

    dma_channel_config out_config = dma_channel_get_default_config(active_state->dma_out);
    channel_config_set_transfer_data_size(&out_config, DMA_SIZE_8);
    channel_config_set_bswap(&out_config, true);
    channel_config_set_dreq(&out_config, pio_get_dreq(active_state->pio, active_state->pio_sm, true));
    dma_channel_configure(active_state->dma_out, &out_config, &active_state->pio->txf[active_state->pio_sm], command_buf, command_len, true);

    dma_channel_config in_config = dma_channel_get_default_config(active_state->dma_in);
    channel_config_set_transfer_data_size(&in_config, DMA_SIZE_8);
    channel_config_set_bswap(&in_config, true);
    channel_config_set_dreq(&in_config, pio_get_dreq(active_state->pio, active_state->pio_sm, false));
    channel_config_set_write_increment(&in_config, true);
    channel_config_set_read_increment(&in_config, false);
    dma_channel_configure(active_state->dma_in, &in_config, rx, &active_state->pio->rxf[active_state->pio_sm], rx_length, true);

#if 1
    pio_sm_set_enabled(active_state->pio, active_state->pio_sm, true);

    __compiler_memory_barrier();

    dma_channel_wait_for_finish_blocking(active_state->dma_out);
    dma_channel_wait_for_finish_blocking(active_state->dma_in);

    __compiler_memory_barrier();

    pio_sm_set_enabled(active_state->pio, active_state->pio_sm, false);
    pio_sm_exec(active_state->pio, active_state->pio_sm, pio_encode_mov(pio_pins, pio_null));

#endif
}

void wiznet_pio_qspi_write_byte(uint8_t op_code, uint16_t AddrSel, uint8_t *tx, uint16_t tx_length) {
    uint8_t command_buf[8] = {0,};
    uint16_t command_len = mk_cmd_buf(command_buf, op_code, AddrSel);
    uint32_t loop_cnt = 0;
    tx_length = tx_length + command_len;

    pio_sm_set_enabled(active_state->pio, active_state->pio_sm, false);
    pio_sm_set_wrap(active_state->pio, active_state->pio_sm, active_state->pio_offset, active_state->pio_offset + WIZNET_PIO_QSPI_OFFSET_WRITE_BITS_END - 1);
    pio_sm_clear_fifos(active_state->pio, active_state->pio_sm);

    loop_cnt = 2;
    pio_sm_set_pindirs_with_mask(active_state->pio,
                                 active_state->pio_sm,
                                 (1u << active_state->qspi_config->data_io0_pin) | (1u << active_state->qspi_config->data_io1_pin) | (1u << active_state->qspi_config->data_io2_pin) | (1u << active_state->qspi_config->data_io3_pin),
                                 (1u << active_state->qspi_config->data_io0_pin) | (1u << active_state->qspi_config->data_io1_pin) | (1u << active_state->qspi_config->data_io2_pin) | (1u << active_state->qspi_config->data_io3_pin));

    pio_sm_restart(active_state->pio, active_state->pio_sm);
    pio_sm_clkdiv_restart(active_state->pio, active_state->pio_sm);
    pio_sm_put(active_state->pio, active_state->pio_sm, tx_length * loop_cnt - 1);
    pio_sm_exec(active_state->pio, active_state->pio_sm, pio_encode_out(pio_x, 32));
    pio_sm_put(active_state->pio, active_state->pio_sm, 0);
    pio_sm_exec(active_state->pio, active_state->pio_sm, pio_encode_out(pio_y, 32));
    pio_sm_exec(active_state->pio, active_state->pio_sm, pio_encode_jmp(active_state->pio_offset));
    dma_channel_abort(active_state->dma_out);


    dma_channel_config out_config = dma_channel_get_default_config(active_state->dma_out);
    channel_config_set_transfer_data_size(&out_config, DMA_SIZE_8);
    channel_config_set_bswap(&out_config, true);
    channel_config_set_dreq(&out_config, pio_get_dreq(active_state->pio, active_state->pio_sm, true));

    pio_sm_set_enabled(active_state->pio, active_state->pio_sm, true);

    dma_channel_configure(active_state->dma_out, &out_config, &active_state->pio->txf[active_state->pio_sm], command_buf, command_len, true);
    dma_channel_wait_for_finish_blocking(active_state->dma_out);
    dma_channel_configure(active_state->dma_out, &out_config, &active_state->pio->txf[active_state->pio_sm], tx, tx_length - command_len, true);
    dma_channel_wait_for_finish_blocking(active_state->dma_out);

    const uint32_t fdebug_tx_stall = 1u << (PIO_FDEBUG_TXSTALL_LSB + active_state->pio_sm);
    active_state->pio->fdebug = fdebug_tx_stall;
    // pio_sm_set_enabled(active_state->pio, active_state->pio_sm, true);
    while (!(active_state->pio->fdebug & fdebug_tx_stall)) {
        tight_loop_contents(); // todo timeout
    }
    __compiler_memory_barrier();
    pio_sm_set_consecutive_pindirs(active_state->pio, active_state->pio_sm, active_state->qspi_config->data_io0_pin, 4, false);

    pio_sm_exec(active_state->pio, active_state->pio_sm, pio_encode_mov(pio_pins, pio_null));
    pio_sm_set_enabled(active_state->pio, active_state->pio_sm, false);
}

static void wiznet_pio_qspi_set_active(wiznet_pio_qspi_handle_t handle) {
    active_state = (wiznet_pio_qspi_state_t *)handle;
}

static void wiznet_pio_qspi_set_inactive(void) {
    active_state = NULL;
}

static void wizchip_pio_qspi_reset(wiznet_pio_qspi_handle_t handle) {

    wiznet_pio_qspi_state_t *state = (wiznet_pio_qspi_state_t *)handle;
    gpio_set_dir(state->qspi_config->reset_pin, GPIO_OUT);
    gpio_put(state->qspi_config->reset_pin, 0);
    sleep_ms(100);
    gpio_put(state->qspi_config->reset_pin, 1);
    sleep_ms(100);

}

static wiznet_pio_qspi_funcs_t *get_wiznet_pio_qspi_impl(void) {
    static wiznet_pio_qspi_funcs_t funcs = {
        .close = wiznet_pio_qspi_close,
        .set_active = wiznet_pio_qspi_set_active,
        .set_inactive = wiznet_pio_qspi_set_inactive,
        .frame_start = wiznet_pio_qspi_frame_start,
        .frame_end = wiznet_pio_qspi_frame_end,
        .read_byte = wiznet_pio_qspi_read_byte,
        .write_byte = wiznet_pio_qspi_write_byte,
        .reset = wizchip_pio_qspi_reset,
    };
    return &funcs;
}
