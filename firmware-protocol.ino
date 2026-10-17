/*
  AWARE MINI - Single Button Disaster Reporting
  Arduino Uno R3 - NO LCD

  ──────────────────────────────────────────
  FIRMWARE PROTOCOL REFERENCE (read by SOC)
  ──────────────────────────────────────────

  BUTTON CLICK ACTIONS
  ─────────────────────
  1 click  = Submit report  (new emergency request)
  2 clicks = Check status   (flash current status of most recent active request)
  3 clicks = Report assistance arrived
  4 clicks = Cycle emergency category  (cycles: Medical → Rescue → Fire → Flood → Shelter → General)
  6 clicks = FULL RESET

  HARDWARE PINS
  ─────────────
  Button:       D2   (INPUT_PULLUP, active LOW)
  LED Yellow:   D11  (meaning: report received / in progress)
  LED Green:    D12  (meaning: acknowledged / assistance delivered)
  LED Red:      D13  (meaning: error / overdue / reset)
  Buzzer:       A2

  SERIAL
  ──────
  Baud rate:    9600
  Line ending:  \n

  MESSAGES SENT BY DEVICE (incoming to SOC)
  ─────────────────────────────────────────
  "CLICKS:1 CAT:Medical"    — 1-click submit report, current category = Medical
  "CLICKS:2"                — status check
  "CLICKS:3"                — assistance arrived
  "CLICKS:4"                — category cycled (next category follows in CAT field)
  "CLICKS:6"                — full reset

  Fallback (bare number, if simplified firmware):
  "1"  "2"  "3"  "4"  "6"

  COMMANDS RECEIVED BY DEVICE (outgoing from SOC)
  ────────────────────────────────────────────────
  "R\n"  — System received report    → Yellow LED blinks
  "A\n"  — Responder acknowledged    → Green LED blinks
  "V\n"  — Assistance verified       → Green LED steady, buzzer confirmation
  "X\n"  — Reset                     → All LEDs blink, system resets

  LED STATE MEANINGS (as implemented in SOC)
  ──────────────────────────────────────────
  Yellow ON  = Report received / waiting for responder
  Green  ON  = Responder in contact / assistance delivered
  Red    ON  = Error, overdue, or reset triggered

  CATEGORY CYCLE ORDER
  ────────────────────
  0: Medical
  1: Rescue
  2: Fire
  3: Flood
  4: Shelter
  5: General

  DEMO COMMANDS (for Serial Monitor testing)
  ─────────────────────────────────────────
  R = System received report
  A = Responder acknowledged
  V = Assistance verified
  X = Reset
*/

// This file is a protocol reference only.
// The actual firmware source is at:
// https://github.com/koqiqok/Hackathon/blob/main/6%20click%20emergency
//
// No changes to firmware hardware pins or click behavior are required
// for the SOC demo. The SOC reads "CLICKS:N CAT:X" lines or bare
// single-digit lines as fallback.
