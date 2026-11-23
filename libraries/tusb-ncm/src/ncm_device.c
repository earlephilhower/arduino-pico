
// Backport of https://github.com/hathach/tinyusb/pull/3914
//
// A Windows Update (KB5124008, Sept 2026) enabled a feature-flag-gated NTB
// parameter validation in usbncm.sys requiring wNdbOutDivisor (and by
// symmetry wNdbInDivisor) to be >=4 and a power of 2
//
// Once that makes it into pico-sdk, remove the entire backport again.
// Just leave the include directive, nothing else.
#define netd_control_xfer_cb netd_control_xfer_cb_orig
#include "../../../pico-sdk/lib/tinyusb/src/class/net/ncm_device.c"
#undef netd_control_xfer_cb

static TU_ATTR_ALIGNED(4) const ntb_parameters_t ntb_parameters_fixed = {
  .wLength                  = sizeof(ntb_parameters_t),
  .bmNtbFormatsSupported    = 0x01,
  .dwNtbInMaxSize           = CFG_TUD_NCM_IN_NTB_MAX_SIZE,
  .wNdbInDivisor            = 4,
  .wNdbInPayloadRemainder   = 0,
  .wNdbInAlignment          = 4, // TUD_NCM_ALIGNMENT
  .wReserved                = 0,
  .dwNtbOutMaxSize          = CFG_TUD_NCM_OUT_NTB_MAX_SIZE,
  .wNdbOutDivisor           = 4,
  .wNdbOutPayloadRemainder  = 0,
  .wNdbOutAlignment         = 4,
  .wNtbOutMaxDatagrams      = CFG_TUD_NCM_OUT_MAX_DATAGRAMS_PER_NTB,
};

bool netd_control_xfer_cb(uint8_t rhport, uint8_t stage,
                          const tusb_control_request_t *request) {
  if (stage == CONTROL_STAGE_SETUP &&
      request->bmRequestType_bit.type == TUSB_REQ_TYPE_CLASS &&
      request->bmRequestType_bit.direction == TUSB_DIR_IN &&
      request->bRequest == NCM_GET_NTB_PARAMETERS) {
    tud_control_xfer(rhport, request, (void *) (uintptr_t) &ntb_parameters_fixed,
                     sizeof(ntb_parameters_fixed));
    return true;
      }
  return netd_control_xfer_cb_orig(rhport, stage, request);
}

static_assert(TUSB_VERSION_NUMBER == 1800, "pico-sdk's TinyUSB was updated: check it includes hathach/tinyusb#3914 and remove this backport");
