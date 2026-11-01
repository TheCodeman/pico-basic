// Small helpers the other files share: the toast, the status line, a context
// menu, downloads and a zip writer (for backups).
(function (root) {
  'use strict';

  const $ = id => document.getElementById(id);

  let toastTimer = null;
  // msg: text; opts.error, opts.action = {label, run}
  function toast(msg, opts = {}) {
    if (opts === true) opts = { error: true };
    const t = $('toast');
    t.innerHTML = '';
    const span = document.createElement('span');
    span.textContent = msg;
    t.appendChild(span);
    if (opts.action) {
      const b = document.createElement('button');
      b.textContent = opts.action.label;
      b.onclick = () => { t.hidden = true; opts.action.run(); };
      t.appendChild(b);
    }
    t.className = 'toast' + (opts.error ? ' error' : '');
    t.onclick = e => { if (e.target === t || e.target === span) t.hidden = true; };  // a click closes it
    t.hidden = false;
    clearTimeout(toastTimer);
    toastTimer = setTimeout(() => { t.hidden = true; }, opts.action ? 12000 : opts.error ? 7000 : 3000);
  }

  function setStatus(text, cls) {
    const s = $('status');
    s.textContent = text;
    s.className = 'status ' + cls;
  }

  // A menu at the mouse: items = [{label, run, disabled, danger} | '-'].
  function contextMenu(x, y, items) {
    closeMenu();
    const m = document.createElement('div');
    m.className = 'menu';
    m.id = 'menu';
    for (const it of items) {
      if (it === '-') { m.appendChild(document.createElement('hr')); continue; }
      const b = document.createElement('button');
      b.textContent = it.label;
      b.disabled = !!it.disabled;
      if (it.danger) b.classList.add('danger');
      b.onclick = () => { closeMenu(); it.run(); };
      m.appendChild(b);
    }
    document.body.appendChild(m);
    const r = m.getBoundingClientRect();
    m.style.left = Math.min(x, innerWidth - r.width - 8) + 'px';
    m.style.top = Math.min(y, innerHeight - r.height - 8) + 'px';
    setTimeout(() => document.addEventListener('mousedown', outside), 0);
    function outside(e) { if (!m.contains(e.target)) closeMenu(); }
    m._outside = outside;
  }
  function closeMenu() {
    const m = $('menu');
    if (m) { document.removeEventListener('mousedown', m._outside); m.remove(); }
  }
  document.addEventListener('keydown', e => { if (e.key === 'Escape') closeMenu(); });

  function download(name, data, type = 'application/octet-stream') {
    const blob = data instanceof Blob ? data : new Blob([data], { type });
    const a = document.createElement('a');
    a.href = URL.createObjectURL(blob);
    a.download = name;
    a.click();
    setTimeout(() => URL.revokeObjectURL(a.href), 2000);
  }

  // ---- A zip file (stored, not compressed: the files are small) ----

  const CRC_TABLE = (() => {
    const t = new Uint32Array(256);
    for (let n = 0; n < 256; n++) {
      let c = n;
      for (let k = 0; k < 8; k++) c = c & 1 ? 0xedb88320 ^ (c >>> 1) : c >>> 1;
      t[n] = c >>> 0;
    }
    return t;
  })();
  function crc32(data) {
    let c = 0xffffffff;
    for (let i = 0; i < data.length; i++) c = CRC_TABLE[(c ^ data[i]) & 0xff] ^ (c >>> 8);
    return (c ^ 0xffffffff) >>> 0;
  }

  // files: [{name: 'GAMES/X.BAS', data: Uint8Array}] -> Blob
  function makeZip(files) {
    const enc = new TextEncoder();
    const parts = [], central = [];
    let offset = 0;
    const d = new Date();
    const time = (d.getHours() << 11) | (d.getMinutes() << 5) | (d.getSeconds() >> 1);
    const date = ((d.getFullYear() - 1980) << 9) | ((d.getMonth() + 1) << 5) | d.getDate();
    for (const f of files) {
      const name = enc.encode(f.name);
      const crc = crc32(f.data);
      const local = new DataView(new ArrayBuffer(30));
      local.setUint32(0, 0x04034b50, true);
      local.setUint16(4, 20, true);
      local.setUint16(6, 0x0800, true);  // UTF-8 names
      local.setUint16(10, time, true);
      local.setUint16(12, date, true);
      local.setUint32(14, crc, true);
      local.setUint32(18, f.data.length, true);
      local.setUint32(22, f.data.length, true);
      local.setUint16(26, name.length, true);
      parts.push(local.buffer, name, f.data);
      const cen = new DataView(new ArrayBuffer(46));
      cen.setUint32(0, 0x02014b50, true);
      cen.setUint16(4, 20, true);
      cen.setUint16(6, 20, true);
      cen.setUint16(8, 0x0800, true);
      cen.setUint16(12, time, true);
      cen.setUint16(14, date, true);
      cen.setUint32(16, crc, true);
      cen.setUint32(20, f.data.length, true);
      cen.setUint32(24, f.data.length, true);
      cen.setUint16(28, name.length, true);
      cen.setUint32(42, offset, true);
      central.push(cen.buffer, name);
      offset += 30 + name.length + f.data.length;
    }
    const cenSize = central.reduce((n, p) => n + p.byteLength, 0);
    const end = new DataView(new ArrayBuffer(22));
    end.setUint32(0, 0x06054b50, true);
    end.setUint16(8, files.length, true);
    end.setUint16(10, files.length, true);
    end.setUint32(12, cenSize, true);
    end.setUint32(16, offset, true);
    return new Blob([...parts, ...central, end.buffer], { type: 'application/zip' });
  }

  function looksBinary(bytes) {
    const n = Math.min(bytes.length, 4096);
    for (let i = 0; i < n; i++) if (bytes[i] === 0) return true;
    return false;
  }

  root.U = { $, toast, setStatus, contextMenu, closeMenu, download, makeZip, crc32, looksBinary };
})(this);
