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
#ifdef __FREERTOS
        if (!_s_first_lock_created) {
            _s_first_lock_created = true;
            _s_first_lock = xSemaphoreCreateMutex();
        }
        xSemaphoreTake(_s_first_lock, portMAX_DELAY);
#endif
        T* tmp = _s_first;
        _s_first = self;
        self->_next = tmp;
#ifdef __FREERTOS
        xSemaphoreGive(_s_first_lock);
#endif
    }

    static void _remove(T* self) {
#ifdef __FREERTOS
        if (!_s_first_lock_created) {
            _s_first_lock_created = true;
            _s_first_lock = xSemaphoreCreateMutex();
        }
        xSemaphoreTake(_s_first_lock, portMAX_DELAY);
#endif

        if (_s_first == self) {
            _s_first = self->_next;
            self->_next = 0;
#ifdef __FREERTOS
            xSemaphoreGive(_s_first_lock);
#endif
            return;
        }

        for (T* prev = _s_first; prev->_next; prev = prev->_next) {
            if (prev->_next == self) {
                prev->_next = self->_next;
                self->_next = 0;
#ifdef __FREERTOS
                xSemaphoreGive(_s_first_lock);
#endif
                return;
            }
        }
#ifdef __FREERTOS
        xSemaphoreGive(_s_first_lock);
#endif
    }

#ifdef __FREERTOS
    static SemaphoreHandle_t _s_first_lock;
    static bool _s_first_lock_created;
#endif
    static T* _s_first;
    T* _next;
};


#endif //SLIST_H
