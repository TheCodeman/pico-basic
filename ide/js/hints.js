// Typing help: keyword (and variable and label) completion, and a line under
// the editor with the syntax of the statement or function the cursor is in,
// both from the manual's reference (basic-data.js).
(function (root) {
  'use strict';

  const DATA = root.BASIC_DATA || { entries: [], keywords: {} };
  const KEYWORDS = Object.keys(DATA.keywords).sort();

  // Each entry's kind and syntax line ("*Statement* · `PWM pin, freq[, duty]`").
  const SYNTAX = new Map();
  for (const e of DATA.entries) {
    const m = /^\*(\w+)\*\s*·\s*(.*)$/m.exec(e.body);
    if (m) SYNTAX.set(e.title.toUpperCase(), { title: e.title, kind: m[1], syntax: m[2], id: e.id });
  }

  function esc(s) { return s.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;'); }
  function syntaxHtml(s) {
    return s.split(/(`[^`]*`)/).map(p => p.startsWith('`') ? '<code>' + esc(p.slice(1, -1)) + '</code>' : esc(p)).join('');
  }

  // Names the program uses (variables, labels, SUBs), for completion too.
  function namesIn(cm) {
    const seen = new Set();
    const re = /[A-Za-z_][A-Za-z0-9_.]*[$%!#&]?/g;
    cm.eachLine(l => {
      const text = l.text.replace(/"[^"]*"?/g, '').replace(/('|\bREM\b).*$/i, '');
      for (const m of text.matchAll(re)) if (!DATA.keywords[m[0].toUpperCase()] && m[0].length > 1) seen.add(m[0]);
    });
    return [...seen];
  }

  function wordBefore(cm) {
    const cur = cm.getCursor();
    const line = cm.getLine(cur.line);
    let a = cur.ch;
    while (a > 0 && /[A-Za-z0-9_$.]/.test(line[a - 1])) a--;
    return { word: line.slice(a, cur.ch), from: CodeMirror.Pos(cur.line, a), to: cur };
  }

  function hint(cm) {
    const { word, from, to } = wordBefore(cm);
    if (!word || /^\d/.test(word)) return null;
    const W = word.toUpperCase();
    const list = [];
    for (const k of KEYWORDS) {
      if (k.startsWith(W) && k !== W) {
        const s = SYNTAX.get(k);
        list.push({ text: k, displayText: k, className: 'hint-kw', render: (el) => {
          el.innerHTML = '<span class="h-word">' + esc(k) + '</span><span class="h-kind">' + esc(s ? s.kind : DATA.keywords[k]) + '</span>';
        } });
      }
    }
    for (const n of namesIn(cm)) {
      if (n.toUpperCase().startsWith(W) && n !== word) {
        list.push({ text: n, displayText: n, render: (el) => {
          el.innerHTML = '<span class="h-word">' + esc(n) + '</span><span class="h-kind">in this file</span>';
        } });
      }
    }
    if (!list.length) return null;
    return { list: list.slice(0, 40), from, to };
  }

  // Is the cursor in a string or a comment (no completion there)?
  function inTextOrComment(cm) {
    const t = cm.getTokenTypeAt(cm.getCursor());
    return t === 'string' || t === 'comment';
  }

  function attach(cm, sigEl, onHelp) {
    cm.on('inputRead', (cm, change) => {
      if (cm.getOption('mode') !== 'picobasic' || change.origin !== '+input') return;
      if (!/^[A-Za-z]$/.test(change.text[0])) return;
      if (inTextOrComment(cm)) return;
      if (wordBefore(cm).word.length < 2) return;
      cm.showHint({ hint, completeSingle: false, closeCharacters: /[\s()\[\]{};:>,="]/ });
    });

    const update = () => {
      const s = cm.getOption('mode') === 'picobasic' ? syntaxAt(cm) : null;
      if (!s) { sigEl.innerHTML = '<span class="sig-hint">Ctrl-Space completes a word. F1 shows help for the word at the cursor.</span>'; return; }
      sigEl.innerHTML = '<span class="sig-kind">' + esc(s.kind) + '</span> ' + syntaxHtml(s.syntax) +
        ' <a href="#" class="sig-more">more</a>';
      sigEl.querySelector('.sig-more').onclick = e => { e.preventDefault(); onHelp(s.id); };
    };
    cm.on('cursorActivity', update);
    cm.on('swapDoc', update);
    update();
  }

  // The function call the cursor is inside, else the statement it is in.
  function syntaxAt(cm) {
    const cur = cm.getCursor();
    const line = cm.getLine(cur.line).slice(0, cur.ch);
    // Blank out strings, so their brackets and colons don't count.
    const plain = line.replace(/"[^"]*("|$)/g, m => ' '.repeat(m.length));
    if (/('|\bREM\b)/i.test(plain.replace(/"[^"]*"/g, ''))) return null;
    let depth = 0;
    for (let i = plain.length - 1; i >= 0; i--) {
      const c = plain[i];
      if (c === ')') depth++;
      else if (c === '(') {
        if (depth === 0) {
          const m = /([A-Za-z][A-Za-z0-9]*\$?)\s*$/.exec(plain.slice(0, i));
          const s = m && SYNTAX.get(m[1].toUpperCase());
          if (s) return s;
        } else depth--;
      } else if (c === ':' && depth === 0) break;
    }
    // The statement: its first word (and two words: LINE INPUT, ON ERROR GOTO).
    const stmt = plain.slice(plain.lastIndexOf(':') + 1).replace(/^\s*\d+\s*/, '').replace(/^\s*[A-Za-z_][A-Za-z0-9_.]*:\s*/, '');
    const words = (stmt.match(/[A-Za-z]+\$?|#/g) || []).map(w => w.toUpperCase());
    for (const n of [3, 2, 1]) {
      if (words.length >= n) {
        const s = SYNTAX.get(words.slice(0, n).join(' '));
        if (s) return s;
      }
    }
    // A word being typed that names something.
    const w = wordBefore(cm).word.toUpperCase();
    return SYNTAX.get(w) || null;
  }

  root.Hints = { attach, hint };
})(this);
