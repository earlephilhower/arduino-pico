/*
    Copyright (c) 2013, WIZnet Co., Ltd.
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
#include "w6100.h"
#include <LwipEthernet.h>

/**
    Check physical link
    @return true when physical link is up
*/
bool Wiznet6100::isLinked()
{
    ethernet_arch_lwip_gpio_mask();
    ethernet_arch_lwip_begin();
    auto ret = wizphy_getphylink() == PHY_LINK_ON;
    ethernet_arch_lwip_end();
    ethernet_arch_lwip_gpio_unmask();
    return ret;
}


uint8_t Wiznet6100::wizchip_read(uint8_t block, uint16_t address) {
    uint8_t ret;

    wizchip_cs_select();

    block |= AccessModeRead;

    wizchip_spi_write_byte((address & 0xFF00) >> 8);
    wizchip_spi_write_byte((address & 0x00FF) >> 0);
    wizchip_spi_write_byte(block);

    ret = wizchip_spi_read_byte();

    wizchip_cs_deselect();
    return ret;
}

uint16_t Wiznet6100::wizchip_read_word(uint8_t block, uint16_t address) {
    return ((uint16_t)wizchip_read(block, address) << 8) + wizchip_read(block, address + 1);
}

void Wiznet6100::wizchip_read_buf(uint8_t block, uint16_t address, uint8_t* pBuf, uint16_t len) {
    uint16_t i;

    wizchip_cs_select();

    block |= AccessModeRead;

    wizchip_spi_write_byte((address & 0xFF00) >> 8);
    wizchip_spi_write_byte((address & 0x00FF) >> 0);
    wizchip_spi_write_byte(block);
    for (i = 0; i < len; i++) {
        pBuf[i] = wizchip_spi_read_byte();
    }

    wizchip_cs_deselect();
}

void Wiznet6100::wizchip_write(uint8_t block, uint16_t address, uint8_t wb) {
    wizchip_cs_select();

    block |= AccessModeWrite | FixedDataMode1;

    wizchip_spi_write_byte((address & 0xFF00) >> 8);
    wizchip_spi_write_byte((address & 0x00FF) >> 0);
    wizchip_spi_write_byte(block);
    wizchip_spi_write_byte(wb);

    wizchip_cs_deselect();
}

void Wiznet6100::wizchip_write_word(uint8_t block, uint16_t address, uint16_t word) {
    wizchip_write(block, address, (uint8_t)(word >> 8));
    wizchip_write(block, address + 1, (uint8_t)word);
}

void Wiznet6100::wizchip_write_buf(uint8_t block, uint16_t address, const uint8_t* pBuf,
                                   uint16_t len) {
    uint16_t i;

    wizchip_cs_select();

    block |= AccessModeWrite;

    wizchip_spi_write_byte((address & 0xFF00) >> 8);
    wizchip_spi_write_byte((address & 0x00FF) >> 0);
    wizchip_spi_write_byte(block);
    for (i = 0; i < len; i++) {
        wizchip_spi_write_byte(pBuf[i]);
    }

    wizchip_cs_deselect();
}

void Wiznet6100::setSn_CR(uint8_t cr) {
    // Write the command to the Command Register
    wizchip_write(BlockSelectSReg, Sn_CR, cr);

    // Now wait for the command to complete
    while (wizchip_read(BlockSelectSReg, Sn_CR))
        ;
}

uint16_t Wiznet6100::getSn_TX_FSR() {
    uint16_t val = 0, val1 = 0;
    do {
        val1 = wizchip_read_word(BlockSelectSReg, Sn_TX_FSR);
        if (val1 != 0) {
            val = wizchip_read_word(BlockSelectSReg, Sn_TX_FSR);
        }
    } while (val != val1);
    return val;
}

uint16_t Wiznet6100::getSn_RX_RSR() {
    uint16_t val = 0, val1 = 0;
    do {
        val1 = wizchip_read_word(BlockSelectSReg, Sn_RX_RSR);
        if (val1 != 0) {
            val = wizchip_read_word(BlockSelectSReg, Sn_RX_RSR);
        }
    } while (val != val1);
    return val;
}

void Wiznet6100::wizchip_send_data(const uint8_t* wizdata, uint16_t len) {
    uint16_t ptr = 0;

    if (len == 0) {
        return;
    }
    ptr = getSn_TX_WR();
    wizchip_write_buf(BlockSelectTxBuf, ptr, wizdata, len);

    ptr += len;

    setSn_TX_WR(ptr);
}

void Wiznet6100::wizchip_recv_data(uint8_t* wizdata, uint16_t len) {
    uint16_t ptr;

    if (len == 0) {
        return;
    }
    ptr = getSn_RX_RD();
    wizchip_read_buf(BlockSelectRxBuf, ptr, wizdata, len);
    ptr += len;

    setSn_RX_RD(ptr);
}

void Wiznet6100::wizchip_recv_ignore(uint16_t len) {
    uint16_t ptr;

    ptr = getSn_RX_RD();
    ptr += len;
    setSn_RX_RD(ptr);
}

bool Wiznet6100::wizchip_sw_reset() {
    setChipLOCK(CHPLCKR_UNLOCK);
    uint16_t count = 0;
    do
    { // Wait Unlock Complete
        if (++count > 20)
        {             // Check retry count
            return false; // Over Limit retry count
        }
    } while ((getStatus() & SYSR_CHPL_LOCK) ^ SYSR_CHPL_ULOCK); // Exit Wait Unlock Complete

    setCommand0(0x0); // Software Reset

    do
    { // Wait Lock Complete
        if (++count > 20)
        {             // Check retry count
            return false; // Over Limit retry count
        }

    } while ((getStatus() & SYSR_CHPL_LOCK) ^ SYSR_CHPL_LOCK); // Exit Wait Lock Complete

    return true;
}

int8_t Wiznet6100::wizphy_getphylink() {
    int8_t tmp;
    if (getPHYCFGR() & PHYCFGR_LNK_ON) {
        tmp = PHY_LINK_ON;
    } else {
        tmp = PHY_LINK_OFF;
    }
    return tmp;
}

int8_t Wiznet6100::wizphy_getphypmode() {
    int8_t tmp = 0;
    if (getPHYCFGR() & PHYCFGR_OPMDC_PDOWN) {
        tmp = PHY_POWER_DOWN;
    } else {
        tmp = PHY_POWER_NORM;
    }
    return tmp;
}

void Wiznet6100::wizphy_reset() {
    uint8_t tmp = getPHYCFGR();
    tmp &= PHYCFGR_RST;
    setPHYCFGR(tmp);
    tmp = getPHYCFGR();
    tmp |= ~PHYCFGR_RST;
    setPHYCFGR(tmp);
}

int8_t Wiznet6100::wizphy_setphypmode(uint8_t pmode) {
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

Wiznet6100::Wiznet6100(int8_t cs, SPIClass &spi, int8_t intr) : _spi(spi), _cs(cs), _intr(intr)
{
}

bool Wiznet6100::begin(const uint8_t* mac_address, netif *net) {
    _netif = net;
    memcpy(_mac_address, mac_address, 6);

    pinMode(_cs, OUTPUT);
    wizchip_cs_deselect();

    wizchip_sw_reset();

    // Unlock
    setChipLOCK(CHPLCKR_UNLOCK);
    setNetLOCK(NETLCKR_UNLOCK);
    setPHYLOCK(PHYLCKR_UNLOCK);

    // W6100 CIDR0
    // Version 97(dec) 0x61(hex)
    int ver = getVERSIONR();

    Serial.print("version = 0x");
    Serial.println(ver, HEX);

    if (ver != 0x61) {
        return false;
    }

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

    Serial.println("MAC RAW mode!");

    if (_intr >= 0) {
        
        setSn_IR(0xff); // Clear everything
        

        // Configure socket 0 interrupts
        setSn_IMR(Sn_IMR_RECV); // we're not interested in Sn_IMR_SENDOK atm

        // Enable socket 0 interrupts
        setSIMR(SIMR_S0_INT);

        // Disable unused interrupts
        setIMR(0);

        // Enable interrupt pin
        setCommand1(SYCR1_IEN);
        
    }

    // Success
    return true;
}

void Wiznet6100::end() {
    setSn_CR(Sn_CR_CLOSE);

    // clear all interrupt of the socket
    setSn_IR(0xFF);

    // Wait for socket to change to closed
    while (getSn_SR() != SOCK_CLOSED)
        ;
}

/*
uint16_t Wiznet6100::readFrame(uint8_t* buffer, uint16_t bufsize) {
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
*/

uint16_t Wiznet6100::readFrameSize() {
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

    // Clear interrupt flags
    setICLR(Sn_IRCLR_RECV);

    return data_len;
}

void Wiznet6100::discardFrame(uint16_t framesize) {
    wizchip_recv_ignore(framesize);
    setSn_CR(Sn_CR_RECV);
}

uint16_t Wiznet6100::readFrameData(uint8_t* buffer, uint16_t framesize) {
    wizchip_recv_data(buffer, framesize);
    setSn_CR(Sn_CR_RECV);

    // let lwIP deal with mac address filtering
    return framesize;
}

uint16_t Wiznet6100::sendFrame(const uint8_t* buf, uint16_t len) {
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
