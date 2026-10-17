# AWARE SOC — Software for Coastguards Operations

> Connecting the AWARE wearable disaster-request device to the coordination dashboard used by coastguard teams.

---

## What Is This?

AWARE combines a **battery-powered wearable device** (Arduino Uno R3) with this **SOC dashboard** running on a local computer or gateway. It is designed for remote coastal communities and disaster zones where internet access, grid electricity, or cellular service may be unavailable.

A person in distress presses the button on their wearable. The signal is received by the SOC. Operators track the request through acknowledgment → assignment → contact → assistance delivery → resolution — ensuring every request is followed up and nothing is missed.

---

## Quick Start (Demo — No Hardware Needed)

```
# 1. Open the dashboard
Open aware-soc/index.html in any modern browser (Chrome, Edge, Firefox, Safari)

# 2. Dashboard loads in Simulation Mode automatically
# 3. Use the "Simulate Device" buttons in the Hardware panel to trigger events
```

**That's it.** No server, no install, no internet needed. All data is stored in `localStorage`.

---

## Hardware Mode (Real Arduino)

### Option A — Chrome / Edge (Web Serial API)

1. Connect the AWARE Mini Arduino via USB
2. Open `index.html` in **Chrome or Edge** (not Firefox/Safari)
3. In the Hardware panel → click **Hardware** mode
4. A port-selection dialog appears — select your Arduino port
5. The dashboard connects at **9600 baud**

### Option B — Any Browser via Node.js Bridge

Use this if you want to run the dashboard in Firefox, or serve it over a local network.

```bash
# Install dependencies (one time)
cd aware-soc
npm install

# Run the bridge (auto-detects Arduino, or specify port)
node serial-bridge.js          # auto-detect
node serial-bridge.js COM3     # Windows
node serial-bridge.js /dev/ttyACM0  # Linux/Mac

# Then open index.html in any browser
# Switch to Hardware mode — it will connect via ws://localhost:8765
```

---

## Firmware Protocol

Derived from the actual AWARE Mini Arduino source code.

| Clicks | Firmware function | Action |
|--------|------------------|--------|
| 1 | `submitReport()` | Create new emergency report |
| 2 | `checkStatus()` | Print current status to serial |
| 3 | `confirmAssistance()` | User reports help has arrived at scene |
| 4 | `cycleCategory()` | Cycle through 6 emergency categories |
| 6 | `fullReset()` | Full device reset |

**Serial:** 9600 baud, `\n` line endings

**Device → SOC** (multi-line blocks — parsed by serial bridge):
```
NEW AWARE REPORT          ← 1-click submit
Report ID: 1
Emergency: MEDICAL HELP
Status: QUEUED LOCALLY

SELECTED CATEGORY: SEARCH AND RESCUE   ← 4-click category cycle
ASSISTANCE ARRIVAL REPORTED             ← 3-click confirm
AWARE FULL RESET                        ← 6-click reset
!!! AWARE OVERDUE ALERT !!!             ← auto, after 30s
```

**SOC → Device** (strict state machine — wrong command is silently ignored by firmware):
| Command | Valid when device is | Firmware transition |
|---------|---------------------|---------------------|
| `R` | `QUEUED` | → `RECEIVED` (Yellow LED on) |
| `A` | `RECEIVED` | → `ACKNOWLEDGED` (Green LED on) |
| `V` | `HELP_REPORTED` | → `VERIFIED` (Green LED stays) |
| `X` | Any | Full reset |

**Emergency Categories (exact firmware strings):**
`MEDICAL HELP` · `SEARCH AND RESCUE` · `FOOD AND WATER` · `SHELTER` · `BLOCKED ROAD` · `OTHER EMERGENCY`

**LED logic (mirrors firmware `updateIndicators()`):**
- Yellow D11 = `status >= RECEIVED`
- Green D12 = `status >= ACKNOWLEDGED`
- Red D13 = blinks when `isOverdue()` (30s after submit with no R)

**Hardware Pins:** Button D2 · Yellow D11 · Green D12 · Red D13 · Buzzer A2

---

## 90-Second Demo Walkthrough

Use the **▶ Walkthrough** button in the top right. Or follow these steps:

1. **Trigger a request** — Press "1 Click — Submit Report" in the Hardware panel (or press button on physical wearable). Watch it appear as a red marker on the map and in the list.

2. **SOC receives it** — Status shows "Received". Active count and Unassigned count update. Summary bar reflects live totals.

3. **Acknowledge & Assign** — Select the request. Click "Acknowledge Receipt". Choose Team Alpha and click "Assign Team". Yellow LED lights on the device (command `R` sent).

4. **Record contact** — Click "📱 Responder" to open the mobile responder view. Select the request, choose "Contact Attempted", add a field note, submit.

5. **Confirm assistance** — In the detail panel, click "Confirm Assistance Delivered". Status → Delivered. Green LED on device (command `V` sent).

6. **Resolve** — Click "Resolve Case". Count updates. Observe the pre-loaded OVERDUE request showing ⚠ OVERDUE in red.

7. **Offline demo** — Click the hardware pill (top right) to simulate going offline. Submit a responder update. It queues. Click the amber queue indicator to sync — any conflicts are preserved for review.

8. **Hardware vs Simulation summary** — See Step 8 of the walkthrough panel.

---

## What Works With Real Hardware

| Capability | Status |
|------------|--------|
| 1-click creates new request in SOC | ✅ Live hardware |
| 2-click status check → LED feedback | ✅ Live hardware |
| 3-click arrival report | ✅ Live hardware |
| 4-click category cycle | ✅ Live hardware |
| 6-click full reset | ✅ Live hardware |
| SOC → device commands (R, A, V, X) | ✅ Live hardware |
| LED state display in dashboard | ✅ Mirrors device |
| Duplicate message suppression | ✅ Implemented |
| Offline queue + sync | ✅ Implemented |
| Persistent storage across reloads | ✅ localStorage |
| ArcGIS / GPS location | 🟡 Planned (map uses demo coordinates) |
| LoRa radio communication | 🟡 Planned (USB serial in demo) |
| Battery readout | 🟡 Planned |
| Multi-gateway sync | 🟡 Planned |

---

## Project Context

AWARE addresses the real problem observed in disasters like the 2025 Panama earthquake, where:
- Authorities visited already-helped areas multiple times while missing others
- No systematic tracking of who received assistance vs. who still needed it
- Limited budget meant every wasted trip reduced overall response capacity

The wearable puts help-request capability in the hands of isolated community members. The SOC ensures every request is tracked to resolution — preventing both missed rescues and wasted repeat visits.

---

## File Structure

```
aware-soc/
├── index.html            ← SOC dashboard (open this)
├── serial-bridge.js      ← Node.js serial↔WebSocket bridge
├── firmware-protocol.ino ← Protocol reference (read-only)
├── package.json          ← Node dependencies for bridge
└── README.md             ← This file
```

---

*AWARE — Adaptive Wearable for Assisted Response and Emergency coordination*
