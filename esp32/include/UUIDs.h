// UUIDs.h
#pragma once
#include <NimBLEUUID.h>

// Nordic UART Service 
namespace UUIDs {
  const NimBLEUUID Service("12345678-1234-5678-1234-56789ABCDEF0");
  const NimBLEUUID RxChar ("12345678-1234-5678-1234-56789ABCDEF1"); // Write
  const NimBLEUUID TxChar ("12345678-1234-5678-1234-56789ABCDEF2"); // Notify
}