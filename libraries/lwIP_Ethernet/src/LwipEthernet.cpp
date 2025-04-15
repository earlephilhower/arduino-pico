/*
    LwipEthernet.cpp

    Handles the async context for wired Ethernet

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <LwipEthernet.h>
#include <lwip/timeouts.h>
#include <lwip/dns.h>
#include <pico/mutex.h>
#if defined(PICO_CYW43_SUPPORTED)
#include <pico/cyw43_arch.h>
#endif
#include <pico/async_context_threadsafe_background.h>
#include <functional>
#include <map>

bool __ethernetContextInitted = false;

// Async context that pumps the ethernet controllers
static async_context_threadsafe_background_t lwip_ethernet_async_context_threadsafe_background;
static async_when_pending_worker_t always_pending_update_timeout_worker;
static async_at_time_worker_t ethernet_timeout_worker;
static async_context_t *_context = nullptr;

// Theoretically support multiple interfaces
static std::map<int, std::function<void(void)>> _handlePacketList;

void ethernet_arch_lwip_begin() {
#if defined(PICO_CYW43_SUPPORTED)
    if (rp2040.isPicoW()) {
        cyw43_arch_lwip_begin();
        return;
    }
#endif
    async_context_acquire_lock_blocking(_context);
}

void ethernet_arch_lwip_end() {
#if defined(PICO_CYW43_SUPPORTED)
    if (rp2040.isPicoW()) {
        cyw43_arch_lwip_end();
        return;
    }
#endif
    async_context_release_lock(_context);
}

int __addEthernetPacketHandler(std::function<void(void)> _packetHandler) {
    static int id = 0xdead;
    ethernet_arch_lwip_begin();
    _handlePacketList.insert({id, _packetHandler});
    ethernet_arch_lwip_end();
    return id++;
}

void __removeEthernetPacketHandler(int id) {
    ethernet_arch_lwip_begin();
    _handlePacketList.erase(id);
    ethernet_arch_lwip_end();
}

#define GPIOSTACKSIZE 8
#if defined(PICO_RP2350) && !PICO_RP2350A // RP2350B
#define GPIOIRQREGS 6
#define GPIOIRQREGSINIT 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
#else
#define GPIOIRQREGS 4
#define GPIOIRQREGSINIT 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
#endif

static uint32_t gpioMaskStack[GPIOSTACKSIZE][GPIOIRQREGS];
static uint32_t gpioMask[GPIOIRQREGS] = {GPIOIRQREGSINIT};

void ethernet_arch_lwip_gpio_mask() {
    noInterrupts();
    memmove(gpioMaskStack[1], gpioMaskStack[0], GPIOIRQREGS * sizeof(uint32_t) * (GPIOSTACKSIZE - 1)); // Push down the stack
    io_bank0_irq_ctrl_hw_t *irq_ctrl_base = get_core_num() ? &io_bank0_hw->proc1_irq_ctrl : &io_bank0_hw->proc0_irq_ctrl;
    for (int i = 0; i < GPIOIRQREGS; i++) {
        gpioMaskStack[0][i] = irq_ctrl_base->inte[i];
        irq_ctrl_base->inte[i] = irq_ctrl_base->inte[i] & gpioMask[i];
    }
    interrupts();
}

void ethernet_arch_lwip_gpio_unmask() {
    noInterrupts();
    io_bank0_irq_ctrl_hw_t *irq_ctrl_base = get_core_num() ? &io_bank0_hw->proc1_irq_ctrl : &io_bank0_hw->proc0_irq_ctrl;
    for (int i = 0; i < GPIOIRQREGS; i++) {
        irq_ctrl_base->inte[i] = gpioMaskStack[0][i];
    }
    memmove(gpioMaskStack[0], gpioMaskStack[1],  GPIOIRQREGS * sizeof(uint32_t) * (GPIOSTACKSIZE - 1)); // Pop up the stack
    interrupts();
}

// To be called after IRQ is set, so we can just rad from the IOREG instead of duplicating the calculation
void __addEthernetGPIO(int pin) {
    int idx = pin / 8;
    int off = (pin % 8) * 4;
    gpioMask[idx] &= ~(0xf << off);
}

void __removeEthernetGPIO(int pin) {
    int idx = pin / 8;
    int off = (pin % 8) * 4;
    gpioMask[idx] |= 0xf << off;
}


static volatile bool _dns_lookup_pending = false;

static void _dns_found_callback(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    (void) name;
    if (!_dns_lookup_pending) {
        return;
    }
    if (ipaddr) {
        *(IPAddress *)callback_arg = IPAddress(ipaddr);
    }
    _dns_lookup_pending = false; // resume hostByName
}

int hostByName(const char* aHostname, IPAddress& aResult, int timeout_ms) {
    ip_addr_t addr;
    aResult = static_cast<uint32_t>(0xffffffff);

    if (aResult.fromString(aHostname)) {
        // Host name is a IP address use it!
        return 1;
    }

#if LWIP_IPV4 && LWIP_IPV6
    err_t err = dns_gethostbyname_addrtype(aHostname, &addr, &_dns_found_callback, &aResult, LWIP_DNS_ADDRTYPE_DEFAULT);
#else
    err_t err = dns_gethostbyname(aHostname, &addr, &_dns_found_callback, &aResult);
#endif
    if (err == ERR_OK) {
        aResult = IPAddress(&addr);
    } else if (err == ERR_INPROGRESS) {
        _dns_lookup_pending = true;
        uint32_t now = millis();
        while ((millis() - now < (uint32_t)timeout_ms) && _dns_lookup_pending) {
            delay(1);
        }
        _dns_lookup_pending = false;
        if (aResult.isSet()) {
            err = ERR_OK;
        }
    }

    if (err == ERR_OK) {
        return 1;
    }

    return 0;
}

static async_context_t *lwip_ethernet_init_default_async_context(void) {
    async_context_threadsafe_background_config_t config = async_context_threadsafe_background_default_config();
    if (async_context_threadsafe_background_init(&lwip_ethernet_async_context_threadsafe_background, &config)) {
        return &lwip_ethernet_async_context_threadsafe_background.core;
    }
    return NULL;
}

uint32_t __ethernet_timeout_reached_calls = 0;
static uint32_t _pollingPeriod = 20;
// This will only be called under the protection of the async context mutex, so no re-entrancy checks needed
static void ethernet_timeout_reached(__unused async_context_t *context, __unused async_at_time_worker_t *worker) {
    assert(worker == &ethernet_timeout_worker);
    __ethernet_timeout_reached_calls++;
    ethernet_arch_lwip_gpio_mask(); // Ensure non-polled devices won't interrupt us
    for (auto handlePacket : _handlePacketList) {
        handlePacket.second();
    }
#if defined(PICO_CYW43_SUPPORTED)
    if (!rp2040.isPicoW()) {
        sys_check_timeouts();
    }
#else
    sys_check_timeouts();
#endif
    ethernet_arch_lwip_gpio_unmask();
}

static void update_next_timeout(async_context_t *context, async_when_pending_worker_t *worker) {
    assert(worker == &always_pending_update_timeout_worker);
    worker->work_pending = true;
    async_context_add_at_time_worker_in_ms(context, &ethernet_timeout_worker, _pollingPeriod);
}

void __startEthernetContext() {
    if (__ethernetContextInitted) {
        return;
    }
#if defined(PICO_CYW43_SUPPORTED)
    if (rp2040.isPicoW()) {
        _context = cyw43_arch_async_context();
    } else {
        _context = lwip_ethernet_init_default_async_context();
    }
#else
    _context = lwip_ethernet_init_default_async_context();
#endif
    ethernet_timeout_worker.do_work = ethernet_timeout_reached;
    always_pending_update_timeout_worker.work_pending = true;
    always_pending_update_timeout_worker.do_work = update_next_timeout;
    async_context_add_when_pending_worker(_context, &always_pending_update_timeout_worker);
    __ethernetContextInitted = true;
}

void lwipPollingPeriod(int ms) {
    if (ms > 0) {
        // No need for mutexes, this is an atomic 32b write
        _pollingPeriod = ms;
    }
}

std::function<void(struct netif *)>  _scb;
void __setStateChangeCallback(std::function<void(struct netif *)> s) {
    _scb = s;
}
