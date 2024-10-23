/*
    Copyright (c) 2024, WIZnet Co., Ltd.
    Copyright (c) 2016, Nicholas Humfrey
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
    COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
    OF THE POSSIBILITY OF SUCH DAMAGE.

*/

// original sources: https://github.com/njh/W5500MacRaw

#include <SPI.h>
#include "wiznet_pio_spi.h"
#include "w55rp20.h"
#include <LwipEthernet.h>

wiznet_pio_spi_config_t wiznet_pio_spi_config;
wiznet_pio_spi_handle_t wiznet_pio_spi_handle = NULL;

uint8_t Wiznet55rp20::wizchip_read(uint8_t block, uint16_t address) {
    uint8_t ret;
    uint8_t spi_data[3];

    wizchip_cs_select();

    block |= AccessModeRead;

	spi_data[0] = (address & 0x00FF00) >> 8;
	spi_data[1] = (address & 0x0000FF) >> 0;
	spi_data[2] = block;
    (*wiznet_pio_spi_handle)->write_buffer(spi_data, 3);
    ret = (*wiznet_pio_spi_handle)->read_byte();

    wizchip_cs_deselect();
    return ret;
}

uint16_t Wiznet55rp20::wizchip_read_word(uint8_t block, uint16_t address) {
    return ((uint16_t)wizchip_read(block, address) << 8) + wizchip_read(block, address + 1);
}

void Wiznet55rp20::wizchip_read_buf(uint8_t block, uint16_t address, uint8_t* pBuf, uint16_t len) {
    uint16_t i;
    uint8_t spi_data[3];

    wizchip_cs_select();

    block |= AccessModeRead;

	spi_data[0] = (address & 0x00FF00) >> 8;
	spi_data[1] = (address & 0x0000FF) >> 0;
	spi_data[2] = block;
    (*wiznet_pio_spi_handle)->write_buffer(spi_data, 3);
    (*wiznet_pio_spi_handle)->read_buffer(pBuf, len);

    wizchip_cs_deselect();
}

void Wiznet55rp20::wizchip_write(uint8_t block, uint16_t address, uint8_t wb) {
    uint8_t spi_data[4];

    wizchip_cs_select();

    block |= AccessModeWrite;

    spi_data[0] = (address & 0x00FF00) >> 8;
	spi_data[1] = (address & 0x0000FF) >> 0;
	spi_data[2] = block;
    spi_data[3] = wb;
    (*wiznet_pio_spi_handle)->write_buffer(spi_data, 4);

    wizchip_cs_deselect();
}

void Wiznet55rp20::wizchip_write_word(uint8_t block, uint16_t address, uint16_t word) {
    wizchip_write(block, address, (uint8_t)(word >> 8));
    wizchip_write(block, address + 1, (uint8_t)word);
}

void Wiznet55rp20::wizchip_write_buf(uint8_t block, uint16_t address, const uint8_t* pBuf,
                                   uint16_t len) {
    uint16_t i;
    uint8_t spi_data[3];

    wizchip_cs_select();

    block |= AccessModeWrite;

    spi_data[0] = (address & 0x00FF00) >> 8;
	spi_data[1] = (address & 0x0000FF);
	spi_data[2] = block;
	(*wiznet_pio_spi_handle)->write_buffer(spi_data, 3);
	(*wiznet_pio_spi_handle)->write_buffer(pBuf, len);

    wizchip_cs_deselect();
}

void Wiznet55rp20::setSn_CR(uint8_t cr) {
    // Write the command to the Command Register
    wizchip_write(BlockSelectSReg, Sn_CR, cr);

    // Now wait for the command to complete
    while (wizchip_read(BlockSelectSReg, Sn_CR))
        ;
}

uint16_t Wiznet55rp20::getSn_TX_FSR() {
    uint16_t val = 0, val1 = 0;
    do {
        val1 = wizchip_read_word(BlockSelectSReg, Sn_TX_FSR);
        if (val1 != 0) {
            val = wizchip_read_word(BlockSelectSReg, Sn_TX_FSR);
        }
    } while (val != val1);
    return val;
}

uint16_t Wiznet55rp20::getSn_RX_RSR() {
    uint16_t val = 0, val1 = 0;
    do {
        val1 = wizchip_read_word(BlockSelectSReg, Sn_RX_RSR);
        if (val1 != 0) {
            val = wizchip_read_word(BlockSelectSReg, Sn_RX_RSR);
        }
    } while (val != val1);
    return val;
}

void Wiznet55rp20::wizchip_send_data(const uint8_t* wizdata, uint16_t len) {
    uint16_t ptr = 0;

    if (len == 0) {
        return;
    }
    ptr = getSn_TX_WR();
    wizchip_write_buf(BlockSelectTxBuf, ptr, wizdata, len);

    ptr += len;

    setSn_TX_WR(ptr);
}

void Wiznet55rp20::wizchip_recv_data(uint8_t* wizdata, uint16_t len) {
    uint16_t ptr;

    if (len == 0) {
        return;
    }
    ptr = getSn_RX_RD();
    wizchip_read_buf(BlockSelectRxBuf, ptr, wizdata, len);
    ptr += len;

    setSn_RX_RD(ptr);
}

void Wiznet55rp20::wizchip_recv_ignore(uint16_t len) {
    uint16_t ptr;

    ptr = getSn_RX_RD();
    ptr += len;
    setSn_RX_RD(ptr);
}

void Wiznet55rp20::wizchip_sw_reset() {
    setMR(MR_RST);
    getMR();  // for delay

    setSHAR(_mac_address);
}

int8_t Wiznet55rp20::wizphy_getphylink() {
    int8_t tmp;
    if (getPHYCFGR() & PHYCFGR_LNK_ON) {
        tmp = PHY_LINK_ON;
    } else {
        tmp = PHY_LINK_OFF;
    }
    return tmp;
}

int8_t Wiznet55rp20::wizphy_getphypmode() {
    int8_t tmp = 0;
    if (getPHYCFGR() & PHYCFGR_OPMDC_PDOWN) {
        tmp = PHY_POWER_DOWN;
    } else {
        tmp = PHY_POWER_NORM;
    }
    return tmp;
}

void Wiznet55rp20::wizphy_reset() {
    uint8_t tmp = getPHYCFGR();
    tmp &= PHYCFGR_RST;
    setPHYCFGR(tmp);
    tmp = getPHYCFGR();
    tmp |= ~PHYCFGR_RST;
    setPHYCFGR(tmp);
}

int8_t Wiznet55rp20::wizphy_setphypmode(uint8_t pmode) {
    uint8_t tmp = 0;
    tmp         = getPHYCFGR();
    if ((tmp & PHYCFGR_OPMD) == 0) {
        return -1;
    }
    tmp &= ~PHYCFGR_OPMDC_ALLA;
    if (pmode == PHY_POWER_DOWN) {
        tmp |= PHYCFGR_OPMDC_PDOWN;
    } else {
        tmp |= PHYCFGR_OPMDC_ALLA;
    }
    setPHYCFGR(tmp);
    wizphy_reset();
    tmp = getPHYCFGR();
    if (pmode == PHY_POWER_DOWN) {
        if (tmp & PHYCFGR_OPMDC_PDOWN) {
            return 0;
        }
    } else {
        if (tmp & PHYCFGR_OPMDC_ALLA) {
            return 0;
        }
    }
    return -1;
}

Wiznet55rp20::Wiznet55rp20(int8_t cs, SPIClass& spi, int8_t intr) : _spi(spi), _cs(cs), _intr(intr) {
}

bool Wiznet55rp20::begin(const uint8_t* mac_address, netif *net) {
    wiznet_pio_spi_config.clock_div_major = 4;
    wiznet_pio_spi_config.clock_div_minor = 0;
    wiznet_pio_spi_config.data_in_pin = WIZNET_PIO_SPI_MISO_PIN;
    wiznet_pio_spi_config.data_out_pin = WIZNET_PIO_SPI_MOSI_PIN;
    wiznet_pio_spi_config.clock_pin = WIZNET_PIO_SPI_SCK_PIN;

    if (wiznet_pio_spi_handle != NULL)
        wiznet_pio_spi_close(wiznet_pio_spi_handle);
    wiznet_pio_spi_handle = wiznet_pio_spi_open(&wiznet_pio_spi_config);
    (*wiznet_pio_spi_handle)->set_active(wiznet_pio_spi_handle);

    _netif = net;
    memcpy(_mac_address, mac_address, 6);

    pinMode(WIZNET_PIO_SPI_CS_PIN, OUTPUT);
    wizchip_cs_deselect();
    
    wizchip_sw_reset();

    // Use the full 16Kb of RAM for Socket 0
    setSn_RXBUF_SIZE(16);
    setSn_TXBUF_SIZE(16);
    
    // Set our local MAC address
    setSHAR(_mac_address);

    // Open Socket 0 in MACRaw mode
    setSn_MR(Sn_MR_MACRAW);
    setSn_CR(Sn_CR_OPEN);
    if (getSn_SR() != SOCK_MACRAW) {
        // Failed to put socket 0 into MACRaw mode
        return false;
    }

    if (_intr >= 0) {
        setSn_IR(0xff); // Clear everything
        setSIMR(1);
    }

    // Success
    return true;
}

void Wiznet55rp20::end() {
    setSn_CR(Sn_CR_CLOSE);

    // clear all interrupt of the socket
    setSn_IR(0xFF);

    // Wait for socket to change to closed
    while (getSn_SR() != SOCK_CLOSED)
        ;
}

uint16_t Wiznet55rp20::readFrame(uint8_t* buffer, uint16_t bufsize) {
    uint16_t data_len = readFrameSize();

    if (data_len == 0) {
        return 0;
    }

    if (data_len > bufsize) {
        // Packet is bigger than buffer - drop the packet
        discardFrame(data_len);
        return 0;
    }

    return readFrameData(buffer, data_len);
}

uint16_t Wiznet55rp20::readFrameSize() {
    setSn_IR(Sn_IR_RECV);

    uint16_t len = getSn_RX_RSR();

    if (len == 0) {
        return 0;
    }

    uint8_t  head[2];
    uint16_t data_len = 0;

    wizchip_recv_data(head, 2);
    setSn_CR(Sn_CR_RECV);

    data_len = head[0];
    data_len = (data_len << 8) + head[1];
    data_len -= 2;

    return data_len;
}

void Wiznet55rp20::discardFrame(uint16_t framesize) {
    wizchip_recv_ignore(framesize);
    setSn_CR(Sn_CR_RECV);
}

uint16_t Wiznet55rp20::readFrameData(uint8_t* buffer, uint16_t framesize) {
    wizchip_recv_data(buffer, framesize);
    setSn_CR(Sn_CR_RECV);

    // let lwIP deal with mac address filtering
    return framesize;
}

uint16_t Wiznet55rp20::sendFrame(const uint8_t* buf, uint16_t len) {
    ethernet_arch_lwip_gpio_mask(); // So we don't fire an IRQ and interrupt the send w/a receive!

    // Wait for space in the transmit buffer
    while (1) {
        uint16_t freesize = getSn_TX_FSR();
        if (getSn_SR() == SOCK_CLOSED) {
            ethernet_arch_lwip_gpio_unmask();
            return -1;
        }
        if (len <= freesize) {
            break;
        }
    };

    wizchip_send_data(buf, len);
    setSn_CR(Sn_CR_SEND);

    while (1) {
        uint8_t tmp = getSn_IR();
        if (tmp & Sn_IR_SENDOK) {
            setSn_IR(Sn_IR_SENDOK);
            // Packet sent ok
            break;
        } else if (tmp & Sn_IR_TIMEOUT) {
            setSn_IR(Sn_IR_TIMEOUT);
            // There was a timeout
            ethernet_arch_lwip_gpio_unmask();
            return -1;
        }
    }

    ethernet_arch_lwip_gpio_unmask();
    return len;
}
