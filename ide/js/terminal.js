// The console: a real terminal (xterm.js) on the board's serial line. Keys
// go straight to the board (so INKEY$, arrow keys and the line editor's
// history work), and the board's ANSI colours and LOCATE show as they would
// in picocom. It also watches the text for "Ok" (a program has finished)
// and for "... in 40" / "... in line 12" (an error, which it can jump to).
(function (root) {
  'use strict';

  const ERR_RE = /^(.+?) in (line )?(\d+)(?: of (\S+))?\s*$/;

  class Console {
    constructor(el, board) {
      this.board = board;
      this.onOk = () => {};          // the board is back at Ok
      this.onError = () => {};       // {message, line, numbered, library}
      this.onJump = () => {};        // a click on an error in the terminal
      this.tail = '';                // the last few hundred characters, plain
      this.term = new Terminal({
        fontFamily: 'ui-monospace, "JetBrains Mono", "DejaVu Sans Mono", Consolas, monospace',
        fontSize: 13,
        cursorBlink: true,
        convertEol: false,
        scrollback: 5000,
        allowProposedApi: true,
        theme: this.theme(),
      });
      this.fit = new FitAddon.FitAddon();
      this.term.loadAddon(this.fit);
      this.term.open(el);
      this.refit();
      new ResizeObserver(() => this.refit()).observe(el);
      matchMedia('(prefers-color-scheme: light)').addEventListener('change', () => { this.term.options.theme = this.theme(); });

      // What is typed goes to the board, unless a file transfer has the line.
      this.term.onData(d => {
        if (!board.connected || board.capture) return;
        board.write(d).catch(() => {});
      });

      // Errors in the text are links to their line.
      this.term.registerLinkProvider({
        provideLinks: (y, callback) => {
          const line = this.term.buffer.active.getLine(y - 1);
          const text = line ? line.translateToString(true) : '';
          const m = ERR_RE.exec(text);
          if (!m) return callback(undefined);
          const at = text.lastIndexOf(' in ' + (m[2] || '') + m[3]);
          callback([{
            range: { start: { x: at + 2, y }, end: { x: text.trimEnd().length, y } },
            text: text.slice(at + 1),
            activate: () => this.onJump(this.parseError(m)),
          }]);
        },
      });
    }

    theme() {
      const light = matchMedia('(prefers-color-scheme: light)').matches;
      return light
        ? { background: '#f5f6f8', foreground: '#1f2328', cursor: '#1f6fd1', selectionBackground: '#cfe0f7' }
        : { background: '#1e1f22', foreground: '#d8dade', cursor: '#4fa3ff', selectionBackground: '#33435a' };
    }

    refit() { try { this.fit.fit(); } catch (e) {} }
    focus() { this.term.focus(); }
    clear() { this.term.clear(); }

    parseError(m) {
      return { message: m[1], line: +m[3], numbered: !m[2], library: m[4] || null };
    }

    // Text from the board.
    write(text) {
      this.term.write(text);
      const plain = text
        .replace(/\x1b\[[0-9;?]*[ -\/]*[@-~]/g, '')
        .replace(/\x1b[@-_]/g, '')
        .replace(/\r\n/g, '\n')
        .replace(/\r/g, '\n');
      const before = this.tail;
      this.tail = (this.tail + plain).slice(-600);
      // Each line that has just been finished.
      const done = this.tail.split('\n');
      done.pop();
      const fresh = done.slice(-Math.max(1, (plain.match(/\n/g) || []).length));
      for (const l of fresh) {
        const m = ERR_RE.exec(l.trim());
        if (m && !/^(Ok|Sent|Received)/.test(m[1])) this.onError(this.parseError(m));
      }
      if (/(^|\n)Ok\n$/.test(this.tail) && !/(^|\n)Ok\n$/.test(before)) this.onOk();
    }
  }

  root.BoardConsole = Console;
})(this);
