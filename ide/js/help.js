// The help panel: the manual's A-Z reference (basic-data.js), searchable,
// with F1 on a word in the editor opening its entry.
(function (root) {
  'use strict';

  const DATA = root.BASIC_DATA || { entries: [], keywords: {} };
  const byId = new Map(DATA.entries.map(e => [e.id, e]));

  function esc(s) {
    return s.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;');
  }

  // Inline markdown: `code`, **bold**, *italic*, [text](#ref-x).
  function inline(s) {
    const parts = s.split(/(`[^`]*`)/);
    return parts.map(p => {
      if (p.startsWith('`') && p.endsWith('`') && p.length > 1) return '<code>' + esc(p.slice(1, -1)) + '</code>';
      return esc(p)
        .replace(/\*\*([^*]+)\*\*/g, '<strong>$1</strong>')
        .replace(/\*([^*]+)\*/g, '<em>$1</em>')
        .replace(/\[([^\]]+)\]\((#ref-[^)]+)\)/g, '<a href="$2" data-ref="$2">$1</a>')
        .replace(/\[([^\]]+)\]\(([^)]+)\)/g, '$1');
    }).join('');
  }

  // Coloured code, as in the editor.
  function codeBlock(text) {
    const pre = document.createElement('pre');
    pre.className = 'cm-s-pico help-code';
    if (root.CodeMirror && CodeMirror.runMode) CodeMirror.runMode(text, 'picobasic', pre);
    else pre.textContent = text;
    return pre.outerHTML;
  }

  // The block markdown the reference uses: paragraphs, - lists, ``` code,
  // | tables |.
  function render(md) {
    const lines = md.split('\n');
    let html = '';
    for (let i = 0; i < lines.length;) {
      const l = lines[i];
      if (l.startsWith('```')) {
        const code = [];
        for (i++; i < lines.length && !lines[i].startsWith('```'); i++) code.push(lines[i]);
        i++;
        html += codeBlock(code.join('\n'));
      } else if (/^\s*- /.test(l)) {
        html += '<ul>';
        while (i < lines.length && /^\s*- /.test(lines[i])) {
          let item = lines[i].replace(/^\s*- /, '');
          for (i++; i < lines.length && /^\s{2,}\S/.test(lines[i]) && !/^\s*- /.test(lines[i]); i++) item += ' ' + lines[i].trim();
          html += '<li>' + inline(item) + '</li>';
        }
        html += '</ul>';
      } else if (l.startsWith('|')) {
        const rows = [];
        for (; i < lines.length && lines[i].startsWith('|'); i++) rows.push(lines[i]);
        const cells = r => r.replace(/^\||\|$/g, '').split('|').map(c => c.trim());
        html += '<table><thead><tr>' + cells(rows[0]).map(c => '<th>' + inline(c) + '</th>').join('') + '</tr></thead><tbody>';
        for (const r of rows.slice(2)) html += '<tr>' + cells(r).map(c => '<td>' + inline(c) + '</td>').join('') + '</tr>';
        html += '</tbody></table>';
      } else if (!l.trim()) {
        i++;
      } else {
        const para = [];
        for (; i < lines.length && lines[i].trim() && !/^(```|\||\s*- )/.test(lines[i]); i++) para.push(lines[i]);
        html += '<p>' + inline(para.join(' ')) + '</p>';
      }
    }
    return html;
  }

  // The entry for a word (and the words either side of it, for LINE INPUT,
  // ON ERROR GOTO, PRINT USING ...).
  function find(word, before, after) {
    const W = (word || '').toUpperCase();
    if (!W) return null;
    const B = (before || '').toUpperCase(), A = (after || '').toUpperCase();
    const titles = DATA.entries;
    const tries = [];
    if (B && A) tries.push(B + ' ' + W + ' ' + A);
    if (B) tries.push(B + ' ' + W);
    if (A) tries.push(W + ' ' + A);
    tries.push(W, W + '$');
    for (const t of tries) {
      const e = titles.find(x => x.title.toUpperCase() === t);
      if (e) return e;
    }
    // A title that starts with it: ON ERROR GOTO for ON, EXIT FOR for EXIT.
    return titles.find(x => x.title.toUpperCase().startsWith(W + ' ') || x.title.toUpperCase().startsWith(W + '(')) || null;
  }

  class Help {
    constructor(panel, list, body, search) {
      this.panel = panel; this.list = list; this.body = body; this.search = search;
      this.history = [];
      this.renderList('');
      search.addEventListener('input', () => this.renderList(search.value));
      search.addEventListener('keydown', e => {
        if (e.key === 'Enter') {
          const first = this.list.querySelector('li');
          if (first) this.show(first.dataset.id);
        }
      });
      body.addEventListener('click', e => {
        const a = e.target.closest('a[data-ref]');
        if (a) { e.preventDefault(); this.show(a.dataset.ref.slice(1)); }
      });
    }

    renderList(q) {
      const Q = q.trim().toUpperCase();
      const hits = DATA.entries.filter(e => !Q || e.title.toUpperCase().includes(Q));
      // Titles that start with the search come first.
      hits.sort((a, b) => (!a.title.toUpperCase().startsWith(Q)) - (!b.title.toUpperCase().startsWith(Q)));
      this.list.innerHTML = hits.map(e =>
        '<li data-id="' + e.id + '"><span class="t">' + esc(e.title) + '</span><span class="k">' + esc(e.kind) + '</span></li>').join('');
      for (const li of this.list.children) li.onclick = () => this.show(li.dataset.id);
      this.list.hidden = false;
    }

    show(id) {
      const e = byId.get(id);
      if (!e) return false;
      this.body.innerHTML = '<h2>' + esc(e.title) + '</h2>' + render(e.body);
      this.body.scrollTop = 0;
      this.current = id;
      for (const li of this.list.children) li.classList.toggle('sel', li.dataset.id === id);
      const sel = this.list.querySelector('li.sel');
      if (sel) sel.scrollIntoView({ block: 'nearest' });
      return true;
    }

    showWord(word, before, after) {
      const e = find(word, before, after);
      if (e) {
        if (this.search.value) { this.search.value = ''; this.renderList(''); }
        this.show(e.id);
        return true;
      }
      return false;
    }
  }

  root.Help = Help;
  root.helpFind = find;
})(this);
