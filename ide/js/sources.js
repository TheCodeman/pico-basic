// Where files live: the board's drives (FLASH:, SD:) and a folder on the PC.
// Both give the same calls, with paths like "FLASH:/GAMES/X.BAS" or
// "PC:/GAMES/X.BAS".
(function (root) {
  'use strict';

  function split(path) {
    const i = path.indexOf(':/');
    const drive = path.slice(0, i + 1);
    const rest = path.slice(i + 2);
    const parts = rest.split('/').filter(Boolean);
    return { drive, parts, name: parts[parts.length - 1] || '', folder: '/' + parts.slice(0, -1).map(p => p + '/').join('') };
  }

  class BoardSource {
    constructor(board, drive) { this.board = board; this.drive = drive; this.kind = 'board'; }
    get label() { return this.drive; }
    async list(folder) { return this.board.listFiles(this.drive + folder); }
    read(path, onProgress) { return this.board.readFile(path, onProgress); }
    write(path, data, onProgress) { return this.board.writeFile(path, data, onProgress); }
    remove(path, isDir) { return isDir ? this.board.removeFolder(path) : this.board.deleteFile(path); }
    rename(path, newName) {
      const s = split(path);
      return this.board.renameFile(path, this.drive + s.folder + newName);
    }
    mkdir(path) { return this.board.makeFolder(path); }
  }

  // ---- The PC folder: the File System Access API (Chrome, Edge) ----

  const DB = 'pico-basic-ide', STORE = 'handles';
  function idb(mode, fn) {
    return new Promise((resolve, reject) => {
      const open = indexedDB.open(DB, 1);
      open.onupgradeneeded = () => open.result.createObjectStore(STORE);
      open.onerror = () => reject(open.error);
      open.onsuccess = () => {
        const tx = open.result.transaction(STORE, mode);
        const req = fn(tx.objectStore(STORE));
        req.onsuccess = () => resolve(req.result);
        req.onerror = () => reject(req.error);
      };
    });
  }

  class PcSource {
    constructor(handle) { this.root = handle; this.drive = 'PC:'; this.kind = 'pc'; }
    get label() { return 'PC: ' + this.root.name; }

    static supported() { return 'showDirectoryPicker' in window; }

    // Asks for a folder (a click must have started this).
    static async pick() {
      const h = await window.showDirectoryPicker({ id: 'pico-basic', mode: 'readwrite' });
      try { await idb('readwrite', s => s.put(h, 'folder')); } catch (e) {}
      return new PcSource(h);
    }

    // The folder chosen last time, if the browser still has it.
    static async remembered() {
      try {
        const h = await idb('readonly', s => s.get('folder'));
        return h ? new PcSource(h) : null;
      } catch (e) { return null; }
    }

    // 'granted', or 'prompt' (then allow() from a click).
    async permission() { return this.root.queryPermission({ mode: 'readwrite' }); }
    async allow() { return (await this.root.requestPermission({ mode: 'readwrite' })) === 'granted'; }

    async dir(parts, create) {
      let d = this.root;
      for (const p of parts) d = await d.getDirectoryHandle(p, { create: !!create });
      return d;
    }

    async list(folder) {
      const d = await this.dir(folder.split('/').filter(Boolean));
      const files = [];
      for await (const [name, h] of d.entries()) {
        if (name.startsWith('.')) continue;
        if (h.kind === 'directory') files.push({ name, dir: true });
        else files.push({ name, dir: false, size: (await h.getFile()).size });
      }
      files.sort((a, b) => (b.dir - a.dir) || a.name.localeCompare(b.name));
      return { files, free: this.root.name + ' on this PC' };
    }

    async read(path) {
      const s = split(path);
      const d = await this.dir(s.parts.slice(0, -1));
      const f = await (await d.getFileHandle(s.name)).getFile();
      return new Uint8Array(await f.arrayBuffer());
    }

    async write(path, data) {
      const s = split(path);
      const d = await this.dir(s.parts.slice(0, -1), true);
      const w = await (await d.getFileHandle(s.name, { create: true })).createWritable();
      await w.write(data);
      await w.close();
    }

    async remove(path) {
      const s = split(path);
      const d = await this.dir(s.parts.slice(0, -1));
      await d.removeEntry(s.name, { recursive: true });
    }

    async rename(path, newName, isDir) {
      if (isDir) throw new Error('Renaming a folder on the PC isn\'t supported here: use your file manager.');
      const s = split(path);
      const to = this.drive + s.folder + newName;
      await this.write(to, await this.read(path));
      await this.remove(path);
    }

    async mkdir(path) {
      const s = split(path);
      await this.dir(s.parts, true);
    }
  }

  root.BoardSource = BoardSource;
  root.PcSource = PcSource;
  root.splitPath = split;
})(this);
