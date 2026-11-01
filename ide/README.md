# Pico BASIC IDE (browser)

A page for editing the files on a Pico BASIC board from the PC, over the
board's USB serial port (Chrome's Web Serial). Nothing to install, and no
change to the firmware: it types `FILES`, `XSEND` and `XRECV` at the `Ok`
prompt as you would.

## Running it

It needs Chrome, Edge or another Chromium browser (Firefox and Safari have no
Web Serial). Serve the folder on localhost and open it:

```bash
cd ide
python3 -m http.server 8000
# then open http://localhost:8000 in Chrome
```

(Opening `index.html` straight from the disk works in Chrome too, except the PC folder.)

Click **Connect** and pick the Pico. On Linux your user must be in the
`dialout` group to open `/dev/ttyACM*` (`sudo usermod -aG dialout $USER`, then
log in again). Close any other terminal (minicom, picocom) on the port first:
only one program can have it open. Next time the page connects by itself.

## What it does

- **Files** on the left: the board's `FLASH:` or `SD:` drive, or a **folder on
  this PC** (pick "PC folder…": Chrome asks which folder, and remembers it).
  Double-click a folder to go in (`..` comes back out) and a file to open it.
  **Right-click** a file to run, rename, duplicate, delete or download it, or
  to copy it between the PC folder and the board. **⋯** has New folder,
  Copy files from this PC, **Back up** the whole drive to a zip, and (in the
  PC folder) Copy all these files to the board. **Drop files** from your file
  manager onto the list to copy them in.
- **Tabs**: each open file has one; the dot means unsaved changes (undoing
  back to the saved text clears it). Middle-click or ✕ closes one.
- **Editor** (CodeMirror 5, in `vendor/`): BASIC colours for `.BAS` files,
  completion as you type (keywords, and the names your program uses;
  Ctrl-Space asks), and under it the syntax of the statement or function the
  cursor is in, from the manual. Ctrl-F finds, Ctrl-/ comments lines out,
  Tab indents. **Save** (Ctrl-S) writes it back where it came from.
- **Run** (F5) saves the program if it has changed and runs it on the board
  (a file from the PC folder is copied to the board's FLASH: folder first).
  If it stops with an error ("Syntax error in 40", "... in line 12", or in an
  INCLUDEd file), the editor jumps to that line and marks it; errors in the
  console are links too.
- **Console**: a real terminal (xterm.js). Click it and type: keys go
  straight to the board, so `INPUT`, `INKEY$`, the arrow keys, the line
  editor's history, `COLOR` and `LOCATE` all work. **Stop** sends Ctrl-C.
- **Help** (F1 on a word, or the Help button): the manual's A-Z reference,
  searchable, with its examples coloured.

### Newer firmware

With the firmware changes in this round (`FILES "x", 1` and
`XSEND`/`XRECV "x", "Y"`), the list shows whole names and sizes in one
command, and files go by YMODEM, which sends the exact size (so a binary file
ending in ^Z bytes survives). The page finds out which firmware it is talking
to and uses the old commands with an older board.

If a program is running (an `AUTORUN.BAS`, say), the board isn't at `Ok`:
press **Stop**, then ⟳.

## Trying it without a board

Add `?mock` to the URL (`http://localhost:8000/?mock`): a pretend board in
`test/mock-serial.js` answers instead (`?mock=new` pretends to have the newer
firmware).

The PC folder needs `http://localhost` (not a file opened from the disk).

## Files

- `js/basic-data.js`: the keywords and reference entries, made from
  `docs/BASIC_REFERENCE.md` by `python3 ide/tools/make_data.py` (run it again
  when the manual changes).
- `js/basic-mode.js`: the BASIC colouring.
- `js/help.js`: the help panel. `js/hints.js`: completion and the syntax line.
- `js/tabs.js`, `js/terminal.js`, `js/sources.js` (the board's drives and the
  PC folder), `js/util.js` (menus, the zip writer).
- `js/xmodem.js`: XMODEM send and receive (tested against the firmware's own
  `src/xfer.c` built on the PC).
- `js/board.js`: the serial port, commands at the `Ok` prompt, FILES parsing,
  reading and writing files.
- `js/app.js`: the page.
