/*
    LWIP-on-FreeRTOS Plumbing

    Copyright (c) 2025 Earle F. Philhower, III <earlephilhower@yahoo.com>

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

#ifdef __FREERTOS

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <lwip_wrap.h>
#include "freertos-lwip.h"

// The data structure for the LWIP work queue
typedef struct {
    __lwip_op op;
    void *req;
    TaskHandle_t wakeup;
} LWIPWork;

#define LWIP_WORK_ENTRIES 16

// The notify item we'll use to wake the calling process back up
#define TASK_NOTIFY_LWIP_WAKEUP (configTASK_NOTIFICATION_ARRAY_ENTRIES - 1)

static void lwipThread(void *params);
static TaskHandle_t __lwipTask;
static QueueHandle_t __lwipQueue;

void __startLWIPThread() {
    __lwipQueue = xQueueCreate(LWIP_WORK_ENTRIES, sizeof(LWIPWork));
    if (!__lwipQueue) {
        panic("Unable to allocate LWIP work queue");
    }
    if (pdPASS != xTaskCreate(lwipThread, "LWIP", 1024, 0, configMAX_PRIORITIES - 1, &__lwipTask)) {
        panic("Unable to create LWIP task");
    }
    vTaskCoreAffinitySet(__lwipTask, 1 << 0);
}

extern "C" void __lwip(__lwip_op op, void *req, bool fromISR) {
    LWIPWork w;
    if (fromISR) {
        w.op = op;
        w.req = req;
        w.wakeup = 0; // Don't try and wake up a task when done, we're not in one!
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if (!xQueueSendFromISR(__lwipQueue, &w, &xHigherPriorityTaskWoken)) {
            panic("LWIP task send failed");
        }
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    } else {
        TaskStatus_t t;
        vTaskGetInfo(nullptr, &t, pdFALSE, eInvalid); // TODO - can we speed this up???
        w.op = op;
        w.req = req;
        w.wakeup = t.xHandle;
        if (!xQueueSend(__lwipQueue, &w, portMAX_DELAY)) {
            panic("LWIP task send failed");
        }
        ulTaskNotifyTakeIndexed(TASK_NOTIFY_LWIP_WAKEUP, pdTRUE, portMAX_DELAY);
    }
}

extern "C" bool __isLWIPThread() {
    TaskStatus_t t;
    vTaskGetInfo(nullptr, &t, pdFALSE, eInvalid); // TODO - can we speed this up???
    return t.xHandle == __lwipTask;
}

static void lwipThread(void *params) {
    (void) params;
    LWIPWork w;
    assert(__isLWIPThread());
    unsigned int scd = 100 / portTICK_PERIOD_MS;

    lwip_init(); // Will call our wrapper and set up the RNG

    while (true) {
        auto ret = xQueueReceive(__lwipQueue, &w, scd);
        if (ret) {
            switch (w.op) {
            case __lwip_init: {
                __real_lwip_init();
                break;
            }
            case __pbuf_header: {
                __pbuf_header_req *r = (__pbuf_header_req *)w.req;
                *(r->ret) = __real_pbuf_header(r->p, r->header_size);
                break;
            }
            case __pbuf_free: {
                __pbuf_free_req *r = (__pbuf_free_req *)w.req;
                *(r->ret) = __real_pbuf_free(r->p);
                break;
            }
            case __pbuf_alloc: {
                __pbuf_alloc_req *r = (__pbuf_alloc_req *)w.req;
                *(r->ret) = __real_pbuf_alloc(r->l, r->length, r->type);
                break;
            }
            case __pbuf_take: {
                __pbuf_take_req *r = (__pbuf_take_req *)w.req;
                *(r->ret) = __real_pbuf_take(r->buf, r->dataptr, r->len);
                break;
            }
            case __pbuf_copy_partial: {
                __pbuf_copy_partial_req *r = (__pbuf_copy_partial_req *)w.req;
                *(r->ret) = __real_pbuf_copy_partial(r->p, r->dataptr, r->len, r->offset);
                break;
            }
            case __pbuf_ref: {
                __pbuf_ref_req *r = (__pbuf_ref_req *)w.req;
                __real_pbuf_ref(r->p);
                break;
            }
            case __pbuf_get_at: {
                __pbuf_get_at_req *r = (__pbuf_get_at_req *)w.req;
                *(r->ret) = __real_pbuf_get_at(r->p, r->offset);
                break;
            }
            case __pbuf_get_contiguous: {
                __pbuf_get_contiguous_req *r = (__pbuf_get_contiguous_req *)w.req;
                *(r->ret) = __real_pbuf_get_contiguous(r->p, r->buffer, r->bufsize, r->len, r->offset);
                break;
            }
            case __pbuf_cat: {
                __pbuf_cat_req *r = (__pbuf_cat_req *)w.req;
                __real_pbuf_cat(r->head, r->tail);
                break;
            }
            case __tcp_arg: {
                __tcp_arg_req *r = (__tcp_arg_req *)w.req;
                __real_tcp_arg(r->pcb, r->arg);
                break;
            }
            case __tcp_new: {
                __tcp_new_req *r = (__tcp_new_req *)w.req;
                *(r->ret) = __real_tcp_new();
                break;
            }
            case __tcp_new_ip_type: {
                __tcp_new_ip_type_req *r = (__tcp_new_ip_type_req *)w.req;
                *(r->ret) = __real_tcp_new_ip_type(r->type);
                break;
            }
            case __tcp_bind: {
                __tcp_bind_req *r = (__tcp_bind_req *)w.req;
                *(r->ret) = __real_tcp_bind(r->pcb, r->ipaddr, r->port);
                break;
            }
            case __tcp_bind_netif: {
                __tcp_bind_netif_req *r = (__tcp_bind_netif_req *)w.req;
                *(r->ret) = __real_tcp_bind_netif(r->pcb, r->netif);
                break;
            }
            case __tcp_listen_with_backlog: {
                __tcp_listen_with_backlog_req *r = (__tcp_listen_with_backlog_req *)w.req;
                *(r->ret) = __real_tcp_listen_with_backlog(r->pcb, r->backlog);
                break;
            }
#if 0
            case __tcp_listen_with_backlog_and_err: {
                __tcp_listen_with_backlog_and_err_req *r = (__tcp_listen_with_backlog_and_err_req *)w.req;
                *(r->ret) = __real_tcp_listen_with_backlog_and_err(r->pcb, r->backlog, r->err);
                break;
            }
#endif
            case __tcp_accept: {
                __tcp_accept_req *r = (__tcp_accept_req *)w.req;
                __real_tcp_accept(r->pcb, r->accept);
                break;
            }
            case __tcp_connect: {
                __tcp_connect_req *r = (__tcp_connect_req *)w.req;
                *(r->ret) = __real_tcp_connect(r->pcb, r->ipaddr, r->port, r->connected);
                break;
            }
            case __tcp_write: {
                __tcp_write_req *r = (__tcp_write_req *)w.req;
                *(r->ret) = __real_tcp_write(r->pcb, r->dataptr, r->len, r->apiflags);
                break;
            }
            case __tcp_sent: {
                __tcp_sent_req *r = (__tcp_sent_req *)w.req;
                __real_tcp_sent(r->pcb, r->sent);
                break;
            }
            case __tcp_recv: {
                __tcp_recv_req *r = (__tcp_recv_req *)w.req;
                __real_tcp_recv(r->pcb, r->recv);
                break;
            }
            case __tcp_recved: {
                __tcp_recved_req *r = (__tcp_recved_req *)w.req;
                __real_tcp_recved(r->pcb, r->len);
                break;
            }
            case __tcp_poll: {
                __tcp_poll_req *r = (__tcp_poll_req *)w.req;
                __real_tcp_poll(r->pcb, r->poll, r->interval);
                break;
            }
            case __tcp_close: {
                __tcp_close_req *r = (__tcp_close_req *)w.req;
                *(r->ret) = __real_tcp_close(r->pcb);
                break;
            }
            case __tcp_abort: {
                __tcp_abort_req *r = (__tcp_abort_req *)w.req;
                __real_tcp_abort(r->pcb);
                break;
            }
            case __tcp_err: {
                __tcp_err_req *r = (__tcp_err_req *)w.req;
                __real_tcp_err(r->pcb, r->err);
                break;
            }
            case __tcp_output: {
                __tcp_output_req *r = (__tcp_output_req *)w.req;
                *(r->ret) = __real_tcp_output(r->pcb);
                break;
            }
            case __tcp_setprio: {
                __tcp_setprio_req *r = (__tcp_setprio_req *)w.req;
                __real_tcp_setprio(r->pcb, r->prio);
                break;
            }
            case __tcp_shutdown: {
                __tcp_shutdown_req *r = (__tcp_shutdown_req *)w.req;
                *(r->ret) = __real_tcp_shutdown(r->pcb, r->shut_rx, r->shut_tx);
                break;
            }
            case __tcp_backlog_delayed: {
                __tcp_backlog_delayed_req *r = (__tcp_backlog_delayed_req *)w.req;
                __real_tcp_backlog_delayed(r->pcb);
                break;
            }
            case __tcp_backlog_accepted: {
                __tcp_backlog_accepted_req *r = (__tcp_backlog_accepted_req *)w.req;
                __real_tcp_backlog_accepted(r->pcb);
                break;
            }
            case __udp_new: {
                __udp_new_req *r = (__udp_new_req *)w.req;
                *(r->ret) = __real_udp_new();
                break;
            }
            case __udp_new_ip_type: {
                __udp_new_ip_type_req *r = (__udp_new_ip_type_req *)w.req;
                *(r->ret) = __real_udp_new_ip_type(r->type);
                break;
            }
            case  __udp_remove: {
                __udp_remove_req *r = (__udp_remove_req *)w.req;
                __real_udp_remove(r->pcb);
                break;
            }
            case __udp_bind: {
                __udp_bind_req *r = (__udp_bind_req *)w.req;
                *(r->ret) = __real_udp_bind(r->pcb, r->ipaddr, r->port);
                break;
            }
            case __udp_connect: {
                __udp_connect_req *r = (__udp_connect_req *)w.req;
                *(r->ret) = __real_udp_connect(r->pcb, r->ipaddr, r->port);
                break;
            }
            case __udp_disconnect: {
                __udp_disconnect_req *r = (__udp_disconnect_req *)w.req;
                *(r->ret) = __real_udp_disconnect(r->pcb);
                break;
            }
            case __udp_send: {
                __udp_send_req *r = (__udp_send_req *)w.req;
                *(r->ret) = __real_udp_send(r->pcb, r->p);
                break;
            }
            case __udp_recv: {
                __udp_recv_req *r = (__udp_recv_req *)w.req;
                __real_udp_recv(r->pcb, r->recv, r->recv_arg);
                break;
            }
            case __udp_sendto: {
                __udp_sendto_req *r = (__udp_sendto_req *)w.req;
                *(r->ret) = __real_udp_sendto(r->pcb, r->p, r->dst_ip, r->dst_port);
                break;
            }
            case __udp_sendto_if: {
                __udp_sendto_if_req *r = (__udp_sendto_if_req *)w.req;
                *(r->ret) = __real_udp_sendto_if(r->pcb, r->p, r->dst_ip, r->dst_port, r->netif);
                break;
            }
            case __udp_sendto_if_src: {
                __udp_sendto_if_src_req *r = (__udp_sendto_if_src_req *)w.req;
                *(r->ret) = __real_udp_sendto_if_src(r->pcb, r->p, r->dst_ip, r->dst_port, r->netif, r->src_ip);
                break;
            }
            case __sys_check_timeouts: {
                __real_sys_check_timeouts();
                break;
            }
            case __dns_gethostbyname: {
                __dns_gethostbyname_req *r = (__dns_gethostbyname_req *)w.req;
                *(r->ret) = __real_dns_gethostbyname(r->hostname, r->addr, r->found, r->callback_arg);
                break;
            }
            case __dns_gethostbyname_addrtype: {
                __dns_gethostbyname_addrtype_req *r = (__dns_gethostbyname_addrtype_req *)w.req;
                *(r->ret) = __real_dns_gethostbyname_addrtype(r->hostname, r->addr, r->found, r->callback_arg, r->dns_addrtype);
                break;
            }
            case __raw_new: {
                __raw_new_req *r = (__raw_new_req *)w.req;
                *(r->ret) = __real_raw_new(r->proto);
                break;
            }
            case __raw_new_ip_type: {
                __raw_new_ip_type_req *r = (__raw_new_ip_type_req *)w.req;
                *(r->ret) = __real_raw_new_ip_type(r->type, r->proto);
                break;
            }
            case __raw_connect: {
                __raw_connect_req *r = (__raw_connect_req *)w.req;
                *(r->ret) = __real_raw_connect(r->pcb, r->ipaddr);
                break;
            }
            case __raw_recv: {
                __raw_recv_req *r = (__raw_recv_req *)w.req;
                __real_raw_recv(r->pcb, r->recv, r->recv_arg);
                break;
            }
            case __raw_bind: {
                __raw_bind_req *r = (__raw_bind_req *)w.req;
                *(r->ret) = __real_raw_bind(r->pcb, r->ipaddr);
                break;
            }
            case __raw_sendto: {
                __raw_sendto_req *r = (__raw_sendto_req *)w.req;
                *(r->ret) = __real_raw_sendto(r->pcb, r->p, r->ipaddr);
                break;
            }
            case __raw_send: {
                __raw_send_req *r = (__raw_send_req *)w.req;
                *(r->ret) = __real_raw_send(r->pcb, r->p);
                break;
            }
            case __raw_remove: {
                __raw_remove_req *r = (__raw_remove_req *)w.req;
                __real_raw_remove(r->pcb);
                break;
            }
            case __netif_add: {
                __netif_add_req *r = (__netif_add_req *)w.req;
                *(r->ret) = __real_netif_add(r->netif, r->ipaddr, r->netmask, r->gw, r->state, r->init, r->input);
                break;
            }
            case __netif_remove: {
                __netif_remove_req *r = (__netif_remove_req *)w.req;
                __real_netif_remove(r->netif);
                break;
            }
            case __ethernet_input: {
                __ethernet_input_req *r = (__ethernet_input_req *)w.req;
                printf("__real_ethernet_input\n");
                *(r->ret) = __real_ethernet_input(r->p, r->netif);
                break;
            }
            case __callback: {
                __callback_req *r = (__callback_req *)w.req;
                r->cb(r->cbData);
                break;
            }
            default: {
                // Any new unimplemented calls = ERROR!!!
                panic("Unimplemented LWIP thread action");
                break;
            }
            }
            // Work done, return value set, just tickle the calling task
            if (w.wakeup) {
                xTaskNotifyGiveIndexed(w.wakeup, TASK_NOTIFY_LWIP_WAKEUP);
            }
        } else {
            // No work received, do periodic processing
            __real_sys_check_timeouts();
            // When should we wake up next to redo timeouts?
            scd = sys_timeouts_sleeptime();
            if (scd == SYS_TIMEOUTS_SLEEPTIME_INFINITE) {
                scd = portMAX_DELAY / portTICK_PERIOD_MS;
            }
        }
    }
}

#endif
