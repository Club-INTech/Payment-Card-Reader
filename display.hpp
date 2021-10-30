#pragma once

#include "detail/utility.hpp"
#include "session.hpp"

static bool is_display_enabled = true;

template<typename... Args>
static nfc::Status print(Args &&... args) {
  if (is_display_enabled)
    Serial.print(FWD(args)...);
}

template<typename... Args>
static nfc::Status println(Args &&... args) {
  if (is_display_enabled)
    Serial.println(FWD(args)...);
}

static void enable_display(bool value) { is_display_enabled = value; }

static nfc::Status error_handler(nfc::Status status) {
  using namespace nfc;

  switch (status) {
  case Status::OK:
    break;
  case Status::NO_CARD_DETECTED:
    println("No card detected");
    break;
  case Status::NO_UID_RECEIVED:
    println("No uid received");
    break;
  case Status::BAD_KEY:
    println("Bad key");
    break;
  case Status::NO_ACTIVE_PICC:
    println("No PICC selected");
    break;
  case Status::HALT_DENIED:
    println("PICC denied the halt request");
    break;
  case Status::ACCESS_DENIED:
    println("PICC didn't grant access to this block");
    break;
  case Status::FATAL:
    println("An unrecoverable error has occured and connection has been terminated");
    break;
  default:
    print("Unknown error : ");
    println(static_cast<int>(status));
    break;
  }

  return status;
}

static void print_byte_sequence(const uint8_t *sequence, size_t size) {
  for (size_t i = 0; i < size; i++) {
    if (sequence[i] <= 0xf)
      print('0');
    print(sequence[i], HEX);
    print(' ');
  }
  println();
}

template<size_t N>
static void print_byte_sequence(const uint8_t (&array)[N]) {
  print_byte_sequence(array, N);
}
