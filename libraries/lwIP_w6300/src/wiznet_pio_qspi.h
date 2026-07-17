/*
    Copyright (c) 2023 Raspberry Pi (Trading) Ltd.

    SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef _WIZNET_SPI_PIO_H_
#define _WIZNET_SPI_PIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct wiznet_pio_qspi_config {
    uint16_t clock_div_major;
    uint8_t clock_div_minor;
    uint8_t clock_pin;
    uint8_t data_io0_pin;
    uint8_t data_io1_pin;
    uint8_t data_io2_pin;
    uint8_t data_io3_pin;
    uint8_t cs_pin;
    uint8_t reset_pin;
    uint8_t irq_pin;
} wiznet_pio_qspi_config_t;

typedef struct wiznet_pio_qspi_funcs** wiznet_pio_qspi_handle_t;

typedef struct wiznet_pio_qspi_funcs {
    void (*close)(wiznet_pio_qspi_handle_t funcs);
    void (*set_active)(wiznet_pio_qspi_handle_t funcs);
    void (*set_inactive)(void);
    void (*frame_start)(void);
    void (*frame_end)(void);
    void (*read_byte)(uint8_t opcode, uint16_t addr, uint8_t* pBuf, uint16_t len);
    void (*write_byte)(uint8_t opcode, uint16_t addr, uint8_t* pBuf, uint16_t len);
    void (*read_buffer)(uint8_t *pBuf, uint16_t len);
    void (*write_buffer)(uint8_t *pBuf, uint16_t len);
    void (*reset)(wiznet_pio_qspi_handle_t funcs);
} wiznet_pio_qspi_funcs_t;

wiznet_pio_qspi_handle_t wiznet_pio_qspi_open(const wiznet_pio_qspi_config_t *qspi_config);
void wiznet_pio_qspi_close(wiznet_pio_qspi_handle_t handle);
void wiznet_pio_qspi_frame_start(void);
void wiznet_pio_qspi_frame_end(void);

#ifdef __cplusplus
}
#endif

#endif
