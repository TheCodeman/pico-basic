# Pico BASIC User's Manual

Pico BASIC is a BASIC interpreter for a bare RP2350 or RP2040 board, used from
a serial terminal. It keeps its programs and data files on a FAT disk in the
board's own flash, swaps files with the PC by XMODEM, plays tones on a
speaker pin, and drives the chip's GPIO, PWM, ADC, I2C, SPI and a second
UART. It needs no HDMI, no keyboard and no USB drive: just the board and a
USB cable to the PC, where it appears as a serial port.

It is the BASIC of the Pico 2 mini OS, built on its own for a serial
terminal.

## 1. Getting started

### Boards

| Board | Chip | Flash disk | Memory for BASIC |
|---|---|---|---|
| Raspberry Pi Pico 2 | RP2350A, GPIO 0-29 | about 3 MB | about 450 KB (SRAM) |
| Pimoroni Pico Plus 2 | RP2350B, GPIO 0-47 | about 15 MB | about 8.4 MB (8 MB PSRAM + SRAM) |
| Raspberry Pi Pico (the original) | RP2040, GPIO 0-29 | about 1 MB | about 190 KB (SRAM) |

Each board needs its own build: `build/pico_basic.uf2` for the Pico 2,
`build-plus2/pico_basic.uf2` for the Pico Plus 2 and
`build-pico/pico_basic.uf2` for the Pico (see the README). On the RP2040,
BASIC is the same language, but with less memory and no floating-point
hardware, so arithmetic-heavy programs run several times slower than on an
RP2350, and a 32 KB stack there has no hardware guard (BASIC still checks it
itself).

### Loading it onto the board

- **BOOTSEL:** hold the BOOTSEL button while plugging the board into USB,
  then copy the `.uf2` file to the drive that appears. The board restarts
  into BASIC, and the same USB port becomes its console.
- **picotool:** while BASIC runs, `picotool load -f build/pico_basic.uf2`
  (or `picotool reboot -f -u`, to get the BOOTSEL drive) reboots it over its
  USB console port: no button needed.
- **Debug Probe (SWD):** with Raspberry Pi's OpenOCD:

        OCD=$(ls -d ~/.pico-sdk/openocd/*/ | tail -1)
        "$OCD/openocd" -s "$OCD/scripts" -f interface/cmsis-dap.cfg -f target/rp2350.cfg \
            -c "adapter speed 5000" -c "program build-plus2/pico_basic.elf verify reset exit"

Loading a new BASIC keeps the files on the flash disk (it lives above the
program). Loading some other program that is bigger than 1 MB, or that uses
the flash for itself, overwrites it.

### Connecting

The console is the board's own **USB port**: connect it to the PC with a
data USB cable (a charge-only cable or a power supply won't do). It appears
as a USB serial port: `/dev/ttyACM0` (or the next free number, `ttyACM1`
when a Debug Probe is plugged in too) on Linux, a `COM` port on Windows,
`/dev/cu.usbmodem...` on a Mac. On Linux, `ls -l /dev/serial/by-id/` shows
it as `usb-Raspberry_Pi_Pico_...`.

The only other wiring is optional:

| Board pin | Connect to |
|---|---|
| GP18 (pin 24) | a piezo buzzer or small speaker (through about 100 Ω) to GND, for BEEP, SOUND and PLAY (the Maker Pi Pico's own buzzer) |
| GP10-15, 3V3, GND | a microSD card slot, as drive `SD:` (section 4; the Maker Pi Pico has one; other pins: CONFIG.TXT) |
| SWD (the debug pins) | a Raspberry Pi Debug Probe, for loading programs with OpenOCD (its UART isn't used) |

**3.3 V levels only** on every pin: never connect RS-232 or 5 V signals
straight to them.

### The terminal

Any terminal program: the speed setting doesn't matter on a USB serial
port (115200 8N1 is fine). Use ANSI or VT100, with the **UTF-8** character
set, and make the window at least **80 columns by 30 rows** (BASIC's screen;
LOCATE takes rows 1-30). For example:

    picocom /dev/ttyACM1
    minicom -D /dev/ttyACM1

On Windows, PuTTY or Tera Term on the board's COM port. gtkterm and screen
work too, but can't send files by XMODEM (section 5). On Linux your user
needs to be in the `dialout` group to open the port.

When the board starts it shows:

    Pico BASIC 0.1 for the RP2350 (pimoroni_pico_plus2_rp2350)
    PSRAM: 8 MB
    Flash disk: 15336 KB, 15332 KB free
    Pico BASIC. SYSTEM starts it afresh.
    8825424 Bytes free
    Ok

BASIC waits up to 3 seconds at start-up for a terminal; a terminal that
connects later (or reconnects) gets the first three lines again and an `Ok`.
The very first start-up formats the flash disk (a second or two).

### First program

Lines that start with a number are stored as the program; anything else
runs straight away:

    10 FOR I = 1 TO 3
    20   PRINT "HELLO"; I
    30 NEXT
    RUN
    SAVE "HELLO"

- Typing a line number alone deletes that line; retyping one replaces it.
- **Ctrl-C** stops a running program with `Break in <line>`; `CONT` carries
  on from there.
- `SYSTEM` (or `BYE`) clears everything and starts BASIC afresh, as after a
  reset (files on the disk stay).

### Programs without line numbers

Line numbers are optional. A program written in the full-screen editor
(`EDIT`, below), or in a text file on the PC, can leave them out and use
**labels** instead: a name and a colon starting a line. GOTO, GOSUB, ON ...
GOTO/GOSUB, IF ... THEN/ELSE, RESTORE, RESUME, ON ERROR GOTO, ON KEY(n) GOSUB
and RUN take a label wherever they take a line number:

    ' Count to 3, then stop
    I = 0
    again:
      I = I + 1
      PRINT "HELLO"; I
      IF I < 3 THEN again
    GOSUB done
    END

    done: PRINT "Done"
    RETURN

- A label is letters, digits, `.` and `_`, starting with a letter, up to 40
  characters; any case (BASIC shows it in capitals, as it does names). It can
  have statements after it on its line (`done: PRINT "Done"`). Two of the
  same name stop RUN with `Duplicate label`; a jump to one that isn't there
  is `Undefined label`.
- A statement keyword alone or with a number after it is a statement, not a
  label: `CLS:` and `SCREEN0:` are statements, `ENDGAME:` is a label.
- After THEN or ELSE a label jumps when nothing else follows it (`THEN
  again`); `THEN GOTO again` always works.
- A file's lines all have numbers or none do (LOAD says `Bad file mode`
  if it mixes them). Without numbers, each line's number is its line in the
  file, blank lines too: errors say `Syntax error in line 12` (Ctrl-G 12 in
  the editor goes there), and ERL and TRON give these numbers. LIST, SAVE and
  EDIT show the program without them.
- Labels work in numbered programs too.
- Typing a numbered line, AUTO, `EDIT n`, RENUM and MERGE need line numbers;
  with a program without them, use `EDIT`.
- `NUMBERS OFF` takes the numbers off a program: each line something jumps
  to gets a label `L` and its old number (`L100:`), and the jumps use it.
  `NUMBERS ON` numbers the lines 10, 20, 30... (labels stay). Numbers in
  `ERL` comparisons (`IF ERL = 100`) aren't changed.
- `examples/LABELS.BAS` is a menu program with labels.

### Library files: INCLUDE

Subroutines you use in many programs can live in a library file of their
own, a program without line numbers made of labelled subroutines, and each
program brings it in with `INCLUDE`:

    INCLUDE "OLEDLIB"
    GOSUB oledstart
    ROW = 0: COL = 0: TX$ = "HELLO": GOSUB oledtext

- `INCLUDE "file"` goes on a line of its own (a label may come before it).
  `.BAS` is added if the name has no extension; a folder or drive may be
  given (`INCLUDE "LIB/OLEDLIB"`, `INCLUDE "SD:MATHS"`).
- RUN reads the files in before the program starts and puts their lines
  after the program's own, so their labels work like the program's. Each
  file comes in once, however many INCLUDEs name it, and a library may
  INCLUDE others. Up to 16 files.
- LIST, SAVE and EDIT show only the program's own lines; the libraries' come
  in again at each RUN (so a changed library is picked up). Changing the
  program takes them out until the next RUN, and CONT can't carry on.
- An error in a library names its file and line: `Division by zero in line
  5 of OLEDLIB.BAS`. One that isn't there is `File not found` at the INCLUDE
  line; a library with line numbers is `Bad file mode`. A label in both the
  program and a library is `Duplicate label`.
- Running on past the program's last line ends it, as always: it doesn't
  carry on into a library. The variables are shared, so a library should
  give its own a prefix (OLEDLIB's start with `OL.`) and say which ones it
  takes, as OLEDLIB.BAS does at its top.

### Formatted listings

LIST, EDIT and SAVE show the program indented: the bodies of FOR ... NEXT
and WHILE ... WEND loops go in two spaces a level, and in a program without
line numbers so do the subroutines. Labels stay at the left; blank lines are
kept:

    ' Count to 3, then stop
    I = 0
    again:
    I = I + 1
    FOR J = 1 TO I
      PRINT J;
    NEXT
    IF I < 3 THEN again
    GOSUB done
    END

    done:
      PRINT "Done"
      RETURN

- A subroutine is a label that GOSUB, ON ... GOSUB or ON KEY(n) GOSUB calls,
  or ON ERROR GOTO goes to. Its lines are indented up to the next
  subroutine, or the next label after the RETURN or RESUME that ends it.
  Other labels (GOTO's, RESTORE's) don't change the indent.

- Only the look changes: loading a program (and leaving EDIT) takes the
  indents off again, so you can indent as you like or not at all.
- In a numbered program the indent goes after the number (`20   PRINT J;`).
- `FORMAT OFF` shows the lines as stored, `FORMAT ON` indents again, and
  `FORMAT n` sets the spaces a level (1-8).
- NEXT and WEND that close more blocks than are open just stop at the left.

### AUTORUN.BAS

At power-up (and after a reset, but not after `SYSTEM`) BASIC runs
`AUTORUN.BAS` if there is one: on the SD card first, so a card can override,
else on the flash disk. BASIC waits up to 3 seconds for a terminal to
connect first, so a terminal that is open sees everything the program
prints; a board on its own (on a USB charger, say) runs it after those 3
seconds. What it prints while no terminal is connected is kept (the last
8 KB, 4 KB on the RP2040) and shown when one connects, after the banner and
Bytes free, between "--- printed while no terminal was connected ---" and
"---", then "A program is running" (or Ok). When it ends, or stops with an
error or Ctrl-C, the `Ok` prompt follows as usual.

    NEW
    10 NEOPIXEL OPEN: NEOPIXEL BRIGHT 40
    20 NEOPIXEL FILL 0, 0, 255: NEOPIXEL SHOW
    SAVE "AUTORUN"

To start without it: hold **GP20** low at power-up (the Maker Pi Pico's
first button), or press **Esc** or **Ctrl-C** within a second if the
terminal is already open (the banner says so). `KILL "AUTORUN.BAS"` removes
it. `RUN "NEODEMO"` saved as AUTORUN (`LOAD "NEODEMO"` then
`SAVE "AUTORUN"`) makes the board start into the demo.

### Editing a line

At the `Ok` prompt and in `INPUT`:

| Key | Does |
|---|---|
| Left, Right, Home, End | move along the line |
| Backspace, Delete | delete before / at the cursor |
| Insert | switches between inserting and overwriting |
| Esc | clears the line |
| Up, Down | (at `Ok`) bring back the last 8 lines typed |
| F1-F10 | type their soft-key text (`KEY LIST` shows it: F1 `LIST `, F2 `RUN` and Enter, ...) |
| Ctrl-C | abandons the line |

`EDIT n` (or `EDIT .`, the last line typed or the one that failed) puts a
program line on the input line to change; Enter stores it. `EDIT` alone
opens the whole program in the full-screen editor (below). A long line runs
on over several rows. Pasting a whole listing into the terminal at the `Ok`
prompt enters it line by line.

### Full-screen editor

`EDIT` shows the program one line per row, to move around and change
freely; `EDIT "file"` edits any text file (on `FLASH:` or `SD:`) the same
way. It asks the terminal its size (80 x 24 if it doesn't say), and the
bottom line shows where you are: `Program *  Ln 3/12 Col 5  INS`.

| Key | Does |
|---|---|
| Arrows, Home, End, PgUp, PgDn | move |
| Ctrl-Home, Ctrl-End | the top, the bottom |
| Backspace, Delete | delete (at a line's ends they join lines) |
| Enter | split the line |
| Insert | insert or overwrite |
| Tab | spaces to the next column of 4 |
| Shift+Up, Shift+Down | mark lines (shown in reverse) |
| Ctrl-X | cut the marked lines, or the cursor's line (pressed again, the next line is added) |
| Ctrl-C | copy them (the cursor moves down, so pressing again adds the next line) |
| Ctrl-V | paste above the cursor's line |
| Ctrl-Z, Ctrl-R | undo, redo (typing or deleting in a row is one step; up to 100 steps) |
| Ctrl-F, F3 | find, find the next (any case) |
| Ctrl-G | go to a line (in a program: a BASIC line number) |
| Ctrl-S | save |
| Esc or Ctrl-Q | leave (asks whether to save the changes) |
| Ctrl-L | draw the screen again |
| F1 | the keys |

When a program is saved, every line that isn't blank must start with a
line number (0-65529, 255 characters at most); otherwise nothing is saved,
the cursor goes to the line, and the bottom line says why. The lines are
then taken as if typed: in any order, blank ones left out. Leaving without
saving keeps the program as it was. Ctrl-S needs the terminal's flow
control off (XON/XOFF would swallow it): gtkterm, minicom and picocom have
it off unless set. In the editor Ctrl-C copies; it stops programs only at
the Ok prompt and while they run. gtkterm keeps Ctrl-C and Ctrl-V for the
editor (its own copy and paste are Shift+Ctrl+C and Shift+Ctrl+V).

## 2. Program and direct commands

| Command | What it does |
|---|---|
| `RUN` | Runs the program from the start (clears variables first). |
| `RUN line` | Runs from that line (or label). |
| `RUN "file"` | Loads `file` and runs it. |
| `LIST [range]` | Lists the program, pausing every 28 lines (any key goes on, Q stops). A range is `n`, `n-`, `-n` or `n-m`. |
| `LLIST [range]` | Same as LIST (there's no printer). |
| `NEW` | Deletes the program and its variables. |
| `CLEAR` | Clears variables and closes files (any arguments are ignored). |
| `LOAD "file"[,R]` | Loads a program, and with `,R` runs it. `.BAS` is added if the name has no extension. Reads plain text and tokenised (binary) BASIC files from a PC. |
| `SAVE "file"[,A]` | Saves as plain text (`,A` is accepted; text is the only format). `.BAS` is added if the name has no extension. |
| `MERGE "file"` | Adds the lines of a plain-text program to the one in memory (lines with the same number are replaced). |
| `CHAIN [MERGE] "file"[, [line][, ALL][, DELETE range]]` | Runs another program from a program, keeping open files. Only the variables named in `COMMON` are passed on, or all of them with `ALL`. `CHAIN MERGE` adds the file's lines (after deleting `range`) and keeps all variables. |
| `INCLUDE "file"` | In a program, on a line of its own: brings in a library file's labelled subroutines when the program runs (see Library files). |
| `COMMON var[, var...]` | Names the variables CHAIN passes on (arrays as `A()`). |
| `FILES ["pattern"]` | Lists the files in the current folder (`FILES "*.BAS"`), then the disk's free space: `15331328 Bytes free`. |
| `KILL "file"` | Deletes a file. |
| `NAME "old" AS "new"` | Renames a file. |
| `DELETE range` | Deletes program lines. |
| `RENUM [new][,[old][,step]]` | Renumbers lines (default 10, first, 10), and the GOTO/GOSUB/THEN/ELSE/RESTORE/RESUME/RUN targets with them. |
| `FORMAT ON` / `FORMAT OFF` / `FORMAT n` | LIST, EDIT and SAVE indent loop bodies and the lines under labels (on at start-up; `n`: spaces a level, 1-8). |
| `NUMBERS OFF` / `NUMBERS ON` | Takes a program's line numbers off (the lines jumped to get labels such as `L100:`), or numbers it 10, 20, 30... (section 1). |
| `AUTO [start][,step]` | Numbers new lines for you (default 10,10). A `*` after the number means that line exists: Enter keeps it. An empty line where none exists, or Ctrl-C, stops. |
| `EDIT n` / `EDIT .` | Puts line `n` on the input line to edit. |
| `CONT` | Continues after Ctrl-C or STOP. |
| `TRON` / `TROFF` | Turns line-number tracing on/off: each line's number is printed in brackets as it runs. |
| `SYSTEM` / `BYE` | Starts BASIC afresh. |

Example:

    LOAD "PRIMES": LIST 10-50
    RENUM 100, , 10
    SAVE "PRIMES2"

## 3. Language

### Numbers, strings and variables

- **Single precision** (`!`, and names without a type) keep 7 digits: a value
  stored in one is rounded to single precision and prints
  with up to 7 digits.
- **Double precision** (`#`) keep about 15 digits (printed with up to 9);
  calculations are always done in double precision.
- **Integers** (`%`) hold whole numbers from -32768 to 32767: a value stored
  in one is rounded (2.5 becomes 3), and one out of range gives Overflow.
- **Constants:** `1.5`, `1E6`, `3#`, `5%`, hex `&HFF`, octal `&O17`.
- **Strings** (`$`) are up to 255 characters and may hold any byte,
  including CHR$(0).
- **Names** are up to 40 characters: letters, digits and `.`, starting with
  a letter, with an optional type character. `A$`, `A%`, `A!` and `A#` are
  four different variables, and `A` is `A!` unless `DEFINT`/`DEFSNG`/
  `DEFDBL`/`DEFSTR` say otherwise (after `DEFINT A-Z`, `A` is `A%`).
- An array and a plain variable may share a name: `A` and `A(3)` are
  separate.
- Keywords are recognised without spaces, so a variable
  name must not start with a keyword: `PRINTER` reads as `PRINT ER`.

### Arrays

- `DIM A(20), B$(5,5)`: indices start at 0; up to 8 dimensions.
- An array used without DIM gets 0-10 in each dimension.
- `OPTION BASE 1` (before any array is used) makes subscripts start at 1.
- `ERASE A, B$` deletes arrays, so they can be dimensioned again.

### Operators, highest priority first

| Operators | Meaning |
|---|---|
| `^` | power |
| `-` | negation |
| `*` `/` | multiply, divide |
| `\` | integer division |
| `MOD` | remainder |
| `+` `-` | add, subtract (`+` also joins strings) |
| `=` `<>` `<` `>` `<=` `>=` | comparisons, giving -1 for true and 0 for false (strings compare too) |
| `NOT` | |
| `AND` | |
| `OR` `XOR` | |
| `EQV` `IMP` | |

The logical operators work bit by bit on integers.

### Flow of control

| Statement | Notes |
|---|---|
| `GOTO line` | `GOTO 100`, or a label: `GOTO again` (section 1) |
| `GOSUB line` ... `RETURN` | RETURN ends the FOR and WHILE loops the subroutine started. |
| `ON n GOTO l1, l2, ...` / `ON n GOSUB ...` | Goes to the n-th line; past the end, carries on. |
| `IF cond THEN statements [ELSE statements]` | `THEN 100` and `ELSE 200` jump to lines; `IF X GOTO 100` works too. `IF A > 0 THEN PRINT "+" ELSE PRINT "-"` |
| `FOR v = a TO b [STEP s]` ... `NEXT [v[, v...]]` | `FOR I = 10 TO 1 STEP -1: PRINT I: NEXT` |
| `EXIT FOR` | Leaves the innermost FOR loop (from QuickBASIC). |
| `WHILE cond` ... `WEND` | `WHILE INKEY$ = "": WEND` waits for a key. |
| `END` | Ends the program (closes files). |
| `STOP` | Stops with `Break in <line>`; CONT goes on. |
| `PAUSE ms` | Waits (an extension); Ctrl-C still works. `PAUSE 500` |
| `REM text` or `' text` | A remark. |

Several statements can share a line, separated by `:`.

### Variables and data

| Statement | Notes |
|---|---|
| `LET var = expr` | `LET` is optional: `X = 5`. |
| `DIM`, `ERASE`, `OPTION BASE` | See Arrays. |
| `DEFINT`, `DEFSNG`, `DEFDBL`, `DEFSTR letters` | `DEFINT A-Z`, `DEFSTR S, T`. RUN, NEW and CLEAR reset them. |
| `DATA items`, `READ var[, var...]`, `RESTORE [line]` | `10 DATA 3, "RED", 2.5` / `20 READ N, C$, X` |
| `SWAP a, b` | Exchanges two variables of the same type. |
| `DEF FNname[(args)] = expr` | `DEF FNSQ(X) = X * X`, then `PRINT FNSQ(3)`. |
| `MID$(v$, start[, n]) = x$` | Replaces characters of `v$` in place (its length never changes). |
| `RANDOMIZE [seed]` | Without a seed it uses the clock. |
| `DATE$ = "mm-dd-yyyy"`, `TIME$ = "hh:mm:ss"` | Sets the clock (`-` or `/` and two-digit years are accepted). |
| `ENVIRON "NAME=value"` | Sets a string `ENVIRON$` returns (there's no DOS environment). |

The board has no battery-backed clock: at power-up the clock starts at
01-01-1970 00:00:00 and counts from there, until `DATE$` and `TIME$` set it.
Times are UTC. Files saved before it is set are dated 1 January 2026.

### Text output and input

| Statement | Notes |
|---|---|
| `PRINT items` or `? items` | `;` keeps the cursor on the line, `,` jumps to the next 14-column zone; `TAB(n)` and `SPC(n)`. `PRINT "X ="; X, "Y ="; Y` |
| `PRINT USING fmt$; values` | Formatted numbers and strings (below). `PRINT USING "##.##"; 3.14159` |
| `LPRINT`, `LPRINT USING` | Same as PRINT (there's no printer). |
| `WRITE items` | Items separated by commas, strings in quotes. |
| `INPUT ["prompt";] var[, var...]` | With `,` instead of `;` after the prompt, no `? ` is shown. Asks again if the input doesn't fit. |
| `LINE INPUT ["prompt";] var$` | Reads a whole line, commas included. |
| `CLS` | Clears the screen. |
| `LOCATE [row][, col][, cursor]` | Rows 1-30, columns 1-80; cursor 0 hides it, 1 shows it. `LOCATE 10, 30: PRINT "HERE"` |
| `COLOR [fg][, bg]` | PC colours 0-15 (fg 16-31 counts as 0-15), sent as ANSI colours; 7 (fg) and 0 (bg) give the terminal's own. `COLOR 14, 1` |
| `VIEW PRINT [top TO bottom]` | Text scrolls only within rows top-bottom; `VIEW PRINT` alone undoes it. |
| `KEY n, x$`, `KEY LIST`, `KEY ON` / `OFF` | Sets or lists the soft keys F1-F10. `KEY ON` shows their labels on row 25, as on a PC. `KEY 5, "FILES" + CHR$(13)` |
| `WIDTH n` | Accepted; the screen stays 80 columns. |

**Characters:** BASIC uses the IBM PC character set (code page 437), and
sends each character to the terminal as its Unicode equivalent in UTF-8:
CHR$(219) is a solid block, CHR$(176)-CHR$(178) are shades, CHR$(196) and
CHR$(179) are box lines, and control codes show their pictures (CHR$(1) is a
smiley, CHR$(3) a heart), except 7 (beep), 8, 9, 10, 12 (clear screen), 13
and 27. 11 moves the cursor home and 28-31 move it right, left, up and down.
Text you type is stored as the bytes the terminal sends (UTF-8 for accented
letters).

### PRINT USING

`PRINT USING fmt$; a, b, ...` prints the values in the fields of `fmt$`
(used again from the start if there are more values; other characters are
printed as they are):

| Field | Meaning |
|---|---|
| `#` | A digit position. `##.##` prints `3.5` as ` 3.50`. |
| `.` | The decimal point. |
| `,` before the point | Commas between thousands (it also counts as a digit). |
| `+` first or last | Always show the sign there. |
| `-` last | A minus after negative numbers. |
| `**` | Fill the space before the number with `*`. |
| `$$` | A `$` just before the number. `**$` does both. |
| `^^^^` (or `^^^^^`) | Exponent form: `##.##^^^^` prints ` 2.35E+02`. |
| `!` | The first character of a string. |
| `\  \` | The first n + 2 characters of a string (n spaces between). |
| `&` | The whole string. |
| `_` | Prints the next character as it is. |

A number too big for its field is printed in full after a `%`. Also
`PRINT #n, USING` and after other items: `PRINT "X ="; USING "##.#"; X`.

### Functions

| Kind | Functions |
|---|---|
| Maths | `ABS(x)` `INT(x)` (rounds down) `FIX(x)` (cuts off the fraction) `SGN(x)` `SQR(x)` `SIN(x)` `COS(x)` `TAN(x)` `ATN(x)` `EXP(x)` `LOG(x)` `CINT(x)` `CSNG(x)` `CDBL(x)` |
| Random | `RND` or `RND(1)`: the next number (0 to 1); `RND(0)` repeats the last one; `RND(-n)` reseeds. RUN always starts the same sequence: use `RANDOMIZE` for a different one. |
| Strings | `LEN(s$)` `LEFT$(s$,n)` `RIGHT$(s$,n)` `MID$(s$,start[,n])` `CHR$(n)` `ASC(s$)` `STR$(x)` `VAL(s$)` `INSTR([start,] s$, find$)` `STRING$(n, c or c$)` `SPACE$(n)` `HEX$(n)` `OCT$(n)` `UCASE$(s$)` `LCASE$(s$)` |
| Keyboard | `INKEY$`: the key waiting, or "". Cursor and function keys give CHR$(0) and the PC scan code: Up CHR$(0)+CHR$(72), Down 80, Left 75, Right 77, F1-F10 59-68, Home 71, End 79, PgUp 73, PgDn 81, Ins 82, Del 83. `INPUT$(n)`: reads n keys without showing them. |
| Screen | `POS(0)` (cursor column), `CSRLIN` (cursor row) |
| System | `TIMER` (seconds since start-up), `FRE(0)` (free memory), `DATE$`, `TIME$`, `ENVIRON$("name")` or `ENVIRON$(n)`, `PEEK(offset)` |
| Errors | `ERR`, `ERL` (see Error trapping), `ERDEV`, `ERDEV$`, `EXTERR(n)` (always 0 / "") |
| Sound | `PLAY(0)`: notes still waiting in the background queue |
| Files | `EOF(n)` `LOF(n)` `LOC(n)` `INPUT$(count, [#]n)` `FREEFILE` `DIR$(pattern$[, n])` `MKI$` `MKS$` `MKD$` `CVI` `CVS` `CVD` (section 4) |
| Hardware | `PIN(pin)`, `ADC(pin)`, `TEMP()`, `I2CREAD(...)`, `I2CREAD$(...)`, `SPI$(...)` (section 6) |

Examples:

    PRINT INT(-2.5), FIX(-2.5), CINT(2.5)        ' -3  -2  3
    PRINT INSTR("HELLO", "L"), MID$("HELLO", 2, 3) '  3  ELL
    PRINT HEX$(255); " "; STRING$(5, "*")        ' FF *****
    K$ = INKEY$: IF K$ = CHR$(0) + CHR$(72) THEN PRINT "UP"

### Error trapping

| Statement | Notes |
|---|---|
| `ON ERROR GOTO line` | Sends errors in a running program to that line; `ON ERROR GOTO 0` turns it off (inside the handler, it reports the error and stops). |
| `RESUME`, `RESUME NEXT`, `RESUME line` | Ends the handler: retries the statement that failed, goes on after it, or goes to the line. |
| `ERROR n` | Raises error `n` (1-255): a test, or a program's own error codes. |

    ON ERROR GOTO nofile
    OPEN "DATA.TXT" FOR INPUT AS #1
    opened: PRINT "opened": END
    nofile:
      IF ERR = 53 THEN PRINT "no file": RESUME opened
      ON ERROR GOTO 0

A program that reaches its end while handling an error stops with
`No RESUME`. Ctrl-C can't be trapped.

### Events

`ON KEY`, `ON TIMER`, `ON PLAY` and `ON COM` jump to a subroutine when
something happens; they are checked between statements. `ON`, `OFF` and
`STOP` switch an event: one that happens while it is STOPped (or while its
own subroutine runs) is remembered and handled once it is back ON. RETURN
from the subroutine continues the program where it was.

| Statement | When |
|---|---|
| `ON KEY(n) GOSUB line`, `KEY(n) ON` / `OFF` / `STOP` | Key `n` is pressed: 1-10 are F1-F10, 11 Up, 12 Left, 13 Right, 14 Down. |
| `ON TIMER(s) GOSUB line`, `TIMER ON` / `OFF` / `STOP` | Every `s` seconds (1-86400). |
| `ON PLAY(n) GOSUB line`, `PLAY ON` / `OFF` / `STOP` | The background music queue drops below `n` notes. |
| `ON COM(1) GOSUB line`, `COM(1) ON` / `OFF` / `STOP` | Something arrives on the open COM1. |

    ON TIMER(1) GOSUB clock: TIMER ON
    ON KEY(1) GOSUB f1: KEY(1) ON
    idle: GOTO idle
    clock: PRINT TIME$: RETURN
    f1: PRINT "F1 pressed": RETURN

### Memory: PEEK and POKE

There's no PC memory. `DEF SEG = segment` with `PEEK(offset)`,
`POKE offset, byte`, `BSAVE "file", offset, length` and
`BLOAD "file"[, offset]` reach two emulated places; anything else is an
Illegal function call:

- `DEF SEG = &HB800` (or `&HB000`): the text screen, two bytes a character.
  A terminal can't be read back, so PEEK gives spaces (32) and colour 7, and
  POKE does nothing.
- `DEF SEG = 0` (or `&H40`): BIOS bytes: video mode (&H449, 3), columns
  (&H44A, 80), rows - 1 (&H484, 29) and the timer tick count (&H46C-&H46E,
  18.2 a second, from midnight). The rest read 0, and POKEs there do nothing.

`SCREEN(row, col[, 1])` likewise gives 32 (and with 1, colour 7).

## 4. The flash disk and data files

### The disk

The flash disk is a FAT file system in the board's flash, after the
program: about 3 MB on a Pico 2 and 15 MB on a Pico Plus 2. It is formatted
(labelled PICOBASIC) the first time the board starts, and keeps its files
when the board is switched off or BASIC is loaded again.

- `FILES` lists it and ends with the free space; `FRE(0)` is memory, not
  disk.
- Names are FAT long names (case is kept, but matching ignores it). Folders
  work: `MKDIR "GAMES"`, `CHDIR "GAMES"`, `CHDIR ".."`, `RMDIR "GAMES"`, and
  paths such as `LOAD "GAMES/GUESS"`.
- **Writes are kept in a 4 KB cache** and go to the flash when a file is
  closed (or the cache moves on to another part of the disk). SAVE closes
  its file at once; a program's own files are closed by CLOSE, END, RUN,
  NEW, an error that stops it, and so on. **CLOSE files before switching
  off**, or the last few KB written can be lost.
- The flash wears out after about 100,000 writes to the same place: plenty
  for saving programs and logging now and then, but don't rewrite a file
  many times a second for hours.
- While the flash is being written (a few tens of milliseconds for each
  4 KB) the board does nothing else; background PLAY notes may stretch.

### The SD card (drive SD:)

A microSD card on SPI is a second drive, `SD:` (or `1:`); the flash disk is
`FLASH:` (or `0:`) and stays the current drive at start-up. Cards formatted
FAT16, FAT32 (SDHC, up to 32 GB) or exFAT (SDXC) work, as a PC formats them;
BASIC doesn't format cards.

Wiring, for the usual microSD breakout (one with its own 3.3 V regulator and
level shifter is fine too, powered from 3V3 or VSYS as it says):

| Card / module | Pico pin |
|---|---|
| CLK (SCK) | GP10 (pin 14) |
| SDI / DI (MOSI) | GP11 (pin 15) |
| SDO / DO (MISO) | GP12 (pin 16) |
| DAT1, DAT2 (if the slot has them) | GP13, GP14 (held up: unused in SPI mode) |
| CS (DAT3) | GP15 (pin 20) |
| 3V3 (VCC) | 3V3(OUT) (pin 36) |
| GND | GND (pin 18) |

These are SPI1's pins, and the Cytron Maker Pi Pico's microSD slot is wired
just so: it works as it is. Others can be chosen when BASIC is built (see the
README). The banner shows the card: `SD card (SD:): 7580 MB, 7571 MB free`,
or `SD card (SD:): none`.

- Use it with a drive in front of a name: `SAVE "SD:GAME"`,
  `LOAD "SD:/GAMES/GUESS"`, `OPEN "SD:LOG.TXT" FOR APPEND AS #1`,
  `FILES "SD:"`, `FILES "SD:/GAMES/*.BAS"`, `XRECV "SD:PIC.BMP"`.
- `CHDIR "SD:"` makes the card the current drive (then names without a
  drive are on the card), `CHDIR "SD:/GAMES"` its folder too, and
  `CHDIR "FLASH:"` goes back.
- Copying between drives: `LOAD "SD:GAME"` then `SAVE "FLASH:GAME"`; for
  any file, a program that reads one with GET and writes the other with PUT.
- A card can be put in or taken out while BASIC runs: the next use of `SD:`
  finds the new one. Close files on it first (CLOSE) and don't pull it out
  while it is being written.
- While a card is in, its pins (GP10-15) and SPI1 are BASIC's: `PIN`, `PWM` and
  `SPI OPEN` on SPI1 refuse them (`SPI OPEN` on SPI0, its default, is fine).
  Without a card they are free.

### Settings: CONFIG.TXT

`FLASH:/CONFIG.TXT`, if there is one, sets the board's pins at power-up
(and after a reset; `SYSTEM` doesn't read it again). Without it the build's
defaults hold: the SD card on, on the Maker Pi Pico's pins. Make or change
it with `EDIT "FLASH:CONFIG.TXT"`, then reset the board; `CONFIG` shows the
settings in force and any problems the file has (the banner shows those
too).

    # Pico BASIC settings (# ' or ; start a comment)
    SD = ON            # OFF: no SD card at all (and no start-up delay)
    SD_SCK = 10        # the card's pins: one SPI's SCK, MOSI and MISO,
    SD_MOSI = 11       #   CS any other pin
    SD_MISO = 12
    SD_CS = 15
    SD_MODE = 3        # SPI mode 0-3 (3 suits most slots; 0 is the usual default)
    SD_PULLUPS = 13, 14   # pins held up while the card is used (DAT1, DAT2); none
    SPEAKER = 18       # BEEP, SOUND and PLAY's pin; none
    NEOPIXEL = 28      # NEOPIXEL OPEN's default pin
    TZ = EST           # the time zone DATE$ and TIME$ show (UTC without it)
    CLOCK = 12         # TIME$ as 01:30:00 PM; 24 (the default): 13:30:00
    WATCHDOG = 5000    # arm the watchdog (ms) just before AUTORUN.BAS runs
    WIFI_SSID = HOME   # Wi-Fi builds: the network to join at power-up
    WIFI_PASSWORD = secret

- Keys and values are any case, with or without spaces; pins can be `18`
  or `GP18`. Lines it doesn't know are reported and left out.
- The SD card's SPI (SPI0 or SPI1) follows from its pins: SCK on GP2, 6, 10,
  14, 18, 22 (26...), MOSI and MISO on the same SPI's pins (MOSI 3, 7, 11,
  15..., MISO 0, 4, 8, 12...). Pins that don't fit turn the card off, with
  the reason in the banner.
- On a board without a card slot, `SD = OFF` saves the moment start-up
  spends looking for one (BASIC gives up after a tenth of a second when
  nothing answers at all).
- `TZ` is a name (UTC, EST, CST, MST, PST, AKST, HST, ARIZONA, UK, CET,
  EET, JST, AEST, or EASTERN, PACIFIC...: these follow daylight saving), hours
  from UTC (`TZ = -5`, `TZ = 5.5`: no daylight saving), or a POSIX TZ string
  (`TZ = EST5EDT,M3.2.0,M11.1.0`).
- `CLOCK = 12` (or `12H`) makes TIME$ 12-hour with AM or PM,
  `"01:30:00 PM"`: 11 characters instead of 8, so `LEFT$(TIME$, 5)` still
  gives `"01:30"` but a program that takes TIME$ apart may need changing.
  `CLOCK = 24`, the default, keeps the usual `"13:30:00"`. WIFI and NTP show
  the time the same way. `TIME$ = "1:30 PM"` and `TIME$ = "13:30"` both work
  either way.
- `examples/CONFIG.TXT` is a copy to start from.

### The watchdog

The chip's watchdog resets the board if a program stops checking in: a
hang, a loop that never ends, a device that never answers. With AUTORUN.BAS
a board on its own then starts again by itself.

    WATCHDOG 2000              ' arm it: reset unless fed within 2 seconds
    WHILE 1
      ... the work ...
      WATCHDOG FEED            ' "still alive"
    WEND

- `WATCHDOG ms` arms it (100 ms up to 8388 on the RP2040, 16777 on the
  RP2350), `WATCHDOG FEED` reloads it, `WATCHDOG OFF` disarms it, and
  `WATCHDOG` alone says how it is.
- A break (Ctrl-C, or STOP) turns it off, as someone is there. An error or
  END leaves it on, and BASIC says so under `Ok` (`Watchdog on: the board
  resets in 2000 ms`): a board on its own resets and runs AUTORUN.BAS
  again. Use `WATCHDOG OFF` before END to stop for good.
- Disk reads and writes, XMODEM, Wi-Fi waits and the full-screen editor feed
  it themselves. INPUT, PAUSE and the like don't: a program that waits
  longer than the time resets the board.
- After a reset by the watchdog the banner says `Reset by the watchdog`, and
  the function `WDRESET` is -1 (true): `IF WDRESET THEN ...`.
- `WATCHDOG = 5000` in CONFIG.TXT arms it just before AUTORUN.BAS runs (and
  not when there's none, so the prompt is safe).
- Files are written to the disk as they go, so a reset loses only what a
  program had in a file it hadn't closed (the last part of a file open for
  output); `CLOSE` before a risky part keeps it all.
- `examples/WATCHDOG.BAS` feeds it ten times, then lets it reset the board.

### MQTT

Wi-Fi builds talk MQTT (3.1.1) to a broker such as Mosquitto or Home
Assistant's: publishing readings, and getting messages from topics they
subscribe to.

    MQTT CONNECT "192.168.1.227"          ' (port 1883; or MQTT_BROKER in CONFIG.TXT)
    MQTT PUBLISH "home/pico/temp", STR$(TEMP())
    MQTT SUBSCRIBE "home/pico/led"
    ON MQTT GOSUB got: MQTT ON
    ...
    got: T$ = MQTT$(0): PRINT T$; " says "; MQTT$(1): RETURN

- `MQTT CONNECT [broker$[, port]]` connects (to CONFIG.TXT's `MQTT_BROKER`
  without one) and waits up to 10 seconds; `Device I/O error` (57) with the
  reason if it can't, and it goes on trying. `MQTT DISCONNECT` ends it.
- `MQTT PUBLISH topic$, payload$[, qos[, retain]]`: QoS 0 or 1 (2 is sent
  as 1); retain 1 has the broker keep it for later subscribers. Not
  connected: `Device I/O error`, so check `MQTT(0)` first in a program that
  runs on its own.
- `MQTT SUBSCRIBE topic$[, qos]` (the + and # wildcards work, up to 8
  subscriptions) and `MQTT UNSUBSCRIBE topic$`. Subscriptions are kept, and
  made again after a reconnect.
- Messages that come wait in a queue of 8 (payloads up to 255 characters, a
  string's limit; a full queue drops the oldest). `MQTT$(0)` takes the next
  one and gives its topic ("" if none); `MQTT$(1)` is then its payload.
  `MQTT(0)` is -1 while connected, `MQTT(1)` the messages waiting, `MQTT(2)`
  how many were dropped.
- `ON MQTT GOSUB line` with `MQTT ON` calls a subroutine when a message is
  waiting, between statements like ON KEY; `MQTT OFF` and `MQTT STOP` as
  for the other events.
- `MQTT` alone shows the broker, the connection, the subscriptions and the
  queue.
- In CONFIG.TXT: `MQTT_BROKER` (a name or an address), `MQTT_PORT` (1883),
  `MQTT_USER` and `MQTT_PASSWORD` if the broker wants them (kept as plain
  text, shown as `****`), `MQTT_CLIENT_ID` (pico-basic- and the end of the
  board's MAC to start with), and `MQTT = ON` to connect at power-up once
  Wi-Fi is up.
- A lost connection (Wi-Fi or the broker) is tried again every 5 seconds;
  a keep-alive every 60 seconds notices a broker that went away.
- `examples/MQTT.BAS` publishes the time and an ADC reading every 5
  seconds and sets the LED from a topic.

### Wi-Fi and the clock

The Wi-Fi builds (a Pico 2 W, a Pico W, or a Pico Plus 2 with Pimoroni's
RM2 breakout: see the README) join a network at power-up and set the clock
from the internet (NTP), so DATE$ and TIME$ are right without setting them.
CONFIG.TXT says which network:

    WIFI_SSID = HOME          # the network's name
    WIFI_PASSWORD = secret    # leave it out for an open network
    WIFI_COUNTRY = US         # two letters; XX (the default) works anywhere
    TZ = EASTERN              # local time for DATE$ and TIME$
    NTP_SERVER = pool.ntp.org # the default
    WIFI = ON                 # OFF: don't connect at power-up

- Connecting goes on in the background: the prompt doesn't wait. The banner
  says how far it got (`Wi-Fi: connecting to HOME`), and `WIFI` tells you
  later: the network, the signal, the IP address, and when NTP last set the
  clock.
- A network that goes away is tried again every 30 seconds; NTP asks again
  every hour. `NTP` asks now and prints the time; `WIFI OFF` and `WIFI ON`
  leave and join again; `WIFI SCAN` lists the networks around.
- The password is kept as plain text in CONFIG.TXT on the board's flash
  disk: anyone with the board (or `XSEND "FLASH:CONFIG.TXT"`) can read it.
  CONFIG shows it as `****`.
- On a Pico 2 W and a Pico W, GP23, GP24, GP25 and GP29 belong to the radio
  (BASIC refuses them), and the LED is on the radio chip: `PIN LED, 1`
  still lights it (PWM on it just turns it on or off). On the RM2 breakout
  the radio has GP32-35.
- A program can check: `WIFI(0)` is -1 (true) while connected, `WIFI(1)`
  the signal (dBm), `WIFI(2)` -1 once NTP has set the clock; `WIFI$(0)` is
  the IP address and `WIFI$(1)` the network. To wait up to 20 seconds:

      T = TIMER
      WHILE NOT WIFI(0) AND TIMER - T < 20: PAUSE 250: WEND
      IF WIFI(0) THEN PRINT "Online at "; WIFI$(0) ELSE PRINT "No Wi-Fi"

- Other builds say `Feature not available` to WIFI and NTP (WIFI() gives 0
  and WIFI$() ""); TZ works on every build (for DATE$ and TIME$ set by hand).

### Files from programs

Up to 4 files can be open at once, numbered #1 to #4. File names are
relative to the current folder; no extension is added. Each open file uses
about 0.8 KB of memory (plus its record buffer for random files).

| Statement / function | What it does |
|---|---|
| `OPEN "file" FOR INPUT \| OUTPUT \| APPEND \| RANDOM AS [#]n [LEN=size]` | Opens a file. OUTPUT creates or empties it, APPEND adds to the end, RANDOM (also the default when FOR is left out) reads and writes fixed-size records (LEN 1-1024, default 128). |
| `OPEN "I" \| "O" \| "A" \| "R", [#]n, "file"[, size]` | The older short form. |
| `CLOSE [[#]n[, [#]n...]]`, `RESET` | Close the given files, or all of them. |
| `PRINT #n, items`, `PRINT #n, USING fmt$; items` | Like PRINT, into the file. Lines end in CR LF, with no 80-column wrap. |
| `WRITE #n, items` | Items separated by commas, strings in quotes, so INPUT # reads them back exactly. |
| `INPUT #n, var[, var...]` | Reads items: a quoted string, or text up to a comma or the end of the line (numbers also stop at a space). |
| `LINE INPUT #n, var$` | Reads a whole line. |
| `INPUT$(count, [#]n)` | Reads count characters. |
| `EOF(n)` | -1 when there is nothing more to read (for a random file: the last GET was past the end). |
| `LOF(n)` | The file's length in bytes. |
| `LOC(n)` | For a random file, the last record read or written; otherwise the position in 128-byte blocks. |
| `FREEFILE` | The lowest free file number. |
| `FIELD #n, width AS var$[, width AS var$...]` | Divides a random file's record buffer into string fields (at most 16, and no more than the record length). |
| `LSET var$ = s$` / `RSET var$ = s$` | Puts a string into a field, padded with spaces, aligned left or right. Use these, not LET, for field variables. |
| `PUT [#]n[, record]` | Writes the record buffer as record `record` (from 1; default: the one after the last). |
| `GET [#]n[, record]` | Reads a record into the buffer and its FIELD variables. |
| `MKI$(n)` / `CVI(s$)` | A whole number -32768 to 32767 as a 2-byte string (low byte first), and back. |
| `MKS$(x)` / `CVS(s$)` | A number as a 4-byte single-precision string, and back. |
| `MKD$(x)` / `CVD(s$)` | A number as an 8-byte double-precision string, and back. |
| `DIR$(pattern$[, n])` | The n-th name (1 first) matching the pattern, e.g. `DIR$("*.BAS", 2)`; "" after the last. For going through a folder's files. |

Files stay open across lines and statements until CLOSE. RUN, NEW, CLEAR,
END, the program finishing, an error that stops the program, and SYSTEM
close every file. Ctrl-C does not, so CONT can carry on.

    OPEN "SCORES.TXT" FOR OUTPUT AS #1
    WRITE #1, "Ann", 120
    CLOSE #1
    OPEN "SCORES.TXT" FOR INPUT AS #1
    WHILE NOT EOF(1): INPUT #1, N$, S: PRINT N$, S: WEND
    CLOSE #1

Random access (a 22-byte record: a 20-character name and a 2-byte number):

    OPEN "PEOPLE.DAT" FOR RANDOM AS #1 LEN = 22
    FIELD #1, 20 AS NM$, 2 AS AGE$
    LSET NM$ = "Grace": LSET AGE$ = MKI$(85): PUT #1, 1
    GET #1, 1: PRINT NM$; CVI(AGE$)
    CLOSE #1

MKS$ and MKD$ use Microsoft Binary Format, so
random files written by BASICs on a PC read correctly here, and the other
way round. CVI, CVS and CVD need at least 2, 4 and 8 bytes (otherwise
Illegal function call).

Differences from BASICs on a PC:

- A file can be open for INPUT under several numbers, but not written while
  it is open elsewhere.
- Ctrl-Z (byte 26) ends a file for INPUT # and INPUT$, so
  binary files are best read with random access (GET).

## 5. Moving files to and from the PC

### Pasting and listing

The simplest way to bring a program over: type `NEW`, paste its listing
into the terminal at the `Ok` prompt (the numbered lines go into the
program), then `SAVE "NAME"`. To take one back: `LOAD` it, `LIST` it, and
copy the text from the terminal window. Many terminals can also log what is
shown to a file.

If pasted lines get lost, slow the terminal's paste down (a few
milliseconds per line): BASIC reads keys between lines, and the USB port
holds only a few hundred characters meanwhile.

### XMODEM: XRECV and XSEND

These copy any file, text or binary, between the flash disk and the PC over
the terminal line:

| Statement | What it does |
|---|---|
| `XRECV "file"` | Receives a file from the terminal's XMODEM send and saves it as `file`, replacing a file of that name. It asks for CRC-16, falling back to checksums for an old sender, and takes 128- or 1K-byte blocks. XMODEM pads the last block with ^Z (byte 26) characters; they are taken off the end. If the transfer fails, the partial file is deleted (a file that was already there is kept if nothing arrived). |
| `XSEND "file"` | Sends the file to the terminal's XMODEM receive, in 1K blocks with CRC-16, or 128-byte blocks with a checksum if the receiver asks for those. |

Type the command; BASIC says `Start the XMODEM send on the terminal now.`
(or receive), and you then have a minute to start the transfer on the PC.
XMODEM doesn't send the file's name, so give it on both ends. Pressing
Ctrl-X a few times in the terminal cancels a transfer; a failed one gives
`Device I/O error` after a line saying why. When it ends, BASIC prints e.g.
`Received 1 file, 329 bytes.`

    XRECV "GAME.BAS"
    (send GAME.BAS from the terminal)
    LOAD "GAME"
    RUN

Starting the PC's side:

- **minicom:** Ctrl-A S, choose `xmodem`, then the file, to send it to
  XRECV. Ctrl-A R, `xmodem` and a name receives from XSEND. (minicom uses
  lrzsz's `sx` and `rx`: `sudo apt install lrzsz`.)
- **picocom:** start it with
  `picocom --send-cmd "sx -vv" --receive-cmd "rx -vv" /dev/ttyACM1`,
  then Ctrl-A Ctrl-S sends a file and Ctrl-A Ctrl-R receives one (type the
  name to save it as).
- **Tera Term:** File > Transfer > XMODEM > Send... or Receive... (pick
  CRC or 1K).
- **From a shell**, with the terminal program closed (or disconnected) so
  it doesn't take the bytes:

        stty -F /dev/ttyACM1 raw -echo
        sx GAME.BAS < /dev/ttyACM1 > /dev/ttyACM1      # to XRECV
        rx GAME.BAS < /dev/ttyACM1 > /dev/ttyACM1      # from XSEND

  (with the board's port in place of `/dev/ttyACM1`).

The PC's receiver may keep XMODEM's ^Z padding (up to 1 KB of byte 26 at
the end). lrzsz's `rx` does, unless run as `rx -a` for a text file (which
also converts line ends). Because BASIC removes it, a binary file that
really ends in ^Z bytes loses them on the way in.

## 6. Sound

A square wave on **GP18** (another pin if BASIC was built with a different
`SOUND_PIN`), for a piezo buzzer or a small speaker through a resistor.

| Statement | What it does |
|---|---|
| `BEEP` | A short beep (800 Hz for a quarter of a second). |
| `SOUND freq, ticks` | A tone of freq Hz (37-32767; 32767 is silence) for `ticks` (18.2 make a second). After `PLAY "MB"` it goes to the background queue; ticks 0 stops the sound playing. `SOUND 440, 18` |
| `PLAY "string"` | Music in the music language (below). `PLAY "T160 O3 L8 CDEFGAB > C"` |
| `PLAY(0)` | Notes still waiting in the background queue. |

PLAY's commands:

| Command | Meaning |
|---|---|
| `A`-`G` [`#`, `+` or `-`] [length] [`.`] | A note (sharp or flat), optionally with its own length; each dot adds half again. |
| `N n` | Note number n (1-84; 0 is a rest). |
| `O n`, `>`, `<` | Octave 0-6 (default 4; octave 3 starts at middle C), up one, down one. |
| `L n` | Length for following notes: 1 whole, 4 quarter (default), up to 64. |
| `P n` | A pause, length as for L. |
| `T n` | Tempo, 32-255 quarter notes a minute (default 120). |
| `MN` `ML` `MS` | Normal (7/8 of each note sounds), legato (all), staccato (3/4). |
| `MF` `MB` | Foreground (PLAY waits until the music ends; the default) or background (up to 32 notes queue up and the program goes on). |
| `=variable;` | A number from a variable: `PLAY "O=OC;C"`. |

The settings last from one PLAY to the next; RUN resets them. `X`
(substrings through VARPTR$) isn't supported. The tones are plain square
waves of one voice, so a note waits for the one before it.

## 7. Hardware: GPIO, PWM, ADC, I2C, SPI, COM1

### Pins

Pins are given by their GP number; `LED` means the board's LED (GP25). The
Pico and Pico 2 have GP0-29 (GP23-25 and GP29 aren't on the header: power
control, VBUS sense, the LED and VSYS / 3); the Pico Plus 2 has GP0-47.

Pins BASIC itself uses are refused, e.g. `GP18 is used by the speaker
(BEEP, SOUND, PLAY)` followed by `Illegal function call`:

| Pins | Used by |
|---|---|
| GP18 | the speaker |
| GP10-15 | the SD card, while one is in |
| GP47 (Pico Plus 2) | the PSRAM |
| COM1's, I2C's and SPI's pins | while they are open |

| Statement / function | What it does |
|---|---|
| `PINMODE pin, INPUT \| OUTPUT \| PULLUP \| PULLDOWN` | Sets a pin up (or modes 0-3). `PINMODE 16, PULLUP` |
| `PIN pin, level` | Drives a pin 0 or 1, making it an output first if needed. `PIN LED, 1` |
| `PIN(pin)` | Reads a pin, making an unused pin an input. `IF PIN(16) = 0 THEN PRINT "PRESSED"` |
| `PWM pin, freq[, duty]` | PWM at 8 Hz-10 MHz with a duty of 0-100 % (default 50). `PWM pin, 0` stops it. `PWM 16, 1000, 25` |
| `SERVO pin, angle` | A hobby servo, 0-180 degrees (0.5-2.5 ms pulses at 50 Hz). `SERVO 16, 90` |
| `ADC(pin)` | An analog input, 0-4095 for 0-3.3 V: GP26-29 on the Pico and Pico 2, GP40-46 on the Pico Plus 2. `V = ADC(26) * 3.3 / 4095` |
| `TEMP()` | The chip's temperature in °C (uncalibrated: within a few degrees). |
| `PINS` | Lists every pin and what it is doing. |
| `NEOPIXEL ...` | WS2812B LEDs (below). |

- PWM pins come in pairs that share one frequency (GP0/1, GP2/3 and so on);
  changing one's frequency prints a warning naming its partner. The chip has
  only 8 slices for GP0-31, so GP16-31 use the same ones as GP0-15: GP2 and
  GP18 (and GP3 and GP19) give the very same signal, with no warning. The
  speaker's slice (GP18) is shared with GP2, GP3 and GP19: PWM there and
  sound interfere.
- `RUN`, `NEW` and `SYSTEM` set every pin BASIC used back to an input and
  stop PWM. `END`, errors and Ctrl-C leave pins as they are, so they can be
  checked in direct mode.
- `PIN`, `ADC` and `TEMP` followed by `(` are functions, so arrays can't use
  those names.

### I2C and SPI

The chip's own I2C and SPI controllers, for sensors, displays, memory chips
and the like. The pins choose the controller: I2C wants SDA on an even pin
and SCL on the next one (I2C0 or I2C1 by the pin numbers); SPI wants SCK,
MOSI and MISO from one group of four (MISO, CS, SCK, MOSI: GP8-11 is SPI1,
GP16-19 SPI0 ...). RUN, NEW and SYSTEM close them.

| Statement / function | What it does |
|---|---|
| `I2C OPEN [sda, scl[, khz]]` | Starts I2C, by default on **GP2 (SDA) and GP3 (SCL)**, I2C1, at 100 kHz (10-1000). Turns on the pins' weak pull-ups; devices should have their own. |
| `I2C WRITE addr, item[, item...][, NOSTOP]` | Sends bytes to the device at `addr` (0-127): numbers 0-255 and strings (their bytes). With `NOSTOP` the bus is kept for a read that follows. |
| `I2CREAD$(addr, n[, reg])` | Reads `n` bytes (1-255); with `reg`, first writes that register number (without a stop). |
| `I2CREAD(addr[, reg])` | Reads one byte. |
| `I2C SCAN` | Prints the addresses that answer (8-119). |
| `I2C CLOSE` | Stops it and frees the pins. |
| `SPI OPEN [sck, mosi, miso[, khz[, mode]]]` | Starts SPI, by default on **GP6 (SCK), GP7 (MOSI), GP4 (MISO)**, SPI0, at 1000 kHz (1-50000), mode 0 (0-3); 8-bit words, most significant bit first. Chip select is any pin you drive with `PIN` (**GP5** is free for it). (SPI1 is the SD card's.) |
| `SPI WRITE item[, item...]` | Sends bytes (numbers 0-255 and strings), ignoring what comes back. |
| `SPI$(out$)` | Sends `out$` and returns the bytes that came back meanwhile: `SPI$(STRING$(4, 0))` reads 4. |
| `SPI CLOSE` | Stops it and frees the pins. |

A device that doesn't answer gives `Device I/O error` (57); using I2C or SPI
before OPEN gives `Device unavailable` (68). Both can be caught with
ON ERROR.

    I2C OPEN: I2C SCAN
    T = I2CREAD(&H48, 0): PRINT "LM75 at &H48:"; T
    PIN 5, 1: SPI OPEN: PIN 5, 0: SPI WRITE &H9F
    ID$ = SPI$(STRING$(3, 0)): PIN 5, 1: PRINT LEN(ID$); "bytes"

### NeoPixels: WS2812B LEDs

Strips, rings and single WS2812B ("NeoPixel") LEDs, on any free pin, driven
by one of the chip's PIO state machines. Colours are kept in a buffer and
sent with `NEOPIXEL SHOW`.

| Statement | What it does |
|---|---|
| `NEOPIXEL OPEN [pin[, count[, order$]]]` | Sets up `count` LEDs (1-1024, default 1) on `pin` (default **GP28**, the Maker Pi Pico's RGB LED), all off. `order$` is `"GRB"` (WS2812B, the default) or `"RGB"` for strips that take red first. Opening again replaces the strip. |
| `NEOPIXEL n, r, g, b` | LED `n` (0 first) to red, green and blue 0-255, in the buffer. |
| `NEOPIXEL FILL r, g, b` | Every LED to one colour. |
| `NEOPIXEL SHOW` | Sends the buffer to the LEDs. |
| `NEOPIXEL BRIGHT level` | Brightness 0-255 (255 at first) for the next SHOWs: full white is bright and draws 60 mA a LED. |
| `NEOPIXEL CLEAR` | All off, shown at once. |
| `NEOPIXEL CLOSE` | Frees the pin and the state machine. |

- The pin is refused to `PIN`, `PWM` and the rest while the strip is open;
  `PINS` shows it. RUN, NEW and SYSTEM close it (the LEDs keep their last
  colours: `NEOPIXEL CLEAR` first to turn them off).
- Using it before `NEOPIXEL OPEN` gives `Device unavailable` (68).
- WS2812B LEDs want 5 V but take the Pico's 3.3 V data on a short wire.
  Power a strip of more than a few LEDs from its own 5 V supply, join its
  GND to the Pico's, and keep `BRIGHT` modest.

    NEOPIXEL OPEN 16, 8: NEOPIXEL BRIGHT 40
    FOR I = 0 TO 7: NEOPIXEL I, I * 32, 0, 255 - I * 32: NEXT
    NEOPIXEL SHOW

`examples/NEOPIXEL.BAS` shows red, green and blue, then a rainbow;
`examples/NEODEMO.BAS` has eight effects to step through.

### Serial port (COM1)

A serial port on the chip's UART1 (the console is on USB). It works
like a file: open it with a "COM1:" name, then use PRINT #, WRITE #, INPUT #,
LINE INPUT # and INPUT$ on it.

    OPEN "COM1:[speed][,parity][,data][,stop][,options]" AS #n

- **Settings:** speed 75-921600 baud; parity N, E or O; data bits 5-8; stop
  bits 1 or 2. Anything left out defaults to 300 baud, even
  parity, 7 data bits and 1 stop bit, so usually you give them all:
  `OPEN "COM1:9600,N,8,1" AS #1`.
- **Pins:** GP8 (TX) and GP9 (RX) unless `TXn` / `RXn` options choose
  others: TX on GP4, 8 or 20 and RX on GP5, 9 or 21 (`OPEN
  "COM1:9600,N,8,1,TX8,RX9" AS #1`). While it is open, those pins are
  refused to PIN and PWM.
- **Options:** `LF` sends an LF after each CR. `RS`, `CS`, `DS`, `CD`,
  `PE`, `BIN` and `ASC` are accepted and do nothing (there are no handshake
  lines).
- **Sending:** PRINT # ends lines with CR (CR LF with LF). It waits while
  the UART is busy.
- **Receiving:** bytes go into a 4 KB buffer in the background, so none are
  lost while the program is busy. `LOC(n)` is how many are waiting, `EOF(n)`
  is -1 when none are, and `LOF(n)` is the room left. INPUT #, LINE INPUT #
  and INPUT$ wait for data (Ctrl-C stops the wait). A line ends at CR (an LF
  after it is skipped). `ON COM(1) GOSUB` runs when bytes arrive.
- **Levels:** 3.3 V logic. Use a level shifter or an RS-232 adapter for
  RS-232 or 5 V devices, and connect GND too.
- Only one COM port can be open, and GET/PUT/FIELD don't apply to it.

`examples/SERIAL.BAS` is a small terminal: wire GP8 to GP9 and what you type
comes straight back.

### The Cytron Maker Pi Pico

BASIC's defaults fit this carrier board:

- **microSD slot:** drive `SD:` as it is (SPI1, GP10-15).
- **Buzzer:** GP18, for BEEP, SOUND and PLAY (its mute switch turns it
  off); the 3.5 mm audio jack is on GP18 (left) and GP19 (right).
- **Buttons:** GP20, GP21 and GP22, pressed = 0. Turn their pull-ups on
  first (`PINMODE 20, PULLUP` and so on), or a released button can read 0
  now and then; then `PIN(20)` is 0 while the button is pressed.
- **RGB LED:** a WS2812B on GP28: `NEOPIXEL OPEN` (with no arguments)
  then `NEOPIXEL 0, 255, 0, 0: NEOPIXEL SHOW` lights it red.
- **LEDs:** every GPIO has one, so they show what the pins are doing.
- **Free for your own use:** COM1 (GP8/GP9), `SPI OPEN` (GP4-7), I2C
  (GP2/GP3) and the Grove connectors on those pins.

## 8. Memory

- **Pico Plus 2:** the PSRAM is found at start-up (the banner shows it).
  Blocks of 512 bytes or more (arrays, mostly) go in the 8 MB PSRAM, and
  small ones (strings, program lines, variables) in the faster SRAM until it
  runs low, so programs get about 8.4 MB.
- **Pico 2:** everything is in SRAM, about 450 KB.
- **Pico (RP2040):** everything is in SRAM, about 190 KB: room for a long
  program, but `DIM A(20000)` (8 bytes an element) is about the most one
  array can have.
- `FRE(0)` (and the banner) give the memory free. A program line is refused
  if it would leave less than 16 KB for data, and any allocation if it would
  leave less than 4 KB.
- BASIC runs on a 32 KB stack: very deeply nested expressions give
  `Out of memory` rather than crashing.

## 9. Errors

Errors print as `<message> in <line>` (`in line <n>` for a program without
line numbers; just the message in direct mode), and `ERR` gives the code:

| Code | Message | Code | Message |
|---|---|---|---|
| 1 | NEXT without FOR | 26 | FOR without NEXT |
| 2 | Syntax error | 27 | Out of paper |
| 3 | RETURN without GOSUB | 29 | WHILE without WEND |
| 4 | Out of DATA | 30 | WEND without WHILE |
| 5 | Illegal function call | 50 | FIELD overflow |
| 6 | Overflow | 51 | Internal error |
| 7 | Out of memory | 52 | Bad file number |
| 8 | Undefined line number | 53 | File not found |
| 9 | Subscript out of range | 54 | Bad file mode |
| 10 | Duplicate Definition | 55 | File already open |
| 11 | Division by zero | 57 | Device I/O error |
| 12 | Illegal direct | 58 | File already exists |
| 13 | Type mismatch | 61 | Disk full |
| 14 | Out of string space | 62 | Input past end |
| 15 | String too long | 63 | Bad record number |
| 16 | String formula too complex | 64 | Bad file name |
| 17 | Can't continue | 66 | Direct statement in file |
| 18 | Undefined user function | 67 | Too many files |
| 19 | No RESUME | 68 | Device unavailable |
| 20 | RESUME without error | 69 | Communication buffer overflow |
| 22 | Missing operand | 70 | Permission denied |
| 23 | Line buffer overflow | 71 | Disk not ready |
| 24 | Device Timeout | 72 | Disk Media Error |
| 25 | Device Fault | 73 | Feature not available |
| | | 74 | Rename across disks |
| | | 75 | Path/File Access Error |
| | | 76 | Path not found |
| | | 80 | Undefined label |
| | | 81 | Duplicate label |

Codes BASIC itself never raises can still be raised with `ERROR n`; unknown
codes print `Unprintable error`. Ctrl-C prints `Break in <line>`.

## 10. Limits

- **Lines and strings:** program lines and strings are up to 255
  characters.
- **Nesting:** FOR, GOSUB and WHILE can each be nested up to 32 deep.
- **Files:** 4 open at once; random records up to 1024 bytes; 16 FIELD
  variables per file.
- **CHAIN:** up to 64 COMMON variables.
- **History:** the last 8 lines typed.
- **The screen:** 80 x 30 as far as BASIC knows (LOCATE, CSRLIN, the KEY
  line); a bigger terminal window is fine.

## 11. Example programs

The source's `examples` folder has these; send them over with `XRECV`
(section 5) or paste them in.

| Program | What it does |
|---|---|
| PRIMES.BAS | prints the primes below 1000 |
| MQTT.BAS | publishes the time and an ADC reading to an MQTT broker, and sets the LED from a topic (Wi-Fi builds) |
| WATCHDOG.BAS | arms the watchdog, feeds it ten times, then lets it reset the board |
| LABELS.BAS | a menu program without line numbers: GOSUB, ON GOSUB, RESTORE, ON ERROR GOTO and RESUME with labels |
| GUESS.BAS | a guess-the-number game |
| BOXES.BAS | coloured boxes drawn with the PC line-drawing characters |
| FILES.BAS | writes, appends and reads back a text file, and stores and reads records (MKI$/CVI) in a random-access file |
| BLINK.BAS | flashes the LED |
| FADE.BAS | fades the LED with PWM |
| ANALOG.BAS | shows the ADC inputs and the chip temperature (GP26-29; change `FIRST = 26: LAST = 29` to `FIRST = 40: LAST = 46` on a Pico Plus 2) |
| SERIAL.BAS | a small terminal on COM1 (GP8 TX, GP9 RX) |
| I2CSCAN.BAS | lists the I2C devices on GP2/GP3 and reads the first one's registers 0-7 |
| OLED.BAS | text, a checkerboard, a sine wave, contrast, invert and a live counter on a 128x64 SSD1309 or SSD1306 OLED (I2C on GP2/GP3, address &H3C), with OLEDLIB.BAS |
| OLEDLIB.BAS | a library (INCLUDE "OLEDLIB") for that OLED: start, clear, text in a 5x7 font, contrast, invert, off |
| NEODEMO.BAS | eight NeoPixel effects (rainbow, breathe, colour wipe, chase, candle, sparkle, police, heartbeat), changing every 8 seconds; Space/N, P, + and - (or the Maker Pi Pico's buttons GP20-22) to choose, Q quits; one line sets a strip's pin and length |
| NEOPIXEL.BAS | red, green, blue, then a rainbow on WS2812B LEDs (the Maker Pi Pico's RGB LED on GP28, or a strip) |
| SPIFLASH.BAS | reads an SPI flash chip's JEDEC ID and first 16 bytes (SPI0 on GP4-7) |

For example, from a shell (terminal closed) and then the terminal:

    XRECV "PRIMES.BAS"                         (in BASIC)
    sx examples/PRIMES.BAS < /dev/ttyACM1 > /dev/ttyACM1
    RUN "PRIMES"                               (in BASIC)
