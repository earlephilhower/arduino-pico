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
        T** link = &_s_first;
        while (*link && *link != self) {
            link = &(*link)->_next;
        }
        if (*link) {
#ifdef __FREERTOS
            // A virtual stop() can remove the next node of any nested traversal.
            for (Cursor * cursor = _cursors(); cursor; cursor = cursor->previous) {
                if (cursor->next == self) {
                    cursor->next = self->_next;
                }
            }
#endif
            *link = self->_next;
            self->_next = 0;
        }
        _unlock();
    }

    template<typename F>
    static void _forEach(F fn) {
#ifdef __FREERTOS
        _lock();
        Cursor cursor = {_s_first, _cursors()};
        _cursors() = &cursor;
        while (cursor.next) {
            T* it = cursor.next;
            cursor.next = it->_next;
            fn(it);
        }
        _cursors() = cursor.previous;
        _unlock();
#else
        for (T * it = _s_first; it; it = it->_next) {
            fn(it);
        }
#endif
    }

#ifdef __FREERTOS
    struct Cursor {
        T* next;
        Cursor* previous;
    };

    static Cursor*& _cursors() {
        static Cursor* cursor;
        return cursor;
    }

    // Created on first use so global constructors work before the scheduler; the critical section makes creation one-time across tasks and cores
    static SemaphoreHandle_t _mutex() {
        static StaticSemaphore_t buf;
        static SemaphoreHandle_t mutex;
        taskENTER_CRITICAL();
        if (!mutex) {
            mutex = xSemaphoreCreateRecursiveMutexStatic(&buf);
        }
        taskEXIT_CRITICAL();
        return mutex;
    }
#endif

    static void _lock() {
#ifdef __FREERTOS
        xSemaphoreTakeRecursive(_mutex(), portMAX_DELAY);
#endif
    }

    static void _unlock() {
#ifdef __FREERTOS
        xSemaphoreGiveRecursive(_mutex());
#endif
    }

    static T* _s_first;
    T* _next;
};


#endif //SLIST_H
