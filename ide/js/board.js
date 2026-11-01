// The Pico BASIC board over Web Serial: connect, run a command at the Ok
// prompt and read what it prints, list files, and move files with XMODEM.
(function (root) {
  'use strict';

  const enc = new TextEncoder();
  const sleep = ms => new Promise(r => setTimeout(r, ms));

  // Terminal text to plain lines: ANSI codes out, CR LF to LF, and a bare CR
  // (the line editor redrawing) keeps what came after it.
  function cleanLines(text) {
    const t = text
      .replace(/\x1b\[[0-9;?]*[ -\/]*[@-~]/g, '')
      .replace(/\x1b[@-_]/g, '')
      .replace(/\r\n/g, '\n');
    return t.split('\n').map(l => {
      const i = l.lastIndexOf('\r');
      return (i >= 0 ? l.slice(i + 1) : l).replace(/[\x00-\x08\x0b-\x1f\x7f]/g, '');
    });
  }

  function quote(s) { return '"' + s + '"'; }

  class Board {
    constructor() {
      this.port = null;
      this.writer = null;
      this.reader = null;
      this.q = [];          // bytes waiting for an operation
      this.qHead = 0;
      this.wake = null;
      this.capture = false; // an operation is reading the line
      this.chain = Promise.resolve();
      this.decoder = new TextDecoder();
      this.onConsole = () => {};  // text the board printed
      this.onState = () => {};    // 'connected' | 'disconnected'
      this.showInternal = false;  // show the IDE's own commands (FILES, XSEND...) on the console
    }

    get connected() { return !!this.port; }

    async connect() {
      if (!('serial' in navigator)) throw new Error('This browser has no Web Serial. Use Chrome or Edge.');
      // 0x2E8A is Raspberry Pi: the Pico SDK's USB serial uses it.
      const port = await navigator.serial.requestPort({ filters: [{ usbVendorId: 0x2e8a }] });
      await this.open(port);
    }

    // Reconnects to a port the browser has already allowed, if there is one.
    async reconnect() {
      if (!('serial' in navigator)) return false;
      const ports = await navigator.serial.getPorts();
      if (!ports.length) return false;
      await this.open(ports[0]);
      return true;
    }

    async open(port) {
      await port.open({ baudRate: 115200 });  // USB CDC: the rate doesn't matter
      // DTR tells the board a terminal is there (it shows its banner).
      try { await port.setSignals({ dataTerminalReady: true, requestToSend: true }); } catch (e) {}
      this.port = port;
      this.newFirmware = null;  // found out again for this board
      this.writer = port.writable.getWriter();
      this.readLoop();
      this.onState('connected');
    }

    async disconnect() {
      const port = this.port;
      if (!port) return;
      this.port = null;
      try { await this.reader?.cancel(); } catch (e) {}
      try { this.writer?.releaseLock(); } catch (e) {}
      try { await port.close(); } catch (e) {}
      this.onState('disconnected');
    }

    async readLoop() {
      const port = this.port;
      while (port.readable && this.port === port) {
        this.reader = port.readable.getReader();
        try {
          for (;;) {
            const { value, done } = await this.reader.read();
            if (done) break;
            if (value) this.received(value);
          }
        } catch (e) {
          // The board was unplugged or reset.
        } finally {
          try { this.reader.releaseLock(); } catch (e) {}
        }
        if (this.port === port) break;
      }
      if (this.port === port) {
        this.port = null;
        try { await port.close(); } catch (e) {}
        this.onState('disconnected');
      }
    }

    received(bytes) {
      if (this.capture) {
        for (const b of bytes) this.q.push(b);
        if (this.wake) { const w = this.wake; this.wake = null; w(); }
      } else {
        this.onConsole(this.decoder.decode(bytes, { stream: true }));
      }
    }

    // A byte from the line, or -1 after timeoutMs.
    readByte(timeoutMs) {
      if (this.qHead < this.q.length) return Promise.resolve(this.take());
      return new Promise(resolve => {
        const t = setTimeout(() => { this.wake = null; resolve(-1); }, timeoutMs);
        this.wake = () => { clearTimeout(t); resolve(this.qHead < this.q.length ? this.take() : -1); };
      });
    }

    take() {
      const b = this.q[this.qHead++];
      if (this.qHead > 4096) { this.q = this.q.slice(this.qHead); this.qHead = 0; }
      return b;
    }

    async write(data) {
      if (!this.writer) throw new Error('Not connected.');
      await this.writer.write(typeof data === 'string' ? enc.encode(data) : Uint8Array.from(data));
    }

    // Types text at the prompt, as a terminal would (used by the console).
    async type(text) { await this.write(text); }

    // Runs fn with the line to itself; what is left over goes to the console.
    exclusive(fn) {
      const run = async () => {
        if (!this.port) throw new Error('Not connected.');
        this.capture = true;
        this.q = []; this.qHead = 0;
        try {
          return await fn();
        } finally {
          const rest = this.q.slice(this.qHead);
          this.q = []; this.qHead = 0;
          this.capture = false;
          if (rest.length) this.onConsole(this.decoder.decode(Uint8Array.from(rest), { stream: true }));
        }
      };
      const p = this.chain.then(run, run);
      this.chain = p.catch(() => {});
      return p;
    }

    // Reads (and shows) text until done(text) is true at the end of a line.
    async readUntil(done, timeoutMs) {
      const dec = new TextDecoder();
      let text = '';
      const until = Date.now() + timeoutMs;
      for (;;) {
        const left = until - Date.now();
        const b = left > 0 ? await this.readByte(left) : -1;
        if (b < 0) {
          const err = new Error('The board did not answer (is a program running? Stop it first).');
          err.text = text;
          throw err;
        }
        const s = dec.decode(Uint8Array.of(b), { stream: true });
        text += s;
        if (this.showInternal) this.onConsole(s);
        if (b === 0x0a && done(text)) return text;
      }
    }

    // Waits for the Ok prompt at the end of what the board prints, and a
    // short quiet spell after it (so "Ok" in a listing doesn't end it early).
    async readToOk(timeoutMs) {
      let text = await this.readUntil(t => /(^|\n)Ok\r?\n$/.test(t), timeoutMs);
      for (;;) {
        const b = await this.readByte(60);
        if (b < 0) return text;
        this.qHead--;  // put it back and read on
        text += await this.readUntil(t => /(^|\n)Ok\r?\n$/.test(t), timeoutMs);
      }
    }

    // Runs a direct command and gives the lines it printed (the echo and the
    // Ok taken off).
    command(cmd, timeoutMs = 5000) {
      return this.exclusive(() => this.commandHere(cmd, timeoutMs));
    }

    async commandHere(cmd, timeoutMs) {
      await this.write(cmd + '\r');
      const text = await this.readToOk(timeoutMs);
      const lines = cleanLines(text);
      while (lines.length && lines[lines.length - 1] === '') lines.pop();
      if (lines.length && lines[lines.length - 1] === 'Ok') lines.pop();
      const echo = lines.findIndex(l => l.includes(cmd));
      return echo >= 0 ? lines.slice(echo + 1) : lines;
    }

    // Is the board at the Ok prompt?
    // An empty line gives no Ok, so a REM (which does nothing) is typed.
    // A board that has just seen the terminal connect prints its banner and
    // an Ok (or "A program is running") first. 'ok', 'running' or 'busy'.
    ping() {
      return this.exclusive(async () => {
        const dec = new TextDecoder();
        let text = '';
        const until = Date.now() + 600;
        for (;;) {  // the banner, if it comes
          const left = until - Date.now();
          const b = left > 0 ? await this.readByte(left) : -1;
          if (b < 0) break;
          text += dec.decode(Uint8Array.of(b), { stream: true });
          if (/A program is running/.test(text)) return 'running';
        }
        await this.write('REM\r');
        try {
          await this.readToOk(3000);
          return 'ok';
        } catch (e) {
          return /A program is running/.test(text + (e.text || '')) ? 'running' : 'busy';
        }
      });
    }

    // Stops a running program (Ctrl-C).
    async stop() { await this.write([0x03]); }

    // The files in a folder, e.g. "FLASH:/" or "SD:/GAMES/".
    // Gives [{name, dir}] and the free space line.
    // Newer firmware has FILES "x", 1 (whole names and sizes, one a line)
    // and XSEND/XRECV "x", "Y" (YMODEM: exact sizes). Found out on the first
    // listing: null until then.
    async listFiles(folder) {
      return this.exclusive(async () => {
        if (this.newFirmware !== false) {
          const lines = await this.commandHere('FILES ' + quote(folder) + ', 1', 8000);
          const text = lines.join(' ');
          if (/Syntax error|Illegal function call/.test(text)) {
            this.newFirmware = false;
          } else {
            this.newFirmware = true;
            return this.parseLong(lines);
          }
        }
        const lines = await this.commandHere('FILES ' + quote(folder), 8000);
        const freeAt = lines.findIndex(l => /Bytes free$/.test(l.trim()));
        if (freeAt < 0) {
          if (lines.some(l => l.trim() === 'File not found')) return { files: [], free: '' };
          throw new Error(lines.filter(l => l.trim()).join(' ') || 'FILES gave nothing back.');
        }
        const files = [];
        for (const line of lines.slice(0, freeAt)) {
          if (line.trim() === 'File not found') continue;
          // FILES prints four to a line, each in 20 columns.
          for (let i = 0; i < line.length; i += 20) {
            let e = line.slice(i, i + 20).replace(/\s+$/, '');
            if (!e) continue;
            const dir = e.endsWith('<DIR>');
            if (dir) e = e.slice(0, -5);
            files.push({ name: e, dir });
          }
        }
        // FILES shows at most 14 characters of a name: DIR$ gives the rest.
        const prefixes = [...new Set(files.filter(f => f.name.length === 14).map(f => f.name))];
        for (const p of prefixes) {
          const full = [];
          for (let n = 1; n < 50; n++) {
            const out = await this.commandHere('PRINT DIR$(' + quote(folder + p + '*') + ',' + n + ')', 3000);
            const name = (out[0] || '').trim();
            if (!name) break;
            full.push(name);
          }
          let k = 0;
          for (const f of files) if (f.name === p && k < full.length) f.name = full[k++];
        }
        files.sort((a, b) => (b.dir - a.dir) || a.name.localeCompare(b.name));
        return { files, free: lines[freeAt].trim() };
      });
    }

    parseLong(lines) {
      const freeAt = lines.findIndex(l => /Bytes free$/.test(l.trim()));
      if (freeAt < 0) {
        if (lines.some(l => l.trim() === 'File not found')) return { files: [], free: '' };
        throw new Error(lines.filter(l => l.trim()).join(' ') || 'FILES gave nothing back.');
      }
      const files = [];
      for (const line of lines.slice(0, freeAt)) {
        const m = /^\s*(<DIR>|\d+)  (.+)$/.exec(line);
        if (!m) continue;
        files.push(m[1] === '<DIR>' ? { name: m[2], dir: true } : { name: m[2], dir: false, size: +m[1] });
      }
      files.sort((a, b) => (b.dir - a.dir) || a.name.localeCompare(b.name));
      return { files, free: lines[freeAt].trim() };
    }

    // Waits for XSEND's or XRECV's "Start the XMODEM ..." line; an error
    // (and the Ok after it) instead throws.
    async waitForStart(what) {
      const text = await this.readUntil(t => /Start the [XYZ]MODEM/.test(t) || /(^|\n)Ok\r?\n$/.test(t), 5000);
      if (!/Start the [XYZ]MODEM/.test(text)) {
        const lines = cleanLines(text).filter(l => l.trim() && l.trim() !== 'Ok');
        throw new Error(lines.slice(1).join(' ') || ('The board would not ' + what + '.'));
      }
    }

    // The io object xmodem.js works over: the raw line, not shown.
    xio() {
      return { read: ms => this.readByte(ms), write: b => this.write(b) };
    }

    // Reads a file from the board (XSEND on the board).
    readFile(path, onProgress) {
      return this.exclusive(async () => {
        const y = this.newFirmware === true;
        await this.write('XSEND ' + quote(path) + (y ? ', "Y"' : '') + '\r');
        await this.waitForStart('send ' + path);
        let data;
        try {
          data = y ? (await XModem.yreceive(this.xio(), { onProgress })).data
                   : await XModem.receive(this.xio(), { onProgress });
        } catch (e) {
          // The board says why (e.g. "Can't open X.") and comes back to Ok.
          let more = '';
          try { more = await this.readToOk(5000); } catch (e2) { more = e2.text || ''; }
          const why = cleanLines(more).map(l => l.trim()).filter(l => l && l !== 'Ok' && !/^Device I\/O error/.test(l));
          throw new Error(why.join(' ') || e.message);
        }
        await this.readToOk(5000);
        return data;
      });
    }

    // Writes a file to the board (XRECV on the board), replacing one there.
    writeFile(path, data, onProgress) {
      return this.exclusive(async () => {
        const y = this.newFirmware === true;
        await this.write('XRECV ' + quote(path) + (y ? ', "Y"' : '') + '\r');
        await this.waitForStart('receive ' + path);
        try {
          if (y) await XModem.ysend(this.xio(), path.slice(path.lastIndexOf('/') + 1), data, { onProgress });
          else await XModem.send(this.xio(), data, { onProgress });
        } catch (e) {
          try { await this.readToOk(5000); } catch (e2) {}
          throw e;
        }
        const lines = cleanLines(await this.readToOk(5000)).filter(l => l.trim() && l.trim() !== 'Ok');
        if (lines.some(l => /error/i.test(l))) throw new Error(lines.join(' '));
      });
    }

    deleteFile(path) { return this.simple('KILL ' + quote(path)); }
    renameFile(from, to) { return this.simple('NAME ' + quote(from) + ' AS ' + quote(to)); }
    makeFolder(path) { return this.simple('MKDIR ' + quote(path)); }
    removeFolder(path) { return this.simple('RMDIR ' + quote(path)); }

    // A command that prints nothing when it works.
    async simple(cmd) {
      const out = (await this.command(cmd)).filter(l => l.trim());
      if (out.length) throw new Error(out.join(' '));
    }
  }

  root.Board = Board;
  root.cleanLines = cleanLines;
})(this);
