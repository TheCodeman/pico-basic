// A pretend Pico BASIC board behind navigator.serial, for trying the page
// without hardware: add ?mock to the page's URL, or inject it in tests.
// It answers FILES, PRINT DIR$(), XSEND, XRECV, KILL and RUN roughly as the
// firmware does (FILES's 20-column layout and 14-character names included).
(function () {
  'use strict';
  const enc = new TextEncoder();
  const disk = {
    'FLASH:/': ['BLINK.BAS', 'BROKEN.BAS', 'LABELS.BAS', 'GUESS.BAS', 'PRIMES.BAS', 'AVERYLONGFILENAME.BAS', 'CONFIG.TXT', 'GAMES/'],
    'FLASH:/GAMES/': ['LANDER.BAS'],
    'SD:/': [],
  };
  const content = {
    'FLASH:/BLINK.BAS': '10 REM Blink the LED\n20 PINMODE 25, OUTPUT\n30 PIN 25, 1: PAUSE 500\n40 PIN 25, 0: PAUSE 500\n50 GOTO 30\n',
    'FLASH:/LABELS.BAS': "' LABELS.BAS: a program without line numbers. A label (a name and a\n' colon starting a line) is where GOTO, GOSUB, RESTORE and RESUME go.\nCLS\nPRINT \"Labels demo\"\n\nmenu:\n  PRINT\n  PRINT \"1 Squares  2 Colours  3 Missing file  4 Quit\"\n  INPUT \"Choice\"; C\n  ON C GOSUB squares, colours, nofile\n  IF C = 4 THEN done\n  GOTO menu\n\nsquares:\n  FOR I = 1 TO 5: PRINT I * I;: NEXT\n  PRINT\n  RETURN\n\ncolours:\n  RESTORE colourlist\n  FOR I = 1 TO 3: READ N$: PRINT N$; \" \";: NEXT\n  PRINT\n  RETURN\ncolourlist:\n  DATA red, green, blue\n\nnofile:\n  ON ERROR GOTO oops\n  OPEN \"NOSUCH.TXT\" FOR INPUT AS #1\n  CLOSE #1\nback: ON ERROR GOTO 0\n  RETURN\noops:\n  PRINT \"No such file: error\"; ERR; \"in line\"; ERL\n  RESUME back\n\ndone:\n  PRINT \"Bye\"\n  END\n",
    'FLASH:/GUESS.BAS': '10 RANDOMIZE TIMER\n20 N = INT(RND * 100) + 1\n30 INPUT "Guess (1-100)"; G\n40 IF G < N THEN PRINT "Higher": GOTO 30\n50 IF G > N THEN PRINT "Lower": GOTO 30\n60 PRINT "Got it!"\n',
    'FLASH:/PRIMES.BAS': '10 FOR N = 2 TO 100\n20   FOR D = 2 TO SQR(N)\n30     IF N MOD D = 0 THEN 60\n40   NEXT D\n50   PRINT N;\n60 NEXT N\n',
    'FLASH:/AVERYLONGFILENAME.BAS': '10 PRINT "A long name"\n',
    'FLASH:/BROKEN.BAS': '10 PRINT "Starting"\n20 COLOR 6: PRINT "Colour works"\n30 PRINT "oops\n40 END\n',
    'FLASH:/CONFIG.TXT': 'WIFI_SSID=home\nTZ=EST5EDT\n',
    'FLASH:/GAMES/LANDER.BAS': '10 PRINT "Lander"\n',
  };

  const leafOf = p => p.slice(p.lastIndexOf('/') + 1);

  class MockPort {
    constructor() {
      this.toPage = [];
      this.fromPage = [];
      this.wakeSim = null;
      this.line = '';
      this.xfer = false;
    }
    getInfo() { return { usbVendorId: 0x2e8a, usbProductId: 0x0009 }; }
    async open() {
      const self = this;
      this.readable = new ReadableStream({ start(c) { self.ctrl = c; } });
      this.writable = new WritableStream({ write(chunk) { self.fromPageBytes(chunk); } });
      this.busyProgram = /[?&]running\b/.test(location.search);
      setTimeout(() => this.out('\r\nPico BASIC (mock board)\r\n' + (this.busyProgram ? 'A program is running (Ctrl-C stops it)\r\n' : 'Ok\r\n')), 50);
    }
    async setSignals() {}
    async close() {}
    out(s) { this.ctrl.enqueue(typeof s === 'string' ? enc.encode(s) : s); }

    // XMODEM side of the pretend board
    io() {
      return {
        read: ms => new Promise(res => {
          if (this.fromPage.length) return res(this.fromPage.shift());
          const t = setTimeout(() => { this.wakeSim = null; res(-1); }, ms);
          this.wakeSim = () => { clearTimeout(t); res(this.fromPage.shift()); };
        }),
        write: b => { this.out(Uint8Array.from(b)); return Promise.resolve(); },
      };
    }

    fromPageBytes(chunk) {
      for (const b of chunk) {
        if (this.xfer) {
          this.fromPage.push(b);
          if (this.wakeSim) { const w = this.wakeSim; this.wakeSim = null; w(); }
          continue;
        }
        if (b === 3) { this.out('^C\r\nBreak\r\nOk\r\n'); this.line = ''; this.waitingInput = false; this.busyProgram = false; continue; }
        if (b === 13) { const l = this.line; this.line = ''; this.out('\r\n'); this.exec(l.trim()); continue; }
        if (b === 10) continue;
        if (b === 127 || b === 8) { if (this.line) { this.line = this.line.slice(0, -1); this.out('\b \b'); } continue; }
        this.line += String.fromCharCode(b);
        this.out(String.fromCharCode(b));  // echo
      }
    }

    async exec(l) {
      let m;
      if (this.busyProgram) return;  // a program is running: lines go to it
      if (!l && !this.waitingInput) return;  // as the board: an empty line prints nothing
      if (/^REM\b/i.test(l)) return this.out('Ok\r\n');
      const NEW = /[?&]mock=new\b/.test(location.search);  // the newer firmware
      if (NEW && (m = /^FILES\s+"([^"]*)",\s*1$/i.exec(l))) {
        const list = disk[m[1].toUpperCase()];
        if (!list) return this.out('Path not found\r\nOk\r\n');
        let s = '';
        for (const f of list) {
          if (f.endsWith('/')) s += '     <DIR>  ' + f.slice(0, -1) + '\r\n';
          else s += String(enc.encode(content[m[1].toUpperCase() + f] || '').length).padStart(10) + '  ' + f + '\r\n';
        }
        if (!list.length) s += 'File not found\r\n';
        return this.out(s + '1234567 Bytes free\r\nOk\r\n');
      }
      if (NEW && (m = /^XSEND\s+"([^"]*)",\s*"Y"$/i.exec(l))) {
        this.out('Start the YMODEM receive on the terminal now.\r\n');
        this.xfer = true;
        const body = content[m[1].toUpperCase()] || '';
        try { await XModem.ysend(this.io(), leafOf(m[1]), enc.encode(body)); } catch (e) { this.out('\r\n' + e.message + '\r\n'); }
        this.xfer = false; this.fromPage = [];
        return this.out('\r\nSent 1 file, ' + body.length + ' bytes.\r\nOk\r\n');
      }
      if (NEW && (m = /^XRECV\s+"([^"]*)",\s*"Y"$/i.exec(l))) {
        this.out('Start the YMODEM send on the terminal now.\r\n');
        this.xfer = true;
        const path = m[1].toUpperCase();
        const r = await XModem.yreceive(this.io());
        content[path] = new TextDecoder().decode(r.data);
        const folder = path.slice(0, path.lastIndexOf('/') + 1);
        if (disk[folder] && !disk[folder].includes(leafOf(path))) disk[folder].push(leafOf(path));
        this.xfer = false; this.fromPage = [];
        return this.out('\r\nReceived 1 file, ' + r.data.length + ' bytes.\r\nOk\r\n');
      }
      if ((m = /^FILES\s+"([^"]*)"$/i.exec(l))) {
        const list = disk[m[1].toUpperCase()];
        if (!list) return this.out('Path not found\r\nOk\r\n');
        let s = '', col = 0;
        for (const f of list) {
          const dir = f.endsWith('/');
          let e = (dir ? f.slice(0, -1) : f).slice(0, 14) + (dir ? '<DIR>' : '');
          s += e;
          if (++col === 4) { col = 0; s += '\r\n'; } else s += ' '.repeat(20 - e.length);
        }
        if (col) s += '\r\n';
        if (!list.length) s += 'File not found\r\n';
        return this.out(s + '1234567 Bytes free\r\nOk\r\n');
      }
      if ((m = /^PRINT DIR\$\("([^"]*)\*",(\d+)\)$/i.exec(l))) {
        const folder = m[1].slice(0, m[1].lastIndexOf('/') + 1).toUpperCase();
        const pre = m[1].slice(folder.length).toUpperCase();
        const hits = (disk[folder] || []).filter(f => f.toUpperCase().startsWith(pre)).map(f => f.replace(/\/$/, ''));
        return this.out((hits[+m[2] - 1] || '') + '\r\nOk\r\n');
      }
      if ((m = /^XSEND\s+"([^"]*)"$/i.exec(l))) {
        this.out('Start the XMODEM receive on the terminal now.\r\n');
        this.xfer = true;
        const body = content[m[1].toUpperCase()];
        if (body === undefined) {
          // as xfer.c: waits for the start, then cancels
          await new Promise(r => setTimeout(r, 200));
          this.fromPage = [];
          this.out(Uint8Array.of(24, 24, 24, 24, 24, 24, 24, 24, 8, 8, 8, 8, 8, 8, 8, 8));
          this.xfer = false;
          return this.out('\r\nCan\'t open ' + m[1] + '.\r\nDevice I/O error\r\nOk\r\n');
        }
        try { await XModem.send(this.io(), enc.encode(body)); } catch (e) { this.out('\r\n' + e.message + '\r\n'); }
        this.xfer = false; this.fromPage = [];
        return this.out('\r\nSent 1 file, ' + body.length + ' bytes.\r\nOk\r\n');
      }
      if ((m = /^XRECV\s+"([^"]*)"$/i.exec(l))) {
        this.out('Start the XMODEM send on the terminal now.\r\n');
        this.xfer = true;
        const path = m[1].toUpperCase();
        try {
          const data = await XModem.receive(this.io());
          content[path] = new TextDecoder().decode(data);
          const folder = path.slice(0, path.lastIndexOf('/') + 1);
          const name = path.slice(folder.length);
          if (disk[folder] && !disk[folder].includes(name)) disk[folder].push(name);
          this.xfer = false; this.fromPage = [];
          return this.out('\r\nReceived 1 file, ' + data.length + ' bytes.\r\nOk\r\n');
        } catch (e) {
          this.xfer = false; this.fromPage = [];
          return this.out('\r\n' + e.message + '\r\nDevice I/O error\r\nOk\r\n');
        }
      }
      const up = s => s.toUpperCase();
      const parent = p => p.slice(0, p.lastIndexOf('/') + 1);
      const leaf = p => p.slice(p.lastIndexOf('/') + 1);
      if ((m = /^KILL\s+"([^"]*)"$/i.exec(l))) {
        const p = up(m[1]); const list = disk[parent(p)];
        if (!list || !list.includes(leaf(p))) return this.out('File not found\r\nOk\r\n');
        list.splice(list.indexOf(leaf(p)), 1); delete content[p];
        return this.out('Ok\r\n');
      }
      if ((m = /^NAME\s+"([^"]*)"\s+AS\s+"([^"]*)"$/i.exec(l))) {
        const a = up(m[1]), b = up(m[2]); const list = disk[parent(a)];
        const i = list ? list.findIndex(x => x.replace(/\/$/, '') === leaf(a)) : -1;
        if (i < 0) return this.out('File not found\r\nOk\r\n');
        const isDir = list[i].endsWith('/');
        list[i] = leaf(b) + (isDir ? '/' : '');
        if (isDir) { disk[b + '/'] = disk[a + '/']; delete disk[a + '/']; } else { content[b] = content[a]; delete content[a]; }
        return this.out('Ok\r\n');
      }
      if ((m = /^MKDIR\s+"([^"]*)"$/i.exec(l))) {
        const p = up(m[1]); disk[parent(p)].push(leaf(p) + '/'); disk[p + '/'] = [];
        return this.out('Ok\r\n');
      }
      if ((m = /^RMDIR\s+"([^"]*)"$/i.exec(l))) {
        const p = up(m[1]);
        if ((disk[p + '/'] || []).length) return this.out('Path/File access error\r\nOk\r\n');
        const list = disk[parent(p)]; list.splice(list.indexOf(leaf(p) + '/'), 1); delete disk[p + '/'];
        return this.out('Ok\r\n');
      }
      if ((m = /^RUN\s+"([^"]*)"$/i.exec(l)) && /BROKEN/i.test(m[1])) {
        return this.out('Starting\r\n\x1b[33mColour works\x1b[0m\r\nSyntax error in 30\r\nOk\r\n');
      }
      if ((m = /^RUN\s+"([^"]*)"$/i.exec(l)) && /LABELS/i.test(m[1])) {
        return this.out('Labels demo\r\nType mismatch in line 21\r\nOk\r\n');
      }
      if (this.waitingInput) { this.waitingInput = false; return this.out('Hello, ' + l + '!\r\nOk\r\n'); }
      if (/^RUN/i.test(l)) { this.waitingInput = true; return this.out('Hello from the mock board\r\nYour name? '); }
      return this.out('Syntax error\r\nOk\r\n');
    }
  }

  const port = new MockPort();
  let allowed = false;
  const serial = {
    requestPort: async () => { allowed = true; return port; },
    getPorts: async () => (allowed ? [port] : []),
    addEventListener() {},
  };
  Object.defineProperty(navigator, 'serial', { value: serial, configurable: true });
  window.__mockBoard = { disk, content };
})();
