#!/usr/bin/env node
/**
 * AWARE SOC — Serial Bridge
 * Node.js companion for running the SOC without a Chrome/Edge browser
 * (e.g. in Firefox, or when serving over a local network).
 *
 * What it does:
 *   1. Opens the AWARE Mini Arduino on the specified COM port at 9600 baud
 *   2. Exposes a WebSocket server on ws://localhost:8765
 *   3. Forwards every line from the device → WebSocket clients
 *   4. Forwards every message from WebSocket clients → device
 *
 * The dashboard connects to ws://localhost:8765 when "Hardware" mode is
 * selected and this bridge is running.
 *
 * Usage:
 *   node serial-bridge.js [port]          (default: auto-detect)
 *   node serial-bridge.js COM3            (Windows)
 *   node serial-bridge.js /dev/ttyACM0   (Linux/Mac)
 *
 * Requires:
 *   npm install serialport ws
 *
 * Firmware protocol (9600 baud):
 *   Incoming from device: text lines, e.g. "CLICKS:1 CAT:Medical"
 *   Outgoing to device:   single-char commands  R  A  V  X
 */

'use strict';

const { SerialPort }      = require('serialport');
const { ReadlineParser }  = require('@serialport/parser-readline');
const { WebSocketServer } = require('ws');

const BAUD_RATE  = 9600;
const WS_PORT    = 8765;

// ── CLI arg ─────────────────────────────────────────────────────────
let portPath = process.argv[2] || null;

// ── WebSocket server ────────────────────────────────────────────────
const wss = new WebSocketServer({ port: WS_PORT });
const clients = new Set();

wss.on('listening', () => {
  console.log(`[BRIDGE] WebSocket server listening on ws://localhost:${WS_PORT}`);
});

wss.on('connection', (ws) => {
  clients.add(ws);
  console.log(`[BRIDGE] Dashboard connected (${clients.size} client(s))`);
  ws.send(JSON.stringify({ type: 'bridge_status', connected: !!serial, port: portPath }));

  ws.on('message', (raw) => {
    const text = raw.toString().trim();
    console.log(`[BRIDGE] → device: ${text}`);
    if (serial && serial.isOpen) {
      serial.write(text + '\n', (err) => {
        if (err) console.error('[BRIDGE] Write error:', err.message);
      });
    }
  });

  ws.on('close', () => clients.delete(ws));
});

function broadcast(obj) {
  const msg = JSON.stringify(obj);
  clients.forEach(ws => { try { ws.send(msg); } catch(e) {} });
}

// ── Serial port ──────────────────────────────────────────────────────
let serial = null;

async function autoDetectPort() {
  const list = await SerialPort.list();
  // Prefer ports that look like Arduino
  const arduino = list.find(p =>
    /arduino|uno|ch340|cp210|ftdi/i.test(p.manufacturer || '') ||
    /ttyACM|ttyUSB|usbmodem/i.test(p.path)
  );
  if (arduino) return arduino.path;
  if (list.length > 0) return list[list.length - 1].path; // last available
  return null;
}

async function openSerial(path) {
  serial = new SerialPort({ path, baudRate: BAUD_RATE });
  const parser = serial.pipe(new ReadlineParser({ delimiter: '\n' }));

  serial.on('open', () => {
    console.log(`[BRIDGE] Serial port opened: ${path} @ ${BAUD_RATE} baud`);
    portPath = path;
    broadcast({ type: 'bridge_status', connected: true, port: path });
  });

  serial.on('close', () => {
    console.log('[BRIDGE] Serial port closed');
    broadcast({ type: 'bridge_status', connected: false, port: path });
    // Attempt reconnect after 3 seconds
    setTimeout(async () => {
      console.log('[BRIDGE] Attempting reconnect…');
      try { await openSerial(path); } catch(e) { console.error('[BRIDGE] Reconnect failed:', e.message); }
    }, 3000);
  });

  serial.on('error', (err) => {
    console.error('[BRIDGE] Serial error:', err.message);
    broadcast({ type: 'bridge_error', message: err.message });
  });

  parser.on('data', (line) => {
    const trimmed = line.trim();
    if (!trimmed) return;
    console.log(`[BRIDGE] ← device: ${trimmed}`);
    broadcast({ type: 'serial_data', data: trimmed });
  });
}

// ── Main ─────────────────────────────────────────────────────────────
(async () => {
  if (!portPath) {
    console.log('[BRIDGE] No port specified — scanning for Arduino…');
    portPath = await autoDetectPort();
  }

  if (portPath) {
    console.log(`[BRIDGE] Opening ${portPath}…`);
    try { await openSerial(portPath); }
    catch(e) { console.error('[BRIDGE] Could not open serial port:', e.message); }
  } else {
    console.warn('[BRIDGE] No serial port found. Running WebSocket-only (simulation forwarding).');
  }

  console.log('[BRIDGE] Ready. Open index.html in Chrome/Edge, or connect any WebSocket client.');
  console.log('[BRIDGE] Press Ctrl+C to stop.\n');
})();
