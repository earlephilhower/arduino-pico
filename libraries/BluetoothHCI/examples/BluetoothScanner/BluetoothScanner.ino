#include <BluetoothHCI.h>

BluetoothHCI hci;

void BTBasicSetup() {
  l2cap_init();
  sm_init();
  gap_set_default_link_policy_settings(LM_LINK_POLICY_ENABLE_SNIFF_MODE | LM_LINK_POLICY_ENABLE_ROLE_SWITCH);
  hci_set_master_slave_policy(HCI_ROLE_MASTER);
  hci_set_inquiry_mode(INQUIRY_MODE_RSSI_AND_EIR);

  hci.install();
  hci.begin();
}

void setup() {
  delay(5000);
  BTBasicSetup();
}

void loop() {
  Serial.printf("BEGIN SCAN @%lu ...", millis());
  auto l = hci.scan(BluetoothHCI::any_cod);
  Serial.printf("END SCAN @%lu\n\n", millis());
  Serial.printf("%-8s | %-17s | %-4s | %s\n", "Class", "Address", "RSSI", "Name");
  Serial.printf("%-8s | %-17s | %-4s | %s\n", "--------", "-----------------", "----", "----------------");
  for (auto e : l) {
    Serial.printf("%08lx | %17s | %4d | %s\n", e.deviceClass(), e.addressString(), e.rssi(), e.name());
  }
  Serial.printf("\n\n\n");
}
