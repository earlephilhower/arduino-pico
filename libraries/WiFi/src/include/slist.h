#ifndef SLIST_H
#define SLIST_H

#ifdef __FREERTOS
#include "FreeRTOS.h"
#include "semphr.h"
#endif

template<typename T>
class SList {
public:
    SList() : _next(0) { }

protected:

    static void _add(T* self) {
        _lock();
        T* tmp = _s_first;
        _s_first = self;
        self->_next = tmp;
        _unlock();
    }

    static void _remove(T* self) {
        _lock();

        if (_s_first == self) {
            _s_first = self->_next;
            self->_next = 0;
            _unlock();
            return;
        }

        for (T* prev = _s_first; prev->_next; prev = prev->_next) {
            if (prev->_next == self) {
                prev->_next = self->_next;
                self->_next = 0;
                _unlock();
                return;
            }
        }
        _unlock();
    }

#ifdef __FREERTOS
    // Created on first use so global constructors work before the scheduler; the critical section makes creation one-time across tasks and cores
    static SemaphoreHandle_t _mutex() {
        static StaticSemaphore_t buf;
        static SemaphoreHandle_t mutex;
        taskENTER_CRITICAL();
        if (!mutex) {
            mutex = xSemaphoreCreateMutexStatic(&buf);
        }
        taskEXIT_CRITICAL();
        return mutex;
    }
#endif

    static void _lock() {
#ifdef __FREERTOS
        xSemaphoreTake(_mutex(), portMAX_DELAY);
#endif
    }

    static void _unlock() {
#ifdef __FREERTOS
        xSemaphoreGive(_mutex());
#endif
    }

    static T* _s_first;
    T* _next;
};


#endif //SLIST_H
