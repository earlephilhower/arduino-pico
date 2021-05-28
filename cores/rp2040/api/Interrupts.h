#ifndef W_INTERRUPTS_CPP
#define W_INTERRUPTS_CPP
#ifdef __cplusplus

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "Common.h"

namespace arduino {

template <typename T>
using voidTemplateFuncPtrParam =  void (*)(T param);

template<typename T> struct __container__ {
  void* param;
  voidTemplateFuncPtrParam<T> function;
};

// C++ only overloaded version of attachInterrupt function
template<typename T> void attachInterrupt(pin_size_t interruptNum, voidTemplateFuncPtrParam<T> userFunc, PinStatus mode, T& param) {

  struct __container__<T> *cont = new __container__<T>();
  cont->param = &param;
  cont->function = userFunc;

  // TODO: check lambda scope
  // TODO: add structure to delete(__container__) when detachInterrupt() is called
  auto f = [](void* a) -> void
  {
    T param = *(T*)((struct __container__<T>*)a)->param;
    (((struct __container__<T>*)a)->function)(param);
  };

  attachInterruptParam(interruptNum, f, mode, cont);
}

template<typename T> void attachInterrupt(pin_size_t interruptNum, voidTemplateFuncPtrParam<T*> userFunc, PinStatus mode, T* param) {
  attachInterruptParam(interruptNum, (voidFuncPtrParam)userFunc, mode, (void*)param);
}

}
#endif
#endif
