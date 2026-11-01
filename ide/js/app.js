// The page: toolbar, files on the left, tabs and the editor, the console and
// the help. The pieces are in the other js/ files; this joins them up.
(function () {
  'use strict';

  const { $, toast, setStatus, contextMenu, download, makeZip, looksBinary } = U;
  const board = new Board();

  const sources = {
    'FLASH:': new BoardSource(board, 'FLASH:'),
    'SD:': new BoardSource(board, 'SD:'),
    'PC:': null,  // a PcSource once a folder is chosen
  };

  const state = {
    drive: 'FLASH:',
    folders: { 'FLASH:': '/', 'SD:': '/', 'PC:': '/' },  // each drive's open folder
    files: [],
    selected: null,
    busy: false,
    running: false,
    runTab: null,      // the tab whose program is running
    runPath: null,     // the path it was RUN as
    errorMark: null,   // {tab, line} of the highlighted error line
  };

  const source = () => sources[state.drive];
  const folder = () => state.folders[state.drive];
  const fullPath = name => state.drive + folder() + name;
  const isBoardPath = p => !p.startsWith('PC:');
  const baseName = p => p.slice(p.lastIndexOf('/') + 1);

  // ---- The editor and tabs -------------------------------------------------

  const cm = CodeMirror.fromTextArea($('text'), {
    mode: 'picobasic',
    theme: 'pico',
    lineNumbers: true,
    matchBrackets: true,
    styleActiveLine: true,
    indentUnit: 2,
    tabSize: 4,
    indentWithTabs: false,
    readOnly: 'nocursor',
    extraKeys: {
      Tab: c => c.somethingSelected() ? c.indentSelection('add') : c.replaceSelection('  '),
      'Shift-Tab': c => c.indentSelection('subtract'),
      'Ctrl-/': 'toggleComment',
      'Cmd-/': 'toggleComment',
      'Ctrl-Space': c => c.showHint({ hint: Hints.hint, completeSingle: false }),
      F1: () => helpForCursor(),
    },
  });
  const tabs = new Tabs(cm, $('tabs'));
  tabs.onChange = () => { updateButtons(); renderFiles(); };
  tabs.render();
  Hints.attach(cm, $('sig'), id => { setHelp(true); help.show(id); });
  cm.on('changes', () => clearErrorMark());

  // ---- Status and buttons --------------------------------------------------

  function idleStatus() {
    if (state.running) setStatus('Running ' + baseName(state.runPath || ''), 'busy');
    else if (board.connected) setStatus('Connected', 'on');
    else setStatus('Not connected', 'off');
  }

  function updateButtons() {
    const c = board.connected;
    const t = tabs.active;
    const src = source();
    const usable = src && (src.kind === 'pc' || c);
    $('connect').textContent = c ? 'Disconnect' : 'Connect';
    $('connect').classList.toggle('primary', !c);
    $('stop').disabled = !c;
    $('refresh').disabled = !usable || (src.kind === 'board' && state.running);
    $('newfile').disabled = !usable;
    $('more').disabled = !usable;
    const canSave = t && (isBoardPath(t.path) ? c && !state.running : true) && !state.busy;
    $('save').disabled = !canSave;
    $('run').disabled = !c || !t || state.busy || state.running || !/\.BAS$/i.test(t.path);
    $('download').disabled = !t;
  }

  // A job on a disk, with the status showing what is going on.
  async function job(label, fn, onBoard = true) {
    if (onBoard && state.running) { toast('A program is running: Stop it first.', true); return undefined; }
    state.busy = true;
    setStatus(label, 'busy');
    updateButtons();
    try {
      return await fn();
    } catch (err) {
      toast(err.message, true);
      return undefined;
    } finally {
      state.busy = false;
      idleStatus();
      updateButtons();
    }
  }
  const progress = label => (n, total) => setStatus(label + ' ' + n + (total ? ' / ' + total : '') + ' bytes', 'busy');

  // ---- The console ---------------------------------------------------------

  const term = new BoardConsole($('term'), board);
  board.onConsole = text => term.write(text);
  $('clear').onclick = () => term.clear();

  term.onOk = () => {
    if (state.running) setRunning(false);
    if (listWhenReady && !state.busy) {
      listWhenReady = false;
      idleStatus();
      setTimeout(() => { if (source().kind === 'board') refresh(); }, 100);
    }
  };
  term.onError = err => {
    if (!state.running) return;
    if (/^Break/.test(err.message)) return;  // Stop: the link is there if wanted
    showError(err, true);
  };
  term.onJump = err => showError(err, false);

  // ---- Connecting ----------------------------------------------------------

  board.onState = s => {
    if (s !== 'connected') {
      state.running = false;
      if (source().kind === 'board') showHint('Connect to see the board\'s files.');
    }
    idleStatus();
    updateButtons();
  };

  // Connected but not at Ok (a program running): the list loads by itself
  // once the console sees the Ok (after Stop, or when the program ends).
  let listWhenReady = false;

  async function afterConnect() {
    const r = await board.ping().catch(() => 'busy');
    if (r !== 'ok') {
      listWhenReady = true;
      setStatus(r === 'running' ? 'Connected: a program is running' : 'Connected: the board is busy', 'busy');
      toast(r === 'running' ? 'A program is running on the board. Press Stop to get the files.'
                            : 'The board isn\'t at Ok yet (a program running?). Press Stop if so; the files load when it is.');
      return;
    }
    if (source().kind === 'board') await refresh();
  }

  $('connect').onclick = async () => {
    if (board.connected) { await board.disconnect(); return; }
    try {
      await board.connect();
    } catch (err) {
      if (err.name !== 'NotFoundError') toast(err.message, true);  // NotFound: the picker was closed
      return;
    }
    await afterConnect();
  };

  $('stop').onclick = () => board.stop().catch(err => toast(err.message, true));

  // ---- The file list -------------------------------------------------------

  function showHint(text) {
    $('filelist').innerHTML = '';
    const li = document.createElement('li');
    li.className = 'hint';
    li.textContent = text;
    $('filelist').appendChild(li);
    $('free').textContent = '';
    $('path').textContent = state.drive === 'PC:' && sources['PC:'] ? sources['PC:'].label : state.drive;
  }

  function renderFiles() {
    const src = source();
    if (!src || (src.kind === 'board' && !board.connected)) return;
    const ul = $('filelist');
    ul.innerHTML = '';
    $('path').textContent = (src.kind === 'pc' ? src.label : state.drive) + folder();
    if (folder() !== '/') addRow({ name: '..', dir: true, up: true });
    for (const f of state.files) addRow(f);
    if (!state.files.length) {
      const li = document.createElement('li');
      li.className = 'hint';
      li.textContent = 'No files here yet. Drop files from the PC here to copy them in.';
      ul.appendChild(li);
    }

    function addRow(f) {
      const li = document.createElement('li');
      const path = fullPath(f.name);
      li.className = (f.dir ? 'dir' : 'file') + (f.name === state.selected ? ' sel' : '') +
        (!f.dir && tabs.find(path) ? ' open' : '');
      li.title = f.up ? 'Up a folder' : f.dir ? 'Double-click to open the folder' : 'Double-click to edit. Right-click for more.';
      const icon = document.createElement('span');
      icon.className = 'icon';
      icon.textContent = f.up ? '↰' : f.dir ? '▸' : '·';
      const name = document.createElement('span');
      name.className = 'fname';
      name.textContent = f.name;
      li.append(icon, name);
      if (f.size != null && !f.dir) {
        const sz = document.createElement('span');
        sz.className = 'fsize';
        sz.textContent = f.size < 1024 ? f.size + ' B' : (f.size / 1024).toFixed(1) + ' K';
        li.appendChild(sz);
      }
      const select = () => {
        state.selected = f.name;
        for (const el of ul.children) el.classList.remove('sel');
        li.classList.add('sel');
      };
      li.onclick = select;
      li.ondblclick = () => {
        if (f.up) goUp();
        else if (f.dir) enter(f.name);
        else openFile(path);
      };
      li.oncontextmenu = e => {
        e.preventDefault();
        if (f.up) return;
        select();
        contextMenu(e.clientX, e.clientY, fileMenu(f, path));
      };
      ul.appendChild(li);
    }
  }

  function enter(name) { state.folders[state.drive] = folder() + name + '/'; state.selected = null; refresh(); }
  function goUp() {
    const parts = folder().split('/').filter(Boolean);
    const left = parts.pop();
    state.folders[state.drive] = '/' + parts.map(p => p + '/').join('');
    state.selected = left || null;
    refresh();
  }

  async function refresh() {
    const src = source();
    if (!src) return;
    if (src.kind === 'board' && !board.connected) return;
    await job('Listing files…', async () => {
      const { files, free } = await src.list(folder());
      state.files = files;
      renderFiles();
      $('free').textContent = free;
    }, src.kind === 'board');
  }

  $('refresh').onclick = refresh;

  $('drive').onchange = async () => {
    const want = $('drive').value;
    if (want === 'PC:' && !(await usePcFolder())) {
      $('drive').value = state.drive;
      return;
    }
    state.drive = want;
    state.selected = null;
    state.files = [];
    if (source().kind === 'board' && !board.connected) showHint('Connect to see the board\'s files.');
    updateButtons();
    refresh();
  };

  // ---- The PC folder -------------------------------------------------------

  async function usePcFolder(pickNew) {
    if (!PcSource.supported()) {
      toast('This browser can\'t open folders on the PC (Chrome or Edge can, from http://localhost).', true);
      return false;
    }
    try {
      let src = pickNew ? null : sources['PC:'] || await PcSource.remembered();
      if (src && (await src.permission()) !== 'granted' && !(await src.allow())) src = null;
      if (!src) src = await PcSource.pick();
      sources['PC:'] = src;
      $('pcoption').textContent = src.label;
      return true;
    } catch (err) {
      if (err.name !== 'AbortError') toast(err.message, true);
      return false;
    }
  }

  // ---- Opening and saving --------------------------------------------------

  function sourceOf(path) { return sources[path.slice(0, path.indexOf(':') + 1)]; }

  async function openFile(path, then) {
    const open = tabs.find(path);
    if (open) { tabs.show(open); cm.focus(); if (then) then(open); return open; }
    const src = sourceOf(path);
    return job('Opening ' + baseName(path) + '…', async () => {
      const bytes = await src.read(path, progress('Opening ' + baseName(path) + '…'));
      if (looksBinary(bytes)) throw new Error(baseName(path) + ' is not a text file (' + bytes.length + ' bytes).');
      const s = new TextDecoder().decode(bytes);
      const t = tabs.open(path, s.replace(/\r\n/g, '\n'), s.includes('\r\n') ? '\r\n' : '\n');
      cm.focus();
      if (then) then(t);
      return t;
    }, src.kind === 'board');
  }

  async function saveTab(t) {
    if (!t) return false;
    const src = sourceOf(t.path);
    const gen = t.doc.changeGeneration();
    const data = tabs.text(t);
    const ok = await job('Saving ' + baseName(t.path) + '…', async () => {
      await src.write(t.path, data, progress('Saving ' + baseName(t.path) + '…'));
      return true;
    }, src.kind === 'board');
    if (!ok) return false;
    const wasNew = t.isNew;
    tabs.saved(t, gen);
    toast('Saved ' + t.path + ' (' + data.length + ' bytes).');
    if (wasNew || !state.files.some(f => fullPath(f.name) === t.path)) refresh();
    return true;
  }

  $('save').onclick = () => saveTab(tabs.active);
  $('download').onclick = () => {
    const t = tabs.active;
    if (t) download(baseName(t.path), tabs.text(t), 'text/plain');
  };

  function askName(what, start) {
    let name = prompt(what, start);
    if (!name) return null;
    name = name.trim().replace(/^\/+|\/+$/g, '');
    if (!name || /["]/.test(name)) { toast('That name won\'t do.', true); return null; }
    return name;
  }

  $('newfile').onclick = () => {
    let name = askName('Name for the new file (in ' + state.drive + folder() + '):', 'NEW.BAS');
    if (!name) return;
    if (!name.includes('.')) name += '.BAS';
    const path = fullPath(name);
    if (state.files.some(f => f.name.toUpperCase() === name.toUpperCase()) && !confirm(name + ' is already there. Start a new one in its place (it is replaced when you save)?')) return;
    tabs.open(path, '', state.drive === 'PC:' ? '\n' : '\r\n', true);
    cm.focus();
  };

  // ---- File management -----------------------------------------------------

  function fileMenu(f, path) {
    const src = source();
    const onBoard = src.kind === 'board';
    const pc = sources['PC:'];
    const items = [];
    if (f.dir) {
      items.push({ label: 'Open folder', run: () => enter(f.name) });
      items.push({ label: 'Rename…', run: () => renameEntry(f, path), disabled: !onBoard });
      items.push('-');
      items.push({ label: 'Delete folder', danger: true, run: () => deleteEntry(f, path) });
      return items;
    }
    items.push({ label: 'Open', run: () => openFile(path) });
    if (/\.BAS$/i.test(f.name)) items.push({ label: 'Run', run: () => openFile(path, t => run(t)), disabled: !board.connected });
    items.push('-');
    items.push({ label: 'Rename…', run: () => renameEntry(f, path) });
    items.push({ label: 'Duplicate', run: () => duplicate(f, path) });
    items.push({ label: 'Download to this PC', run: () => downloadFile(path) });
    if (onBoard && pc) items.push({ label: 'Copy to ' + pc.label + folderLabel('PC:'), run: () => copyTo(path, 'PC:') });
    if (!onBoard) {
      items.push({ label: 'Copy to the board (FLASH:' + state.folders['FLASH:'] + ')', run: () => copyTo(path, 'FLASH:'), disabled: !board.connected });
    }
    items.push('-');
    items.push({ label: 'Delete', danger: true, run: () => deleteEntry(f, path) });
    return items;
  }
  const folderLabel = drive => state.folders[drive] === '/' ? '' : state.folders[drive];

  async function renameEntry(f, path) {
    const name = askName('New name for ' + f.name + ':', f.name);
    if (!name || name === f.name) return;
    const src = source();
    const ok = await job('Renaming…', async () => { await src.rename(path, name, f.dir); return true; }, src.kind === 'board');
    if (ok) {
      tabs.renamed(path, fullPath(name));
      state.selected = name;
      refresh();
    }
  }

  async function deleteEntry(f, path) {
    const what = f.dir ? 'the folder ' + f.name + (source().kind === 'pc' ? ' and everything in it' : ' (it must be empty)') : f.name;
    if (!confirm('Delete ' + what + ' from ' + state.drive + folder() + '? This can\'t be undone.')) return;
    const src = source();
    const ok = await job('Deleting…', async () => { await src.remove(path, f.dir); return true; }, src.kind === 'board');
    if (ok) {
      const t = tabs.find(path);
      if (t) { t.isNew = true; tabs.render(); }  // still open: saving puts it back
      refresh();
    }
  }

  function copyName(name) {
    const dot = name.lastIndexOf('.');
    const stem = dot > 0 ? name.slice(0, dot) : name, ext = dot > 0 ? name.slice(dot) : '';
    for (let n = 1; n < 100; n++) {
      const c = stem + (n === 1 ? '-COPY' : '-COPY' + n) + ext;
      if (!state.files.some(f => f.name.toUpperCase() === c.toUpperCase())) return c;
    }
    return stem + '-COPY' + Date.now() % 1000 + ext;
  }

  async function duplicate(f, path) {
    const src = source();
    const to = fullPath(copyName(f.name));
    const ok = await job('Copying…', async () => { await src.write(to, await src.read(path)); return true; }, src.kind === 'board');
    if (ok) { toast('Copied to ' + baseName(to) + '.'); refresh(); }
  }

  async function downloadFile(path) {
    const src = sourceOf(path);
    const data = await job('Reading ' + baseName(path) + '…', () => src.read(path, progress('Reading…')), src.kind === 'board');
    if (data) download(baseName(path), data);
  }

  async function copyTo(path, drive) {
    const from = sourceOf(path), to = sources[drive];
    if (!to) return;
    const dest = drive + state.folders[drive] + baseName(path);
    const ok = await job('Copying ' + baseName(path) + '…', async () => {
      await to.write(dest, await from.read(path), progress('Copying…'));
      return true;
    });
    if (ok) toast('Copied to ' + dest + '.');
    return ok;
  }

  async function newFolder() {
    const name = askName('Name for the new folder (in ' + state.drive + folder() + '):', 'NEWDIR');
    if (!name) return;
    const src = source();
    const ok = await job('Making the folder…', async () => { await src.mkdir(fullPath(name)); return true; }, src.kind === 'board');
    if (ok) refresh();
  }

  // Copies PC files (from a drop or the file picker) into the open folder.
  async function upload(fileList) {
    const files = [...fileList].filter(f => f.size >= 0);
    if (!files.length) return;
    const src = source();
    if (src.kind === 'board' && !board.connected) { toast('Connect first.', true); return; }
    let done = 0;
    await job('Copying…', async () => {
      for (const f of files) {
        const data = new Uint8Array(await f.arrayBuffer());
        setStatus('Copying ' + f.name + ' (' + (done + 1) + ' of ' + files.length + ')…', 'busy');
        await src.write(fullPath(f.name), data, progress('Copying ' + f.name + '…'));
        done++;
      }
    }, src.kind === 'board');
    if (done) toast('Copied ' + done + ' file' + (done > 1 ? 's' : '') + ' to ' + state.drive + folder() + '.');
    refresh();
  }

  $('uploadinput').onchange = () => { upload($('uploadinput').files); $('uploadinput').value = ''; };

  const filesPane = $('files');
  let dragDepth = 0;
  filesPane.addEventListener('dragenter', e => {
    if (![...e.dataTransfer.types].includes('Files')) return;
    e.preventDefault();
    if (dragDepth++ === 0) { filesPane.classList.add('drop'); $('dropnote').hidden = false; }
  });
  filesPane.addEventListener('dragover', e => { if ([...e.dataTransfer.types].includes('Files')) e.preventDefault(); });
  filesPane.addEventListener('dragleave', () => {
    if (--dragDepth <= 0) { dragDepth = 0; filesPane.classList.remove('drop'); $('dropnote').hidden = true; }
  });
  filesPane.addEventListener('drop', e => {
    e.preventDefault();
    dragDepth = 0;
    filesPane.classList.remove('drop');
    $('dropnote').hidden = true;
    upload(e.dataTransfer.files);
  });

  // Every file on a board drive into one zip.
  async function backup() {
    const src = source();
    const files = [];
    const ok = await job('Backing up…', async () => {
      async function walk(dir) {
        const { files: list } = await src.list(dir);
        for (const f of list) {
          if (f.dir) { await walk(dir + f.name + '/'); continue; }
          setStatus('Backing up ' + dir + f.name + ' (' + (files.length + 1) + ')…', 'busy');
          files.push({ name: dir.slice(1) + f.name, data: await src.read(src.drive + dir + f.name) });
        }
      }
      await walk('/');
      return true;
    });
    if (!ok) return;
    const day = new Date().toISOString().slice(0, 10);
    download('pico-basic-' + state.drive.replace(':', '') + '-' + day + '.zip', makeZip(files));
    toast('Backed up ' + files.length + ' files.');
    refresh();
  }

  // Every file in the PC folder (not its subfolders) to the board's folder.
  async function pushAll() {
    const pc = sources['PC:'], dest = state.folders['FLASH:'];
    const { files } = await pc.list(folder());
    const list = files.filter(f => !f.dir);
    if (!list.length) { toast('No files in this folder.'); return; }
    if (!confirm('Copy ' + list.length + ' files from ' + pc.label + folder() + ' to FLASH:' + dest + '? Files of the same names there are replaced.')) return;
    let n = 0;
    await job('Copying to the board…', async () => {
      for (const f of list) {
        setStatus('Copying ' + f.name + ' (' + (n + 1) + ' of ' + list.length + ')…', 'busy');
        await sources['FLASH:'].write('FLASH:' + dest + f.name, await pc.read(fullPath(f.name)));
        n++;
      }
    });
    toast('Copied ' + n + ' files to FLASH:' + dest + '.');
  }

  $('more').onclick = e => {
    const src = source();
    const r = e.target.getBoundingClientRect();
    const items = [
      { label: 'New folder…', run: newFolder },
      { label: 'Copy files from this PC…', run: () => $('uploadinput').click() },
    ];
    if (src.kind === 'board') {
      items.push('-', { label: 'Back up ' + state.drive + ' to a zip', run: backup });
    } else {
      items.push('-', { label: 'Copy all these files to the board', run: pushAll, disabled: !board.connected });
      items.push({ label: 'Choose another PC folder…', run: async () => { if (await usePcFolder(true)) { state.folders['PC:'] = '/'; refresh(); } } });
    }
    contextMenu(r.left, r.bottom + 4, items);
  };

  // ---- Running a program ---------------------------------------------------

  function setRunning(on) {
    state.running = on;
    idleStatus();
    updateButtons();
  }

  async function run(t = tabs.active) {
    if (!t || !board.connected || state.busy || state.running) return;
    if (tabs.dirty(t) && !(await saveTab(t))) return;
    let path = t.path;
    if (!isBoardPath(path)) {
      // A PC file runs from the board: copy it there first.
      const dest = 'FLASH:' + state.folders['FLASH:'] + baseName(path);
      const ok = await job('Copying ' + baseName(path) + ' to the board…', async () => {
        await sources['FLASH:'].write(dest, tabs.text(t), progress('Copying…'));
        return true;
      });
      if (!ok) return;
      path = dest;
    }
    clearErrorMark();
    state.runTab = t;
    state.runPath = path;
    setRunning(true);
    term.write('\r\n');
    try {
      await board.write('RUN "' + path + '"\r');
    } catch (err) {
      setRunning(false);
      toast(err.message, true);
      return;
    }
    term.focus();
  }

  $('run').onclick = () => run();
  document.addEventListener('keydown', e => {
    if (e.key === 'F5') { e.preventDefault(); run(); }
  });

  // ---- Errors: jump to the line ------------------------------------------

  function clearErrorMark() {
    const m = state.errorMark;
    if (!m) return;
    state.errorMark = null;
    m.tab.doc.removeLineClass(m.handle, 'background', 'error-line');
  }

  // The editor line for a BASIC line: "in 40" is the line numbered 40,
  // "in line 12" (a program without numbers) is the 12th line of the file.
  function editorLine(doc, err) {
    if (!err.numbered) return Math.min(err.line - 1, doc.lineCount() - 1);
    const re = new RegExp('^\\s*' + err.line + '(\\D|$)');
    for (let i = 0; i < doc.lineCount(); i++) if (re.test(doc.getLine(i))) return i;
    return -1;
  }

  async function showError(err, auto) {
    let t = state.runTab || tabs.active;
    if (err.library) {
      // An error in an INCLUDEd file: it sits next to the program.
      const dir = (state.runPath || t.path).replace(/[^/]*$/, '');
      const name = /\./.test(err.library) ? err.library : err.library + '.BAS';
      t = await openFile(dir + name);
      if (!t) return;
    }
    if (!t) return;
    const line = editorLine(t.doc, err);
    if (line < 0) { toast(err.message + ': line ' + err.line + ' isn\'t in ' + baseName(t.path) + '.', true); return; }
    const go = () => {
      if (tabs.active !== t) tabs.show(t);
      clearErrorMark();
      const handle = t.doc.addLineClass(line, 'background', 'error-line');
      state.errorMark = { tab: t, handle };
      cm.setCursor(line, 0);
      cm.scrollIntoView({ line, ch: 0 }, 80);
      cm.focus();
    };
    if (auto) {
      go();
      toast(err.message + (err.library ? ' in ' + err.library : '') + ', line ' + (line + 1) + ' of the editor.', { error: true });
    } else {
      go();
    }
  }

  // ---- Help ----------------------------------------------------------------

  const help = new Help($('help'), $('helplist'), $('helpbody'), $('helpsearch'));

  function setHelp(open) {
    $('help').hidden = !open;
    document.querySelector('.layout').classList.toggle('with-help', open);
    $('helpbtn').classList.toggle('primary', open);
    cm.refresh();
    term.refit();
  }

  function wordsAtCursor() {
    const pos = cm.getCursor();
    const line = cm.getLine(pos.line) || '';
    const isW = c => /[A-Za-z0-9_$]/.test(c || '');
    let a = pos.ch, b = pos.ch;
    while (a > 0 && isW(line[a - 1])) a--;
    while (b < line.length && isW(line[b])) b++;
    const word = line.slice(a, b);
    const before = (line.slice(0, a).match(/([A-Za-z$]+)\s*$/) || [])[1];
    const after = (line.slice(b).match(/^\s*([A-Za-z$]+)/) || [])[1];
    return [word, before, after];
  }

  function helpForCursor() {
    setHelp(true);
    const [word, before, after] = wordsAtCursor();
    if (word && !help.showWord(word, before, after)) {
      $('helpsearch').value = word;
      help.renderList(word);
      toast('No entry for ' + word + ' in the reference.');
    }
  }

  $('helpbtn').onclick = () => {
    const open = $('help').hidden;
    setHelp(open);
    if (open) $('helpsearch').focus();
  };
  $('helpclose').onclick = () => { setHelp(false); cm.focus(); };
  document.addEventListener('keydown', e => {
    if (e.key === 'F1') { e.preventDefault(); if (!cm.hasFocus()) setHelp(true); }
    if (e.key === 'Escape' && !$('help').hidden && $('help').contains(document.activeElement)) { setHelp(false); cm.focus(); }
    if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === 's') { e.preventDefault(); saveTab(tabs.active); }
  });

  window.addEventListener('beforeunload', e => {
    if (tabs.anyDirty()) { e.preventDefault(); e.returnValue = ''; }
  });

  // ---- Start ---------------------------------------------------------------

  if (!('serial' in navigator)) {
    $('nosupport').hidden = false;
    $('connect').disabled = true;
  }
  if (!PcSource.supported()) $('pcoption').disabled = true;
  PcSource.remembered().then(src => { if (src) $('pcoption').textContent = src.label; });
  showHint('Connect to see the board\'s files.');
  updateButtons();
  // A port allowed before connects again without the picker.
  board.reconnect().then(ok => { if (ok) afterConnect(); }).catch(() => {});
})();
