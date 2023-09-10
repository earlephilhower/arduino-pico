#include <LwipEthernet.h>
#include <SPI.h>
#include <lwip/init.h>
#include <pico/mutex.h>
#include <pico/async_context_threadsafe_background.h>


// Protection against LWIP re-entrancy
extern "C" {
    auto_init_recursive_mutex(__ethernetMutex); // Only for non-PicoW case

    void ethernet_arch_lwip_begin() {
        recursive_mutex_enter_blocking(&__ethernetMutex);
    }
    void ethernet_arch_lwip_end() {
        recursive_mutex_exit(&__ethernetMutex);
    }
    bool ethernet_arch_lwip_try() {
        uint32_t unused;
        return recursive_mutex_try_enter(&__ethernetMutex, &unused);
    }
};


#include <functional>
#include <list>

// I apologize in advance for the template mess below....
std::list<std::function<void(void)>> _handlePacketList;

void addEthernetInterface(std::function<void(void)> _packet) {
    _handlePacketList.push_back(_packet);
}

// Async context that pumps the ethernet controllers and
static async_context_threadsafe_background_t lwip_ethernet_async_context_threadsafe_background;

async_context_t *lwip_ethernet_init_default_async_context(void) {
    async_context_threadsafe_background_config_t config = async_context_threadsafe_background_default_config();
    if (async_context_threadsafe_background_init(&lwip_ethernet_async_context_threadsafe_background, &config)) {
        return &lwip_ethernet_async_context_threadsafe_background.core;
    }
    return NULL;
}


static void update_next_timeout(async_context_t *context, async_when_pending_worker_t *worker);
static void ethernet_timeout_reached(async_context_t *context, async_at_time_worker_t *worker);

static async_when_pending_worker_t always_pending_update_timeout_worker = {
    .next = 0,
    .do_work = update_next_timeout,
    .work_pending = 0,
    .user_data = 0,
};

static async_at_time_worker_t ethernet_timeout_worker = {
    .next = 0,
    .do_work = ethernet_timeout_reached,
    .next_time = 0,
    .user_data = 0,
};
bool go = false;

static void ethernet_timeout_reached(__unused async_context_t *context, __unused async_at_time_worker_t *worker) {
    assert(worker == &ethernet_timeout_worker);
    if (go) {
        for (auto handlePacket : _handlePacketList) {
            handlePacket();
        }
        if (ethernet_arch_lwip_try()) {
            sys_check_timeouts();
            ethernet_arch_lwip_end();
        }
    }
}


static void update_next_timeout(async_context_t *context, async_when_pending_worker_t *worker) {
    assert(worker == &always_pending_update_timeout_worker);
    worker->work_pending = true;
    uint32_t sleep_ms = sys_timeouts_sleeptime();
    if (sleep_ms == SYS_TIMEOUTS_SLEEPTIME_INFINITE) {
        ethernet_timeout_worker.next_time = at_the_end_of_time;
    } else {
        ethernet_timeout_worker.next_time = make_timeout_time_ms(sleep_ms);
    }
    async_context_add_at_time_worker_in_ms(context, &ethernet_timeout_worker, 50);
}


void ethinit() {
    async_context_t *context  = lwip_ethernet_init_default_async_context();
    always_pending_update_timeout_worker.work_pending = true;
    async_context_add_when_pending_worker(context, &always_pending_update_timeout_worker);
}



//#ifndef ETHERNET_SPI_CLOCK_DIV
//#define ETHERNET_SPI_CLOCK_DIV SPI_CLOCK_DIV4  // 4MHz (SPI.h)
//#endif

void SPI4EthInit() {
    SPI.begin();
    lwip_init();
    go = true;
    //    SPI.setClockDivider(ETHERNET_SPI_CLOCK_DIV);
    //    SPI.setBitOrder(MSBFIRST);
    //    SPI.setDataMode(SPI_MODE0);
}
