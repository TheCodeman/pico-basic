// XMODEM for the PC side of XSEND and XRECV.
//
// The board's XSEND sends XMODEM-1K (1024-byte blocks, CRC-16), and its
// XRECV takes 128 or 1024-byte blocks, CRC-16 or a checksum. Both work over
// an `io` object:
//   io.read(timeoutMs) -> Promise<byte, or -1 on timeout>
//   io.write(Uint8Array | number[]) -> Promise
// This file has no browser or serial code in it, so Node can test it too.
(function (root) {
  'use strict';

  const SOH = 0x01, STX = 0x02, EOT = 0x04, ACK = 0x06, NAK = 0x15, CAN = 0x18, SUB = 0x1a, C = 0x43;
  const RETRIES = 10;

  function crc16(data) {
    let crc = 0;
    for (let i = 0; i < data.length; i++) {
      crc ^= data[i] << 8;
      for (let k = 0; k < 8; k++) crc = crc & 0x8000 ? ((crc << 1) ^ 0x1021) & 0xffff : (crc << 1) & 0xffff;
    }
    return crc;
  }

  function checksum(data) {
    let sum = 0;
    for (let i = 0; i < data.length; i++) sum = (sum + data[i]) & 0xff;
    return sum;
  }

  async function readN(io, n, timeoutMs) {
    const buf = new Uint8Array(n);
    for (let i = 0; i < n; i++) {
      const b = await io.read(timeoutMs);
      if (b < 0) return null;
      buf[i] = b;
    }
    return buf;
  }

  // Reads and drops bytes until the line has been quiet for `ms`.
  async function purge(io, ms) {
    while ((await io.read(ms)) >= 0) {}
  }

  async function cancel(io) {
    await io.write([CAN, CAN, CAN, CAN, CAN]);
  }

  // Receives one file. Returns its bytes with the ^Z padding taken off the end
  // (pass keepPadding to keep it). onProgress(bytesSoFar) is optional.
  async function receive(io, opts = {}) {
    const onProgress = opts.onProgress || (() => {});
    const parts = [];
    let total = 0, expect = 1, errors = 0, started = false, asks = 0;
    for (;;) {
      if (!started) {
        if (++asks > RETRIES) throw new Error('The board did not start sending.');
        await io.write([C]);
      }
      const h = await io.read(started ? 10000 : 3000);
      if (h < 0) {
        if (!started) continue;
        if (++errors > RETRIES) { await cancel(io); throw new Error('The board stopped sending.'); }
        await io.write([NAK]);
        continue;
      }
      if (h === EOT) { await io.write([ACK]); break; }
      if (h === CAN) {
        if ((await io.read(1000)) === CAN) throw new Error('The board cancelled the transfer.');
        continue;
      }
      if (h !== SOH && h !== STX) continue;  // noise before the first block
      started = true;
      const len = h === SOH ? 128 : 1024;
      const blk = await readN(io, 2 + len + 2, 2000);
      const good = blk && (blk[0] ^ blk[1]) === 0xff &&
        crc16(blk.subarray(2, 2 + len)) === ((blk[2 + len] << 8) | blk[3 + len]);
      if (!good) {
        if (++errors > RETRIES) { await cancel(io); throw new Error('Too many bad blocks.'); }
        await purge(io, 200);
        await io.write([NAK]);
        continue;
      }
      const seq = blk[0];
      if (seq === (expect & 0xff)) {
        parts.push(blk.slice(2, 2 + len));
        total += len;
        expect++;
        onProgress(total);
      } else if (seq !== ((expect - 1) & 0xff)) {  // not a repeat of the last one
        await cancel(io);
        throw new Error('Blocks arrived out of order.');
      }
      await io.write([ACK]);
    }
    const out = new Uint8Array(total);
    let at = 0;
    for (const p of parts) { out.set(p, at); at += p.length; }
    let end = out.length;
    if (!opts.keepPadding) while (end > 0 && out[end - 1] === SUB) end--;
    return out.subarray(0, end);
  }

  // Waits for ACK (true) or NAK (false); throws on a cancel or a timeout.
  async function waitAck(io) {
    for (;;) {
      const b = await io.read(10000);
      if (b < 0) return false;
      if (b === ACK) return true;
      if (b === NAK) return false;
      if (b === CAN && (await io.read(1000)) === CAN) throw new Error('The board cancelled the transfer.');
    }
  }

  // Sends `data` (a Uint8Array). onProgress(bytesSoFar, total) is optional.
  async function send(io, data, opts = {}) {
    const onProgress = opts.onProgress || (() => {});
    // The receiver asks with 'C' (CRC-16) or NAK (checksum).
    let mode = -1;
    for (let waited = 0; waited < 60000 && mode < 0; waited += 1000) {
      const b = await io.read(1000);
      if (b === C) mode = 1;
      else if (b === NAK) mode = 0;
      else if (b === CAN && (await io.read(1000)) === CAN) throw new Error('The board cancelled the transfer.');
    }
    if (mode < 0) throw new Error('The board did not ask for the file.');
    const crc = mode === 1;
    let seq = 1, pos = 0;
    while (pos < data.length) {
      const left = data.length - pos;
      const len = crc && left >= 1024 ? 1024 : 128;  // the tail goes in 128s: less padding
      const blk = new Uint8Array(3 + len + (crc ? 2 : 1));
      blk[0] = len === 1024 ? STX : SOH;
      blk[1] = seq & 0xff;
      blk[2] = 0xff - (seq & 0xff);
      blk.fill(SUB, 3, 3 + len);
      blk.set(data.subarray(pos, pos + Math.min(len, left)), 3);
      const body = blk.subarray(3, 3 + len);
      if (crc) { const c = crc16(body); blk[3 + len] = c >> 8; blk[4 + len] = c & 0xff; }
      else blk[3 + len] = checksum(body);
      let tries = 0;
      for (;;) {
        await io.write(blk);
        if (await waitAck(io)) break;
        if (++tries >= RETRIES) { await cancel(io); throw new Error('The board did not take block ' + seq + '.'); }
      }
      pos += Math.min(len, left);
      seq++;
      onProgress(pos, data.length);
    }
    for (let tries = 0;; tries++) {
      await io.write([EOT]);
      if (await waitAck(io)) break;
      if (tries >= RETRIES) throw new Error('The board did not take the end of the file.');
    }
  }

  // ---- YMODEM (new firmware: XSEND/XRECV "file", "Y") ----------------------
  // Block 0 carries the name and size, so the file comes back its exact size.

  function block(seq, body) {
    const len = body.length;
    const blk = new Uint8Array(3 + len + 2);
    blk[0] = len === 1024 ? STX : SOH;
    blk[1] = seq & 0xff;
    blk[2] = 0xff - (seq & 0xff);
    blk.set(body, 3);
    const c = crc16(body);
    blk[3 + len] = c >> 8;
    blk[4 + len] = c & 0xff;
    return blk;
  }

  async function waitFor(io, want, ms) {
    for (let waited = 0; waited < ms; waited += 1000) {
      const b = await io.read(1000);
      if (b === want) return;
      if (b === CAN && (await io.read(1000)) === CAN) throw new Error('The board cancelled the transfer.');
    }
    throw new Error('No answer from the board.');
  }

  async function sendBlock(io, blk) {
    for (let tries = 0; ; tries++) {
      await io.write(blk);
      if (await waitAck(io)) return;
      if (tries >= RETRIES) { await cancel(io); throw new Error('The board did not take a block.'); }
    }
  }

  async function ysend(io, name, data, opts = {}) {
    const onProgress = opts.onProgress || (() => {});
    await waitFor(io, C, 60000);
    const head = new Uint8Array(128);
    const meta = new TextEncoder().encode(name + '\0' + data.length);
    head.set(meta.subarray(0, 127));
    await sendBlock(io, block(0, head));
    await waitFor(io, C, 10000);
    let seq = 1, pos = 0;
    while (pos < data.length) {
      const left = data.length - pos;
      const len = left > 128 ? 1024 : 128;
      const body = new Uint8Array(len).fill(SUB);
      body.set(data.subarray(pos, pos + Math.min(len, left)));
      await sendBlock(io, block(seq++, body));
      pos += Math.min(len, left);
      onProgress(pos, data.length);
    }
    for (let tries = 0; ; tries++) {
      await io.write([EOT]);
      if (await waitAck(io)) break;
      if (tries >= RETRIES) throw new Error('The board did not take the end of the file.');
    }
    await waitFor(io, C, 10000);  // the next file? none: an empty block 0
    await sendBlock(io, block(0, new Uint8Array(128)));
  }

  async function readBlock(io, ms) {
    for (;;) {
      const h = await io.read(ms);
      if (h < 0) return { timeout: true };
      if (h === EOT) return { eot: true };
      if (h === CAN) {
        if ((await io.read(1000)) === CAN) throw new Error('The board cancelled the transfer.');
        continue;
      }
      if (h !== SOH && h !== STX) continue;
      const len = h === SOH ? 128 : 1024;
      const blk = await readN(io, 2 + len + 2, 2000);
      if (!blk || (blk[0] ^ blk[1]) !== 0xff || crc16(blk.subarray(2, 2 + len)) !== ((blk[2 + len] << 8) | blk[3 + len])) {
        return { bad: true };
      }
      return { seq: blk[0], data: blk.slice(2, 2 + len) };
    }
  }

  async function yreceive(io, opts = {}) {
    const onProgress = opts.onProgress || (() => {});
    // The header block
    let head = null;
    for (let tries = 0; !head; tries++) {
      if (tries > RETRIES) throw new Error('The board did not start sending.');
      await io.write([C]);
      const b = await readBlock(io, 3000);
      if (b.data && b.seq === 0) head = b.data;
      else if (b.bad) await purge(io, 200);
    }
    const z = head.indexOf(0);
    const name = new TextDecoder().decode(head.subarray(0, z));
    const size = parseInt(new TextDecoder().decode(head.subarray(z + 1)).split(/[\s\0]/)[0], 10) || 0;
    await io.write([ACK, C]);
    const out = new Uint8Array(Math.max(size, 0));
    const parts = [];
    let got = 0, expect = 1, errors = 0;
    for (;;) {
      const b = await readBlock(io, 10000);
      if (b.eot) { await io.write([ACK]); break; }
      if (b.timeout || b.bad) {
        if (++errors > RETRIES) { await cancel(io); throw new Error('Too many bad blocks.'); }
        await purge(io, 200);
        await io.write([NAK]);
        continue;
      }
      if (b.seq === (expect & 0xff)) {
        parts.push(b.data);
        got += b.data.length;
        expect++;
        errors = 0;
        onProgress(Math.min(got, size || got), size);
      } else if (b.seq !== ((expect - 1) & 0xff)) {
        await cancel(io);
        throw new Error('Blocks arrived out of order.');
      }
      await io.write([ACK]);
    }
    // The end of the batch: an empty header.
    await io.write([C]);
    const end = await readBlock(io, 5000);
    if (end.data) await io.write([ACK]);
    const all = new Uint8Array(got);
    let at = 0;
    for (const p of parts) { all.set(p, at); at += p.length; }
    return { name, data: size ? all.subarray(0, Math.min(size, got)) : all, size };
  }

  const api = { receive, send, ysend, yreceive, crc16, CAN };
  if (typeof module !== 'undefined' && module.exports) module.exports = api;
  else root.XModem = api;
})(this);
