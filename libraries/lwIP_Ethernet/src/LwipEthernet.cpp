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
#include <pico/mutex.h>
#include <pico/async_context_threadsafe_background.h>
#include <functional>
#include <list>

bool __lwipInitted = false;

// Async context that pumps the ethernet controllers
static async_context_threadsafe_background_t lwip_ethernet_async_context_threadsafe_background;
static async_when_pending_worker_t always_pending_update_timeout_worker;
static async_at_time_worker_t ethernet_timeout_worker;

// Theoretically support multiple interfaces
static std::list<std::function<void(void)>> _handlePacketList;

void ethernet_arch_lwip_begin() {
    async_context_acquire_lock_blocking(&lwip_ethernet_async_context_threadsafe_background.core);
}

void ethernet_arch_lwip_end() {
    async_context_release_lock(&lwip_ethernet_async_context_threadsafe_background.core);
}

void __addEthernetInterface(std::function<void(void)> _packet) {
    ethernet_arch_lwip_begin();
    _handlePacketList.push_back(_packet);
    ethernet_arch_lwip_end();
}

static async_context_t *lwip_ethernet_init_default_async_context(void) {
    async_context_threadsafe_background_config_t config = async_context_threadsafe_background_default_config();
    if (async_context_threadsafe_background_init(&lwip_ethernet_async_context_threadsafe_background, &config)) {
        return &lwip_ethernet_async_context_threadsafe_background.core;
    }
    return NULL;
}

// This will only be called under the protection of the async context mutex, so no re-entrancy checks needed
static void ethernet_timeout_reached(__unused async_context_t *context, __unused async_at_time_worker_t *worker) {
    assert(worker == &ethernet_timeout_worker);
    for (auto handlePacket : _handlePacketList) {
        handlePacket();
    }
    sys_check_timeouts();
}

static void update_next_timeout(async_context_t *context, async_when_pending_worker_t *worker) {
    assert(worker == &always_pending_update_timeout_worker);
    worker->work_pending = true;
    async_context_add_at_time_worker_in_ms(context, &ethernet_timeout_worker, 50);
}

void __startEthernetContext() {
    async_context_t *context  = lwip_ethernet_init_default_async_context();
    always_pending_update_timeout_worker.work_pending = true;
    always_pending_update_timeout_worker.do_work = update_next_timeout;
    ethernet_timeout_worker.do_work = ethernet_timeout_reached;
    async_context_add_when_pending_worker(context, &always_pending_update_timeout_worker);
}
