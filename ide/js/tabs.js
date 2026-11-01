// Tabs: each open file is a CodeMirror document, swapped into the one editor.
(function (root) {
  'use strict';

  function modeFor(path) { return /\.BAS$/i.test(path) ? 'picobasic' : null; }
  const baseName = p => p.slice(p.lastIndexOf('/') + 1);

  class Tabs {
    constructor(cm, bar) {
      this.cm = cm;
      this.bar = bar;
      this.tabs = [];        // {path, doc, eol, cleanGen, isNew}
      this.active = null;
      this.onChange = () => {};  // the active tab, or a tab's dot, changed
      cm.on('changes', () => this.render());
    }

    find(path) { return this.tabs.find(t => t.path === path); }

    dirty(t) { return !!t && (t.isNew || !t.doc.isClean(t.cleanGen)); }
    anyDirty() { return this.tabs.some(t => this.dirty(t)); }

    // Opens (or shows) a file with this text.
    open(path, text, eol, isNew) {
      let t = this.find(path);
      if (!t) {
        const doc = CodeMirror.Doc(text, modeFor(path));
        t = { path, doc, eol: eol || '\n', cleanGen: doc.changeGeneration(), isNew: !!isNew };
        this.tabs.push(t);
      }
      this.show(t);
      return t;
    }

    show(t) {
      this.active = t;
      if (t) this.cm.swapDoc(t.doc);
      else this.cm.swapDoc(CodeMirror.Doc('', null));
      this.cm.setOption('readOnly', t ? false : 'nocursor');
      this.render();
      this.onChange();
    }

    close(t) {
      if (this.dirty(t) && !confirm(t.path + ' has changes that are not saved. Close it anyway?')) return false;
      const i = this.tabs.indexOf(t);
      this.tabs.splice(i, 1);
      if (this.active === t) this.show(this.tabs[Math.min(i, this.tabs.length - 1)] || null);
      else this.render();
      return true;
    }

    // After a save: the text as it was when the save began is the clean one.
    saved(t, gen, newPath) {
      t.cleanGen = gen;
      t.isNew = false;
      if (newPath) {
        t.path = newPath;
        if (t === this.active) this.cm.setOption('mode', modeFor(newPath));
      }
      this.render();
      this.onChange();
    }

    // A file was renamed or deleted on the disk.
    renamed(from, to) {
      for (const t of this.tabs) {
        if (t.path === from) t.path = to;
        else if (t.path.startsWith(from + '/')) t.path = to + t.path.slice(from.length);
      }
      this.render();
      this.onChange();
    }

    text(t) {
      let body = t.doc.getValue();
      if (body && !body.endsWith('\n')) body += '\n';
      return new TextEncoder().encode(body.replace(/\n/g, t.eol));
    }

    render() {
      this.bar.innerHTML = '';
      if (!this.tabs.length) {
        const s = document.createElement('span');
        s.className = 'tab-empty';
        s.textContent = 'No file open: double-click one on the left.';
        this.bar.appendChild(s);
      }
      for (const t of this.tabs) {
        const el = document.createElement('div');
        el.className = 'tab' + (t === this.active ? ' active' : '') + (t.path.startsWith('PC:') ? ' pc' : '');
        el.title = t.path + (t.isNew ? ' (new)' : '');
        const name = document.createElement('span');
        name.className = 'tab-name';
        name.textContent = (t.path.startsWith('PC:') ? 'PC: ' : '') + baseName(t.path);
        const dot = document.createElement('span');
        dot.className = 'tab-dot';
        dot.textContent = this.dirty(t) ? '●' : '';
        const x = document.createElement('button');
        x.className = 'tab-x';
        x.textContent = '✕';
        x.title = 'Close';
        x.onclick = e => { e.stopPropagation(); this.close(t); };
        el.append(name, dot, x);
        el.onclick = () => { if (t !== this.active) this.show(t); this.cm.focus(); };
        el.onauxclick = e => { if (e.button === 1) { e.preventDefault(); this.close(t); } };
        this.bar.appendChild(el);
      }
      const a = this.bar.querySelector('.tab.active');
      if (a) a.scrollIntoView({ block: 'nearest', inline: 'nearest' });
    }
  }

  root.Tabs = Tabs;
  root.modeFor = modeFor;
})(this);
