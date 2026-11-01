// A CodeMirror 5 mode for Pico BASIC: line numbers, labels, keywords (from
// basic-data.js, so it follows the manual), functions, strings, numbers,
// comments (REM and '), and the line or label after GOTO, GOSUB, THEN ...
(function () {
  'use strict';

  const KEYWORDS = (window.BASIC_DATA && window.BASIC_DATA.keywords) || {};
  const STYLE = { statement: 'keyword', function: 'builtin', operator: 'operator' };
  // Words after which a line number or label is a jump target.
  const JUMPS = new Set(['GOTO', 'GOSUB', 'THEN', 'ELSE', 'RESTORE', 'RESUME', 'RUN', 'LIST', 'DELETE', 'EDIT', 'RENUM']);

  CodeMirror.defineMode('picobasic', function () {
    return {
      startState: () => ({ jump: false, start: true, data: false }),

      token(stream, state) {
        if (stream.sol()) { state.start = true; state.jump = false; state.data = false; }
        if (stream.eatSpace()) return null;

        // The start of a line: a line number, or a label (name:).
        if (state.start) {
          state.start = false;
          if (stream.match(/^\d+/)) return 'linenumber';
          const m = stream.match(/^([A-Za-z_][A-Za-z0-9_.]*):(?!=)/, false);
          if (m && !KEYWORDS[m[1].toUpperCase()]) {
            stream.match(/^[A-Za-z_][A-Za-z0-9_.]*:/);
            return 'label';
          }
        }

        const ch = stream.peek();

        // DATA items run to the ':' (quoted ones may hold it).
        if (state.data && ch !== ':' && ch !== '"') {
          if (ch === ',') { stream.next(); return null; }
          stream.match(/^[^,:"]*/);
          return 'string';
        }

        if (ch === "'") { stream.skipToEnd(); return 'comment'; }

        if (ch === '"') {
          stream.next();
          while (!stream.eol()) if (stream.next() === '"') break;
          state.jump = false;
          return 'string';
        }

        // &H1F, &O17, &B101 and plain numbers (1, .5, 1.5E-3, 10#, 3%)
        if (stream.match(/^&[Hh][0-9A-Fa-f]+/) || stream.match(/^&[Oo]?[0-7]+/) || stream.match(/^&[Bb][01]+/)) {
          state.jump = false;
          return 'number';
        }
        if (stream.match(/^(\d+\.?\d*|\.\d+)([EeDd][-+]?\d+)?[!#%&]?/)) {
          if (state.jump) return 'link';  // GOTO 100
          return 'number';
        }

        const word = stream.match(/^[A-Za-z_][A-Za-z0-9_.]*[$%!#&]?/);
        if (word) {
          const w = word[0].toUpperCase();
          if (w === 'REM') { stream.skipToEnd(); return 'comment'; }
          const kind = KEYWORDS[w];
          if (kind) {
            state.jump = JUMPS.has(w);
            state.data = w === 'DATA';
            return STYLE[kind];
          }
          if (state.jump) return 'link';  // GOTO again (a label)
          return 'variable';
        }

        if (ch === ',') { stream.next(); return null; }  // ON X GOTO 10, 20
        state.jump = false;
        if (ch === ':') { stream.next(); state.data = false; return 'meta'; }
        if (ch === '?') { stream.next(); return 'keyword'; }  // PRINT
        if (stream.match(/^(<>|<=|>=|[-+*\/^\\=<>])/)) return 'operator';
        if (ch === '#') { stream.next(); return 'operator'; }  // PRINT #1
        stream.next();
        return null;
      },

      lineComment: "'",
    };
  });

  CodeMirror.defineMIME('text/x-picobasic', 'picobasic');
})();
