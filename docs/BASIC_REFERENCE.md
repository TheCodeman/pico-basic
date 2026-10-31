# Pico BASIC Command Reference

## 13. Command reference

Every statement, command, function and operator, A to Z. Each entry gives its form (`[...]` is optional, `|` separates choices), what it does, its limits and errors, an example and what the terminal then shows. Numbers print with a space before them (or a `-`) and one after.

### Index by topic

**Programs and the disks:** [AUTO](#ref-auto), [BYE](#ref-bye), [CHAIN](#ref-chain), [CHDIR](#ref-chdir), [CLEAR](#ref-clear), [COMMON](#ref-common), [CONT](#ref-cont), [DELETE](#ref-delete), [EDIT](#ref-edit), [FILES](#ref-files), [KILL](#ref-kill), [LIST](#ref-list), [LLIST](#ref-llist), [LOAD](#ref-load), [MERGE](#ref-merge), [MKDIR](#ref-mkdir), [NAME](#ref-name), [NEW](#ref-new), [RENUM](#ref-renum), [RESET](#ref-reset), [RMDIR](#ref-rmdir), [RUN](#ref-run), [SAVE](#ref-save), [SYSTEM](#ref-system), [TROFF](#ref-troff), [TRON](#ref-tron), [XRECV](#ref-xrecv), [XSEND](#ref-xsend)

**Flow of control and errors:** [END](#ref-end), [ERL](#ref-erl), [ERR](#ref-err), [ERROR](#ref-error), [EXIT FOR](#ref-exit-for), [FOR](#ref-for), [GOSUB](#ref-gosub), [GOTO](#ref-goto), [IF](#ref-if), [NEXT](#ref-next), [ON ERROR GOTO](#ref-on-error-goto), [ON...GOSUB](#ref-ongosub), [ON...GOTO](#ref-ongoto), [PAUSE](#ref-pause), [REM](#ref-rem), [RESUME](#ref-resume), [RETURN](#ref-return), [STOP](#ref-stop), [WEND](#ref-wend), [WHILE](#ref-while)

**Variables and data:** [DATA](#ref-data), [DEF FN](#ref-def-fn), [DEF SEG](#ref-def-seg), [DEFDBL](#ref-defdbl), [DEFINT](#ref-defint), [DEFSNG](#ref-defsng), [DEFSTR](#ref-defstr), [DIM](#ref-dim), [ERASE](#ref-erase), [LET](#ref-let), [MID$ =](#ref-mid-1), [OPTION BASE](#ref-option-base), [RANDOMIZE](#ref-randomize), [READ](#ref-read), [RESTORE](#ref-restore), [SWAP](#ref-swap)

**The screen and keyboard:** [?](#ref-op-1), [CLS](#ref-cls), [COLOR](#ref-color), [CSRLIN](#ref-csrlin), [INKEY$](#ref-inkey), [INPUT](#ref-input), [INPUT$](#ref-input-2), [KEY](#ref-key), [LINE INPUT](#ref-line-input), [LOCATE](#ref-locate), [LPRINT](#ref-lprint), [POS](#ref-pos), [PRINT](#ref-print), [PRINT USING](#ref-print-using), [SPC](#ref-spc), [TAB](#ref-tab), [VIEW PRINT](#ref-view-print), [WIDTH](#ref-width), [WRITE](#ref-write)

**Numbers:** [ABS](#ref-abs), [ATN](#ref-atn), [CDBL](#ref-cdbl), [CINT](#ref-cint), [COS](#ref-cos), [CSNG](#ref-csng), [EXP](#ref-exp), [FIX](#ref-fix), [INT](#ref-int), [LOG](#ref-log), [RND](#ref-rnd), [SGN](#ref-sgn), [SIN](#ref-sin), [SQR](#ref-sqr), [TAN](#ref-tan)

**Strings:** [ASC](#ref-asc), [CHR$](#ref-chr), [HEX$](#ref-hex), [INSTR](#ref-instr), [LCASE$](#ref-lcase), [LEFT$](#ref-left), [LEN](#ref-len), [MID$](#ref-mid), [OCT$](#ref-oct), [RIGHT$](#ref-right), [SPACE$](#ref-space), [STR$](#ref-str), [STRING$](#ref-string), [UCASE$](#ref-ucase), [VAL](#ref-val)

**Data files:** [CLOSE](#ref-close), [CVD](#ref-cvd), [CVI](#ref-cvi), [CVS](#ref-cvs), [DIR$](#ref-dir), [EOF](#ref-eof), [FIELD](#ref-field), [FREEFILE](#ref-freefile), [GET](#ref-get), [INPUT #](#ref-input-1), [LINE INPUT #](#ref-line-input-1), [LOC](#ref-loc), [LOF](#ref-lof), [LSET](#ref-lset), [MKD$](#ref-mkd), [MKI$](#ref-mki), [MKS$](#ref-mks), [OPEN](#ref-open), [PRINT #](#ref-print-1), [PUT](#ref-put), [RSET](#ref-rset), [WRITE #](#ref-write-1)

**Events:** [COM(1)](#ref-com1), [KEY(n)](#ref-keyn), [ON COM](#ref-on-com), [ON KEY](#ref-on-key), [ON PLAY](#ref-on-play), [ON TIMER](#ref-on-timer), [PLAY ON](#ref-play-on), [TIMER ON](#ref-timer-on)

**Sound:** [BEEP](#ref-beep), [PLAY](#ref-play), [PLAY(n)](#ref-playn), [SOUND](#ref-sound)

**Pins and hardware:** [ADC](#ref-adc), [I2C](#ref-i2c), [I2CREAD](#ref-i2cread), [I2CREAD$](#ref-i2cread-1), [NEOPIXEL](#ref-neopixel), [PIN](#ref-pin), [PINMODE](#ref-pinmode), [PINS](#ref-pins), [PWM](#ref-pwm), [SERVO](#ref-servo), [SPI](#ref-spi), [SPI$](#ref-spi-1), [TEMP](#ref-temp)

**System:** [BLOAD](#ref-bload), [BSAVE](#ref-bsave), [DATE$](#ref-date), [ENVIRON](#ref-environ), [ENVIRON$](#ref-environ-1), [ERDEV](#ref-erdev), [EXTERR](#ref-exterr), [FRE](#ref-fre), [PEEK](#ref-peek), [POKE](#ref-poke), [SCREEN(r, c)](#ref-screenr-c), [TIME$](#ref-time), [TIMER](#ref-timer)

**Operators:** [+ - * / ^](#ref--), [= <> < > <= >=](#ref-op), [AND](#ref-and), [EQV](#ref-eqv), [IMP](#ref-imp), [NOT](#ref-not), [OR](#ref-or), [XOR](#ref-xor), [\ and MOD](#ref-and-mod)

**Not in this build:** [CALL](#ref-call), [CIRCLE](#ref-circle), [DRAW](#ref-draw), [FONT](#ref-font), [FONTW](#ref-fontw), [GPRINT](#ref-gprint), [IMAGE](#ref-image), [KEYDOWN](#ref-keydown), [LINE](#ref-line), [LOCK](#ref-lock), [LPOS](#ref-lpos), [MOUSE](#ref-mouse), [MOUSEX](#ref-mousex), [MUSIC](#ref-music), [MUSIC(n)](#ref-musicn), [PAINT](#ref-paint), [PALETTE](#ref-palette), [PCOPY](#ref-pcopy), [PMAP](#ref-pmap), [POINT](#ref-point), [PRESET](#ref-preset), [PSET](#ref-pset), [RGB](#ref-rgb), [SAVEIMAGE](#ref-saveimage), [SCREEN](#ref-screen), [SHELL](#ref-shell), [SPRITE](#ref-sprite), [SPRITEHIT](#ref-spritehit), [VIEW](#ref-view), [WINDOW](#ref-window)

### + - * / ^ {#ref--}

*Operator* · `a + b, a - b, a * b, a / b, a ^ b`

Add, subtract, multiply, divide, power. `+` also joins strings. `-x` negates. Calculations are in double precision.

- Errors: `Division by zero` (11), `Overflow` (6), `Type mismatch` (13) mixing strings and numbers.

Example:

```
PRINT 2 ^ 10; 7 / 2; "AB" + "CD"
```

Shows:

```
 1024  3.5 ABCD
```

See also: [\ and MOD](#ref-and-mod)

### = <> < > <= >= {#ref-op}

*Operator* · `a = b, a <> b, a < b, a > b, a <= b, a >= b`

Comparisons, of numbers or of strings (by character codes): -1 for true, 0 for false.

Example:

```
PRINT 3 > 2; "A" < "B"; 1 = 2
```

Shows:

```
-1 -1  0
```

See also: [IF](#ref-if), [AND](#ref-and)

### ? {#ref-op-1}

*Statement* · `? items`

Short for PRINT.

Example:

```
? 1 + 1
```

Shows:

```
 2
```

See also: [PRINT](#ref-print)

### ABS {#ref-abs}

*Function* · `ABS(x)`

The absolute value of `x` (`x` without its sign).

Example:

```
PRINT ABS(-3.5)
```

Shows:

```
 3.5
```

See also: [SGN](#ref-sgn)

### ADC {#ref-adc}

*Function* · `ADC(pin)`

Reads an analog input: 0-4095 for 0-3.3 V. GP26-29 on the Pico and Pico 2 (GP29 is VSYS / 3), GP40-46 on the Pico Plus 2.

- Errors: `Illegal function call` (5) for a pin without one.

Example:

```
PRINT ADC(26) * 3.3 / 4095
```

Shows:

```
 1.65040293
```

See also: [TEMP](#ref-temp), [PIN](#ref-pin)

### AND {#ref-and}

*Operator* · `a AND b`

Bitwise AND of two integers: true (-1) only if both are. Also masks bits: `X AND 15`.

Example:

```
PRINT 12 AND 10; (1 < 2) AND (2 < 3)
```

Shows:

```
 8 -1
```

See also: [OR](#ref-or), [NOT](#ref-not)

### ASC {#ref-asc}

*Function* · `ASC(s$)`

The code (0-255) of the first character of `s$`.

- Errors: `Illegal function call` (5) for "".

Example:

```
PRINT ASC("A")
```

Shows:

```
 65
```

See also: [CHR$](#ref-chr)

### ATN {#ref-atn}

*Function* · `ATN(x)`

The arctangent of `x`, in radians (-π/2 to π/2).

Example:

```
PRINT ATN(1) * 4
```

Shows:

```
 3.14159265
```

See also: [TAN](#ref-tan), [SIN](#ref-sin), [COS](#ref-cos)

### AUTO {#ref-auto}

*Command* · `AUTO [start][, step]`

Numbers the lines you type: each new line starts with the next number, so you just type the statement.

- `start` and `step` default to 10 and 10.
- A `*` after the number means that line already exists: Enter keeps it as it is.
- An empty line where no line exists, or Ctrl-C, stops it.

Example:

```
AUTO 100, 5
```

Shows:

```
100 
```

See also: [RENUM](#ref-renum), [DELETE](#ref-delete)

### BEEP {#ref-beep}

*Statement* · `BEEP`

A short beep: 800 Hz for a quarter of a second, on the speaker pin (GP18, the Maker Pi Pico's buzzer).

Example:

```
BEEP
```

See also: [SOUND](#ref-sound), [PLAY](#ref-play)

### BLOAD {#ref-bload}

*Statement* · `BLOAD file$[, offset]`

Loads a file saved by BSAVE (GW-BASIC's 7-byte header) into the emulated memory (DEF SEG).

See also: [BSAVE](#ref-bsave), [DEF SEG](#ref-def-seg)

### BSAVE {#ref-bsave}

*Statement* · `BSAVE file$, offset, length`

Saves `length` bytes of the emulated memory (DEF SEG) to a file.

See also: [BLOAD](#ref-bload)

### BYE {#ref-bye}

*Command* · `BYE`

The same as SYSTEM.

See also: [SYSTEM](#ref-system)

### CALL {#ref-call}

*Function / statement* · `CALL | DEF USR | USR | VARPTR | VARPTR$ | INP(port) | OUT port, b | WAIT port, m | IOCTL | IOCTL$`

Machine code and PC I/O ports: `Illegal function call` (5).

### CDBL {#ref-cdbl}

*Function* · `CDBL(x)`

`x` as a double-precision number.

Example:

```
PRINT CDBL(1) / 3
```

Shows:

```
 .333333333
```

See also: [CSNG](#ref-csng), [CINT](#ref-cint)

### CHAIN {#ref-chain}

*Statement* · `CHAIN [MERGE] file$[, [line][, ALL][, DELETE range]]`

Runs another program from a program, keeping the files that are open.

- Only the variables named in COMMON are passed on, or all of them with `ALL`.
- `line` is where the new program starts (default its first line).
- `CHAIN MERGE` adds the file's lines to the program (after deleting `range`) and keeps every variable.
- `.BAS` is added if the name has no extension.

Example:

```
10 COMMON SCORE
20 SCORE = 100: CHAIN "PART2"
RUN
```

See also: [COMMON](#ref-common), [MERGE](#ref-merge), [RUN](#ref-run)

### CHDIR {#ref-chdir}

*Statement* · `CHDIR path$`

Makes a folder the current one, or a drive the current drive.

- `CHDIR "GAMES"`, `CHDIR ".."`, `CHDIR "/"` move between folders.
- `CHDIR "SD:"` makes the SD card the current drive, `CHDIR "SD:/GAMES"` its folder too, `CHDIR "FLASH:"` goes back.
- Errors: `Path not found` (76).

Example:

```
MKDIR "GAMES": CHDIR "GAMES": FILES
```

Shows:

```
File not found
3080192 Bytes free
```

See also: [MKDIR](#ref-mkdir), [RMDIR](#ref-rmdir), [FILES](#ref-files)

### CHR$ {#ref-chr}

*Function* · `CHR$(n)`

The character with code `n` (0-255), in the IBM PC character set (see Characters).

- Errors: `Illegal function call` (5) out of range.

Example:

```
PRINT CHR$(65); CHR$(219); CHR$(1)
```

Shows:

```
A█☺
```

See also: [ASC](#ref-asc), [STRING$](#ref-string)

### CINT {#ref-cint}

*Function* · `CINT(x)`

`x` rounded to the nearest whole number (halves away from 0), -32768 to 32767.

- Errors: `Overflow` (6) out of range.

Example:

```
PRINT CINT(2.5); CINT(-2.5); CINT(2.4)
```

Shows:

```
 3 -3  2
```

See also: [INT](#ref-int), [FIX](#ref-fix)

### CIRCLE {#ref-circle}

*Not available* · `CIRCLE (x, y), r[, colour...]`

Draws a circle. Not available in this build: it gives **Feature not available** (error 73), as there is no screen of its own.

### CLEAR {#ref-clear}

*Statement* · `CLEAR [, n[, m]]`

Deletes every variable and array, closes every file, and forgets DATA, FOR, GOSUB and WHILE positions and event traps. The program stays.

- Any numbers after it (GW-BASIC's memory sizes) are accepted and ignored.

Example:

```
A = 5: CLEAR: PRINT A
```

Shows:

```
 0
```

See also: [NEW](#ref-new), [ERASE](#ref-erase)

### CLOSE {#ref-close}

*Statement* · `CLOSE [[#]n[, [#]n...]]`

Closes the files numbered, or all of them. Writes still in memory go to the disk.

Example:

```
CLOSE #1
```

See also: [OPEN](#ref-open), [RESET](#ref-reset)

### CLS {#ref-cls}

*Statement* · `CLS [n]`

Clears the terminal and puts the cursor at the top left.

- `n` (0-2) is accepted for old programs.

See also: [LOCATE](#ref-locate), [COLOR](#ref-color)

### COLOR {#ref-color}

*Statement* · `COLOR [fg][, [bg][, border]]`

Sets the text colours, as PC colours 0-15 sent as ANSI codes: 0 black, 1 blue, 2 green, 3 cyan, 4 red, 5 magenta, 6 brown, 7 white, 8-15 bright. 7 (fg) and 0 (bg) give the terminal's own colours.

- fg 16-31 counts as 0-15 (no blinking). `border` is ignored.
- Errors: `Illegal function call` (5) out of range.

Example:

```
COLOR 14, 1: PRINT "YELLOW ON BLUE": COLOR 7, 0
```

Shows:

```
YELLOW ON BLUE
```

See also: [CLS](#ref-cls)

### COM(1) {#ref-com1}

*Statement* · `COM(1) ON | OFF | STOP`

Turns the COM1 event (ON COM) on, off, or holds it (remembered until ON).

See also: [ON COM](#ref-on-com)

### COMMON {#ref-common}

*Statement* · `COMMON var[, var...]`

Names the variables CHAIN passes to the next program. Arrays are written with empty brackets: `COMMON A(), N$`.

- Up to 64 names.

Example:

```
COMMON SCORE, NAME$
```

See also: [CHAIN](#ref-chain)

### CONT {#ref-cont}

*Command* · `CONT`

Carries on with the program after Ctrl-C or STOP, from where it stopped.

- Errors: `Can't continue` (17) after the program has been changed or after an error.

See also: [STOP](#ref-stop), [END](#ref-end)

### COS {#ref-cos}

*Function* · `COS(x)`

The cosine of `x` radians.

Example:

```
PRINT COS(0)
```

Shows:

```
 1
```

See also: [SIN](#ref-sin), [TAN](#ref-tan)

### CSNG {#ref-csng}

*Function* · `CSNG(x)`

`x` as a single-precision number (7 digits).

Example:

```
PRINT CSNG(1#/3)
```

Shows:

```
 .3333333
```

See also: [CDBL](#ref-cdbl)

### CSRLIN {#ref-csrlin}

*Function* · `CSRLIN`

The row the cursor is on, 1-30.

Example:

```
LOCATE 10, 1: PRINT CSRLIN
```

Shows:

```
 10
```

See also: [POS](#ref-pos), [LOCATE](#ref-locate)

### CVD {#ref-cvd}

*Function* · `CVD(s$)`

The double-precision number in an 8-byte string made by MKD$ (GW-BASIC's format).

- Errors: `Illegal function call` (5) for a string shorter than 8.

See also: [MKD$](#ref-mkd)

### CVI {#ref-cvi}

*Function* · `CVI(s$)`

The integer in a 2-byte string made by MKI$ (low byte first).

Example:

```
PRINT CVI(MKI$(1234))
```

Shows:

```
 1234
```

See also: [MKI$](#ref-mki)

### CVS {#ref-cvs}

*Function* · `CVS(s$)`

The single-precision number in a 4-byte string made by MKS$.

Example:

```
PRINT CVS(MKS$(2.5))
```

Shows:

```
 2.5
```

See also: [MKS$](#ref-mks)

### DATA {#ref-data}

*Statement* · `DATA item[, item...]`

Holds numbers and strings for READ. Strings may be quoted (needed if they hold commas or start with spaces).

Example:

```
10 DATA 3, "RED", 2.5
```

See also: [READ](#ref-read), [RESTORE](#ref-restore)

### DATE$ {#ref-date}

*Statement / function* · `DATE$ = "mm-dd-yyyy" | DATE$`

Sets the date, or gives it as "mm-dd-yyyy". `-` or `/` and two-digit years are accepted. There is no battery clock: it starts at 01-01-1970 at power-up.

Example:

```
DATE$ = "10-31-2026": PRINT DATE$
```

Shows:

```
10-31-2026
```

See also: [TIME$](#ref-time)

### DEF FN {#ref-def-fn}

*Statement* · `DEF FNname[(arg[, arg...])] = expr`

Defines a one-line function, called as FNname(...). String functions end in $.

- Errors: `Undefined user function` (18) for one not defined.

Example:

```
DEF FNSQ(X) = X * X: PRINT FNSQ(7)
```

Shows:

```
 49
```

See also: [GOSUB](#ref-gosub)

### DEF SEG {#ref-def-seg}

*Statement* · `DEF SEG [= segment]`

Chooses the memory segment PEEK, POKE, BSAVE and BLOAD use: only `&HB800` (the text screen) and `0` (BIOS data) are emulated.

Example:

```
DEF SEG = 0: PRINT PEEK(&H44A)
```

Shows:

```
 80
```

See also: [PEEK](#ref-peek), [POKE](#ref-poke)

### DEFDBL {#ref-defdbl}

*Statement* · `DEFDBL letters`

Names starting with these letters (and no type character) are double precision: `DEFDBL A-C, X`.

- RUN, NEW and CLEAR reset it.

Example:

```
DEFDBL D: D = 1 / 3: PRINT D
```

Shows:

```
 .333333333
```

See also: [DEFINT](#ref-defint), [DEFSNG](#ref-defsng), [DEFSTR](#ref-defstr)

### DEFINT {#ref-defint}

*Statement* · `DEFINT letters`

Names starting with these letters are integers (-32768 to 32767): `DEFINT A-Z` makes all plain names integers.

Example:

```
DEFINT I: I = 2.7: PRINT I
```

Shows:

```
 3
```

See also: [DEFDBL](#ref-defdbl), [DEFSNG](#ref-defsng), [DEFSTR](#ref-defstr)

### DEFSNG {#ref-defsng}

*Statement* · `DEFSNG letters`

Names starting with these letters are single precision (the default).

See also: [DEFINT](#ref-defint), [DEFDBL](#ref-defdbl)

### DEFSTR {#ref-defstr}

*Statement* · `DEFSTR letters`

Names starting with these letters are strings, without a `$`.

Example:

```
DEFSTR S: S = "HI": PRINT S
```

Shows:

```
HI
```

See also: [DEFINT](#ref-defint)

### DELETE {#ref-delete}

*Command* · `DELETE range`

Deletes program lines. `range` is `n`, `n-`, `-n` or `n-m`.

- Errors: `Illegal function call` (5) if there is nothing in the range.

Example:

```
DELETE 100-190
```

See also: [NEW](#ref-new), [AUTO](#ref-auto)

### DIM {#ref-dim}

*Statement* · `DIM name(size[, size...])[, ...]`

Makes an array with subscripts 0 (or 1 after OPTION BASE 1) to `size`, up to 8 dimensions. An array used without DIM gets 0-10 in each dimension.

- Errors: `Duplicate Definition` (10) if it exists; `Subscript out of range` (9) for a subscript too big; `Out of memory` (7).

Example:

```
DIM A(5), B$(2, 3): A(5) = 7: PRINT A(5)
```

Shows:

```
 7
```

See also: [ERASE](#ref-erase), [OPTION BASE](#ref-option-base)

### DIR$ {#ref-dir}

*Function* · `DIR$(pattern$[, n])`

The `n`-th (default 1st) file name matching the pattern, e.g. `DIR$("*.BAS", 2)`; "" after the last. For going through a folder's files.

Example:

```
10 I = 1
20 F$ = DIR$("*.BAS", I): IF F$ = "" THEN END
30 PRINT F$: I = I + 1: GOTO 20
RUN
```

Shows:

```
PRIMES.BAS
NEODEMO.BAS
```

See also: [FILES](#ref-files)

### DRAW {#ref-draw}

*Not available* · `DRAW cmd$`

Turtle graphics. Not available in this build: it gives **Feature not available** (error 73), as there is no screen of its own.

### EDIT {#ref-edit}

*Command* · `EDIT n | EDIT .`

Puts program line `n` (or `.`, the last line typed or the one that failed) on the input line to change; Enter stores it.

- `EDIT` alone (the full-screen editor) is not available in this build: Feature not available.

Example:

```
EDIT 20
```

Shows:

```
20 PRINT "HELLO"
```

See also: [LIST](#ref-list), [AUTO](#ref-auto)

### END {#ref-end}

*Statement* · `END`

Ends the program and closes its files.

Example:

```
10 PRINT "DONE": END
```

Shows:

```
DONE
```

See also: [STOP](#ref-stop)

### ENVIRON {#ref-environ}

*Statement* · `ENVIRON "name=value"`

Sets a string ENVIRON$ gives back (there is no DOS environment).

Example:

```
ENVIRON "USER=KEN"
```

See also: [ENVIRON$](#ref-environ-1)

### ENVIRON$ {#ref-environ-1}

*Function* · `ENVIRON$(name$) | ENVIRON$(n)`

The value set for `name$` with ENVIRON, or the n-th "name=value" string; "" if none.

Example:

```
ENVIRON "USER=KEN": PRINT ENVIRON$("USER")
```

Shows:

```
KEN
```

See also: [ENVIRON](#ref-environ)

### EOF {#ref-eof}

*Function* · `EOF(n)`

-1 when file `n` has nothing more to read (for a random file: the last GET was past the end; for COM1: nothing waiting), else 0.

Example:

```
WHILE NOT EOF(1): LINE INPUT #1, L$: PRINT L$: WEND
```

See also: [LOF](#ref-lof), [LOC](#ref-loc)

### EQV {#ref-eqv}

*Operator* · `a EQV b`

Bitwise equivalence: a bit is 1 where both are the same.

Example:

```
PRINT -1 EQV 0
```

Shows:

```
 0
```

See also: [XOR](#ref-xor)

### ERASE {#ref-erase}

*Statement* · `ERASE array[, array...]`

Deletes arrays, freeing their memory, so they can be dimensioned again.

Example:

```
DIM A(100): ERASE A: DIM A(5)
```

See also: [DIM](#ref-dim)

### ERDEV {#ref-erdev}

*Function* · `ERDEV | ERDEV$`

GW-BASIC's device error details: always 0 and "" here.

See also: [ERR](#ref-err)

### ERL {#ref-erl}

*Function* · `ERL`

The line number where the last error happened (0 in direct mode).

Example:

```
100 PRINT "Error"; ERR; "in"; ERL: RESUME NEXT
```

See also: [ERR](#ref-err), [ON ERROR GOTO](#ref-on-error-goto)

### ERR {#ref-err}

*Function* · `ERR`

The code of the last error (see Error codes).

Example:

```
10 ON ERROR GOTO 100
20 X = 1 / 0
30 END
100 PRINT ERR: RESUME NEXT
RUN
```

Shows:

```
 11
```

See also: [ERL](#ref-erl), [ERROR](#ref-error)

### ERROR {#ref-error}

*Statement* · `ERROR n`

Raises error `n` (1-255), as if it had happened: a test, or a program's own error codes.

- Codes without a message print `Unprintable error`.

Example:

```
ERROR 53
```

Shows:

```
File not found
```

See also: [ON ERROR GOTO](#ref-on-error-goto), [ERR](#ref-err)

### EXIT FOR {#ref-exit-for}

*Statement* · `EXIT FOR`

Leaves the innermost FOR loop at once, going on after its NEXT (from QuickBASIC).

Example:

```
10 FOR I = 1 TO 10: IF I = 3 THEN EXIT FOR
20 NEXT: PRINT I
RUN
```

Shows:

```
 3
```

See also: [FOR](#ref-for)

### EXP {#ref-exp}

*Function* · `EXP(x)`

e (2.718...) to the power `x`.

- Errors: `Overflow` (6) if too big.

Example:

```
PRINT EXP(1)
```

Shows:

```
 2.71828183
```

See also: [LOG](#ref-log)

### EXTERR {#ref-exterr}

*Function* · `EXTERR(n)`

DOS extended error information: always 0 here.

See also: [ERR](#ref-err)

### FIELD {#ref-field}

*Statement* · `FIELD #n, width AS var$[, width AS var$...]`

Divides random file `n`'s record buffer into string fields (at most 16, no more than the record length). GET fills them; LSET/RSET set them for PUT.

- Errors: `FIELD overflow` (50).

Example:

```
FIELD #1, 20 AS NM$, 2 AS AGE$
```

See also: [GET](#ref-get), [PUT](#ref-put), [LSET](#ref-lset)

### FILES {#ref-files}

*Command* · `FILES [pattern$]`

Lists files, four to a line (folders marked `<DIR>`), then the free space on that drive.

- The pattern may have `*` and `?` and a drive and folder in front: `FILES "*.BAS"`, `FILES "SD:"`, `FILES "SD:/GAMES/*.BAS"`.
- Without one, all files in the current folder.
- `File not found` is printed (not an error) when nothing matches.

Example:

```
FILES "*.BAS"
```

Shows:

```
PRIMES.BAS          NEODEMO.BAS
3084288 Bytes free
```

See also: [DIR$](#ref-dir), [KILL](#ref-kill), [CHDIR](#ref-chdir)

### FIX {#ref-fix}

*Function* · `FIX(x)`

`x` with its fraction cut off (towards 0).

Example:

```
PRINT FIX(-2.7); FIX(2.7)
```

Shows:

```
-2  2
```

See also: [INT](#ref-int), [CINT](#ref-cint)

### FONT {#ref-font}

*Not available* · `FONT LOAD n, file$... | FONT n`

Font sheets for GPRINT. Not available in this build: it gives **Feature not available** (error 73), as there is no screen of its own.

### FONTW {#ref-fontw}

*Function / statement* · `FONTW(n) | FONTH(n) | GWIDTH(s$)`

Font cell sizes: font 0 is 8 x 16 (FONTW(0) 8, FONTH(0) 16, GWIDTH 8 a character); other fonts give 0.

### FOR {#ref-for}

*Statement* · `FOR v = a TO b [STEP s] ... NEXT [v]`

Repeats the lines up to NEXT with `v` = a, a+s, ... while it hasn't passed `b`. STEP defaults to 1; a negative STEP counts down.

- The loop body is skipped when `a` is already past `b`.
- Up to 32 loops nested.

Example:

```
FOR I = 10 TO 1 STEP -3: PRINT I;: NEXT
```

Shows:

```
 10  7  4  1
```

See also: [NEXT](#ref-next), [EXIT FOR](#ref-exit-for), [WHILE](#ref-while)

### FRE {#ref-fre}

*Function* · `FRE(x)`

The memory free for programs and data, in bytes (`x` is ignored).

Example:

```
PRINT FRE(0)
```

Shows:

```
 431456
```

### FREEFILE {#ref-freefile}

*Function* · `FREEFILE`

The lowest file number (1-4) not in use.

Example:

```
F = FREEFILE: OPEN "LOG.TXT" FOR APPEND AS #F
```

See also: [OPEN](#ref-open)

### GET {#ref-get}

*Statement* · `GET [#]n[, record]`

Reads record `record` (from 1; default the next) of random file `n` into its buffer and FIELD variables.

- `GET (x1,y1)-(x2,y2), a` (graphics) is not available: Feature not available.
- Errors: `Bad record number` (63).

Example:

```
GET #1, 3: PRINT NM$
```

See also: [PUT](#ref-put), [FIELD](#ref-field)

### GOSUB {#ref-gosub}

*Statement* · `GOSUB line`

Calls the subroutine at `line`; RETURN comes back to the statement after the GOSUB.

- Up to 32 deep.
- Errors: `Undefined line number` (8).

Example:

```
10 GOSUB 100: END
100 PRINT "SUB": RETURN
RUN
```

Shows:

```
SUB
```

See also: [RETURN](#ref-return), [ON...GOSUB](#ref-ongosub)

### GOTO {#ref-goto}

*Statement* · `GOTO line`

Goes on at `line`.

- Errors: `Undefined line number` (8).

Example:

```
10 PRINT "HI": GOTO 10
RUN
```

Shows:

```
HI
HI
...
```

See also: [GOSUB](#ref-gosub), [IF](#ref-if)

### GPRINT {#ref-gprint}

*Not available* · `GPRINT x, y, items`

Text drawn with a font sheet. Not available in this build: it gives **Feature not available** (error 73), as there is no screen of its own.

### HEX$ {#ref-hex}

*Function* · `HEX$(n)`

`n` (-32768 to 65535) in hexadecimal; negative numbers as their 16-bit two's complement.

- Errors: `Overflow` (6) out of range.

Example:

```
PRINT HEX$(255); " "; HEX$(-1)
```

Shows:

```
FF FFFF
```

See also: [OCT$](#ref-oct), [VAL](#ref-val)

### I2C {#ref-i2c}

*Statement* · `I2C OPEN [sda, scl[, khz]] | I2C WRITE addr, item[, item...][, NOSTOP] | I2C SCAN | I2C CLOSE`

The chip's I2C. OPEN starts it (default GP2 SDA, GP3 SCL, I2C1, 100 kHz; 10-1000), WRITE sends bytes (numbers 0-255 and strings) to device `addr` (0-127) (NOSTOP keeps the bus for a read), SCAN lists the devices that answer, CLOSE frees the pins.

- SDA on an even pin, SCL the next one.
- Errors: `Device I/O error` (57) if no device answers, `Device unavailable` (68) before OPEN.

Example:

```
I2C OPEN: I2C SCAN
```

See also: [I2CREAD](#ref-i2cread), [SPI](#ref-spi)

### I2CREAD {#ref-i2cread}

*Function* · `I2CREAD(addr[, reg])`

Reads one byte from I2C device `addr`; with `reg`, first writes the register number.

Example:

```
PRINT I2CREAD(&H68, &H75)
```

Shows:

```
 104
```

See also: [I2CREAD$](#ref-i2cread-1), [I2C](#ref-i2c)

### I2CREAD$ {#ref-i2cread-1}

*Function* · `I2CREAD$(addr, n[, reg])`

Reads `n` bytes (1-255) from I2C device `addr` as a string; with `reg`, from that register.

Example:

```
D$ = I2CREAD$(&H68, 6, &H3B)
```

See also: [I2CREAD](#ref-i2cread)

### IF {#ref-if}

*Statement* · `IF cond THEN statements [ELSE statements] | IF cond THEN line | IF cond GOTO line`

Runs the statements after THEN when `cond` is not 0 (true), else those after ELSE. A line number after THEN or ELSE jumps there.

- Comparisons give -1 (true) or 0 (false); any number not 0 counts as true.
- Everything to the end of the line belongs to THEN or ELSE.

Example:

```
A = 5: IF A > 3 THEN PRINT "BIG" ELSE PRINT "SMALL"
```

Shows:

```
BIG
```

See also: [WHILE](#ref-while), [ON...GOTO](#ref-ongoto)

### IMAGE {#ref-image}

*Not available* · `IMAGE file$[, x, y[, w, h]]`

Draws a picture. Not available in this build: it gives **Feature not available** (error 73), as there is no screen of its own.

### IMP {#ref-imp}

*Operator* · `a IMP b`

Bitwise implication: false only where `a` is true and `b` false.

Example:

```
PRINT 0 IMP 0
```

Shows:

```
-1
```

See also: [EQV](#ref-eqv)

### INKEY$ {#ref-inkey}

*Function* · `INKEY$`

The key waiting, or "" if none: it doesn't wait. Cursor and function keys give CHR$(0) and the PC scan code (see INKEY$ key codes).

Example:

```
K$ = INKEY$: IF K$ = CHR$(0) + CHR$(72) THEN PRINT "UP"
```

See also: [INPUT$](#ref-input-2), [ON KEY](#ref-on-key)

### INPUT {#ref-input}

*Statement* · `INPUT ["prompt"{; | ,}] var[, var...]`

Waits for a line typed at the terminal and puts its comma-separated items into the variables. A `;` after the prompt adds `? `, a `,` doesn't.

- If the items don't fit (too few, or text for a number) it says `?Redo from start` and asks again.
- Ctrl-C stops the program.

Example:

```
INPUT "Age"; A: PRINT A * 12; "months"
```

Shows:

```
Age? 3
 36 months
```

See also: [LINE INPUT](#ref-line-input), [INPUT #](#ref-input-1)

### INPUT # {#ref-input-1}

*Statement* · `INPUT #n, var[, var...]`

Reads items from file `n`: quoted strings, or text up to a comma or the end of the line (numbers stop at a space too).

- Errors: `Input past end` (62).

Example:

```
INPUT #1, N$, S
```

See also: [WRITE #](#ref-write-1), [LINE INPUT #](#ref-line-input-1)

### INPUT$ {#ref-input-2}

*Function* · `INPUT$(n[, [#]f])`

Reads `n` characters: from file `f`, or from the keyboard (waiting, without showing them).

Example:

```
K$ = INPUT$(1): PRINT "You pressed "; K$
```

Shows:

```
You pressed A
```

See also: [INKEY$](#ref-inkey), [INPUT](#ref-input)

### INSTR {#ref-instr}

*Function* · `INSTR([start,] s$, find$)`

Where `find$` first appears in `s$` (from position `start`, default 1); 0 if it doesn't.

Example:

```
PRINT INSTR("HELLO", "L"); INSTR(4, "HELLO", "L")
```

Shows:

```
 3  4
```

See also: [MID$](#ref-mid)

### INT {#ref-int}

*Function* · `INT(x)`

The largest whole number not more than `x` (rounds down).

Example:

```
PRINT INT(-2.5); INT(2.5)
```

Shows:

```
-3  2
```

See also: [FIX](#ref-fix), [CINT](#ref-cint)

### KEY {#ref-key}

*Statement* · `KEY n, text$ | KEY LIST | KEY ON | KEY OFF`

Sets what function key F1-F10 types (`CHR$(13)` in it is Enter), lists them, or shows their labels on row 25 (`KEY ON`) as on a PC.

- Defaults: F1 `LIST `, F2 `RUN`+Enter, F3 `LOAD"`, F4 `SAVE"`, F5 `CONT`+Enter, F7 `TRON`, F8 `TROFF`, F9 `KEY `, F10 `SCREEN 0,0,0`.
- `KEY(n) ON/OFF/STOP` is for key events: see ON KEY.

Example:

```
KEY 5, "FILES" + CHR$(13)
```

See also: [ON KEY](#ref-on-key)

### KEY(n) {#ref-keyn}

*Statement* · `KEY(n) ON | OFF | STOP`

Turns key `n`'s event (ON KEY) on, off, or holds it.

Example:

```
KEY(1) ON
```

See also: [ON KEY](#ref-on-key)

### KEYDOWN {#ref-keydown}

*Function / statement* · `KEYDOWN(n)`

Whether a key is held: a terminal sends key presses, not that, so it is always 0.

### KILL {#ref-kill}

*Statement* · `KILL file$`

Deletes a file. No extension is added.

- Errors: `File not found` (53); `File already open` (55) while it is open.

Example:

```
KILL "OLD.BAS"
```

See also: [NAME](#ref-name), [FILES](#ref-files)

### LCASE$ {#ref-lcase}

*Function* · `LCASE$(s$)`

`s$` in lower case.

Example:

```
PRINT LCASE$("Hello")
```

Shows:

```
hello
```

See also: [UCASE$](#ref-ucase)

### LEFT$ {#ref-left}

*Function* · `LEFT$(s$, n)`

The first `n` characters of `s$`.

Example:

```
PRINT LEFT$("HELLO", 2)
```

Shows:

```
HE
```

See also: [RIGHT$](#ref-right), [MID$](#ref-mid)

### LEN {#ref-len}

*Function* · `LEN(s$)`

The number of characters in `s$` (0-255).

Example:

```
PRINT LEN("HELLO")
```

Shows:

```
 5
```

### LET {#ref-let}

*Statement* · `[LET] var = expr`

Gives a variable a value. `LET` itself is optional.

- Errors: `Type mismatch` (13) for a string into a number or the other way; `Overflow` (6) out of an integer's range.

Example:

```
LET A = 5: B$ = "X"
```

See also: [SWAP](#ref-swap)

### LINE {#ref-line}

*Not available* · `LINE (x1, y1)-(x2, y2)[, colour[, B | BF]]`

Draws a line or box (LINE INPUT works). Not available in this build: it gives **Feature not available** (error 73), as there is no screen of its own.

### LINE INPUT {#ref-line-input}

*Statement* · `LINE INPUT ["prompt";] var$`

Reads a whole line typed at the terminal, commas and quotes included, into a string.

Example:

```
LINE INPUT "Name: "; N$
```

Shows:

```
Name: Ada Lovelace
```

See also: [INPUT](#ref-input)

### LINE INPUT # {#ref-line-input-1}

*Statement* · `LINE INPUT #n, var$`

Reads a whole line of file `n` into a string.

Example:

```
LINE INPUT #1, L$
```

See also: [INPUT #](#ref-input-1)

### LIST {#ref-list}

*Command* · `LIST [range]`

Lists the program, pausing after each 28 lines (any key goes on, Q stops). `range` is `n`, `n-`, `-n` or `n-m`.

Example:

```
LIST 10-30
```

Shows:

```
10 PRINT "HELLO"
20 GOTO 10
```

See also: [LLIST](#ref-llist), [EDIT](#ref-edit)

### LLIST {#ref-llist}

*Command* · `LLIST [range]`

The same as LIST: there is no printer.

See also: [LIST](#ref-list)

### LOAD {#ref-load}

*Command* · `LOAD file$[, R]`

Loads a program, replacing the one in memory; with `,R` runs it too.

- `.BAS` is added if the name has no extension.
- Plain text (CR LF or LF lines) and GW-BASIC tokenised files are read.
- Errors: `File not found` (53).

Example:

```
LOAD "PRIMES": RUN
```

Shows:

```
 168 primes below 1000
```

See also: [SAVE](#ref-save), [RUN](#ref-run), [MERGE](#ref-merge)

### LOC {#ref-loc}

*Function* · `LOC(n)`

For a random file: the last record read or written. For COM1: the bytes waiting. Otherwise the position in 128-byte blocks.

See also: [LOF](#ref-lof), [EOF](#ref-eof)

### LOCATE {#ref-locate}

*Statement* · `LOCATE [row][, [col][, cursor]]`

Moves the cursor to `row` (1-30) and `col` (1-80); `cursor` 0 hides it, 1 shows it.

- Errors: `Illegal function call` (5) off the screen.

Example:

```
LOCATE 12, 35: PRINT "MIDDLE"
```

See also: [CSRLIN](#ref-csrlin), [POS](#ref-pos)

### LOCK {#ref-lock}

*Function / statement* · `LOCK | UNLOCK | MOTOR`

Accepted for old programs; they do nothing.

### LOF {#ref-lof}

*Function* · `LOF(n)`

The length of file `n` in bytes (for COM1: the room left in its buffer).

Example:

```
OPEN "PRIMES.BAS" FOR INPUT AS #1: PRINT LOF(1): CLOSE
```

Shows:

```
 329
```

See also: [LOC](#ref-loc)

### LOG {#ref-log}

*Function* · `LOG(x)`

The natural logarithm of `x` (> 0). LOG(x) / LOG(10) is the base-10 log.

- Errors: `Illegal function call` (5) for 0 or less.

Example:

```
PRINT LOG(100) / LOG(10)
```

Shows:

```
 2
```

See also: [EXP](#ref-exp)

### LPOS {#ref-lpos}

*Function / statement* · `LPOS(n) | STICK(n) | STRIG(n) | PEN(n)`

Printer column, joystick, light pen: always 0. STRIG and PEN statements are accepted and do nothing.

### LPRINT {#ref-lprint}

*Statement* · `LPRINT items | LPRINT USING fmt$; items`

The same as PRINT: there is no printer.

See also: [PRINT](#ref-print)

### LSET {#ref-lset}

*Statement* · `LSET var$ = s$`

Puts `s$` in a FIELD variable, left-aligned and padded with spaces (cut if too long). Use it, not LET, for field variables.

Example:

```
LSET NM$ = "Grace"
```

See also: [RSET](#ref-rset), [FIELD](#ref-field)

### MERGE {#ref-merge}

*Command* · `MERGE file$`

Adds the lines of a plain-text program to the one in memory; lines with the same number are replaced.

Example:

```
MERGE "SUBS"
```

See also: [LOAD](#ref-load), [CHAIN](#ref-chain)

### MID$ {#ref-mid}

*Function* · `MID$(s$, start[, n])`

Up to `n` characters of `s$` from position `start` (all the rest without `n`).

- Errors: `Illegal function call` (5) for `start` < 1.

Example:

```
PRINT MID$("HELLO", 2, 3)
```

Shows:

```
ELL
```

See also: [LEFT$](#ref-left), [RIGHT$](#ref-right), [MID$ =](#ref-mid-1)

### MID$ = {#ref-mid-1}

*Statement* · `MID$(v$, start[, n]) = x$`

Replaces characters of `v$` in place, from `start`, with those of `x$` (at most `n`); `v$` keeps its length.

Example:

```
A$ = "HELLO": MID$(A$, 2, 3) = "ipp": PRINT A$
```

Shows:

```
HippO
```

See also: [MID$](#ref-mid)

### MKD$ {#ref-mkd}

*Function* · `MKD$(x)`

The number `x` as an 8-byte double-precision string, for random files.

See also: [CVD](#ref-cvd)

### MKDIR {#ref-mkdir}

*Statement* · `MKDIR path$`

Makes a folder.

- Errors: `Path/File Access Error` (75) if it exists.

Example:

```
MKDIR "SD:/GAMES"
```

See also: [CHDIR](#ref-chdir), [RMDIR](#ref-rmdir)

### MKI$ {#ref-mki}

*Function* · `MKI$(n)`

The integer `n` (-32768 to 32767) as a 2-byte string, low byte first.

- Errors: `Overflow` (6).

Example:

```
PRINT LEN(MKI$(5))
```

Shows:

```
 2
```

See also: [CVI](#ref-cvi)

### MKS$ {#ref-mks}

*Function* · `MKS$(x)`

The number `x` as a 4-byte single-precision string (GW-BASIC's format).

See also: [CVS](#ref-cvs)

### MOUSE {#ref-mouse}

*Not available* · `MOUSE ON | OFF`

The mouse pointer. Not available in this build: it gives **Feature not available** (error 73), as there is no screen of its own.

### MOUSEX {#ref-mousex}

*Function / statement* · `MOUSEX | MOUSEY | MOUSEB | MOUSEW`

The mouse: there is none, so they give 0.

### MUSIC {#ref-music}

*Not available* · `MUSIC file$ | MUSIC STOP...`

MOD and XM songs in the background. Not available in this build: it gives **Feature not available** (error 73), as there is no screen of its own.

### MUSIC(n) {#ref-musicn}

*Function / statement* · `MUSIC(n)`

The song's state: always 0 (MUSIC isn't available).

### NAME {#ref-name}

*Statement* · `NAME old$ AS new$`

Renames a file (or moves it to another folder on the same drive).

- Errors: `File not found` (53), `File already exists` (58).

Example:

```
NAME "A.BAS" AS "B.BAS"
```

See also: [KILL](#ref-kill)

### NEOPIXEL {#ref-neopixel}

*Statement* · `NEOPIXEL OPEN [pin[, count[, order$]]] | NEOPIXEL n, r, g, b | NEOPIXEL FILL r, g, b | NEOPIXEL SHOW | NEOPIXEL BRIGHT level | NEOPIXEL CLEAR | NEOPIXEL CLOSE`

WS2812B LEDs ("NeoPixels") on one pin, through a PIO state machine. OPEN sets up `count` (1-1024, default 1) LEDs on `pin` (default GP28, the Maker Pi Pico's RGB LED), GRB order or "RGB"; `n, r, g, b` and FILL set colours (0-255) in the buffer; SHOW sends it; BRIGHT (0-255) scales the next SHOWs; CLEAR turns them all off; CLOSE frees the pin.

- Errors: `Device unavailable` (68) before OPEN, `Illegal function call` (5) for a value out of range.
- RUN, NEW and SYSTEM close it.

Example:

```
NEOPIXEL OPEN: NEOPIXEL BRIGHT 40: NEOPIXEL 0, 255, 0, 0: NEOPIXEL SHOW
```

See also: [PIN](#ref-pin), [PWM](#ref-pwm)

### NEW {#ref-new}

*Command* · `NEW`

Deletes the program and its variables, closes files, and frees the pins BASIC used.

See also: [CLEAR](#ref-clear), [DELETE](#ref-delete)

### NEXT {#ref-next}

*Statement* · `NEXT [v[, v...]]`

Ends a FOR loop: adds the step and goes round again unless the end is passed. `NEXT J, I` ends two loops.

- Errors: `NEXT without FOR` (1).

Example:

```
FOR I = 1 TO 3: PRINT I;: NEXT I
```

Shows:

```
 1  2  3
```

See also: [FOR](#ref-for)

### NOT {#ref-not}

*Operator* · `NOT a`

Bitwise NOT: -1 becomes 0 and 0 becomes -1 (NOT x is -x - 1).

Example:

```
PRINT NOT 0; NOT 5
```

Shows:

```
-1 -6
```

See also: [AND](#ref-and)

### OCT$ {#ref-oct}

*Function* · `OCT$(n)`

`n` (-32768 to 65535) in octal.

Example:

```
PRINT OCT$(8)
```

Shows:

```
10
```

See also: [HEX$](#ref-hex)

### ON COM {#ref-on-com}

*Statement* · `ON COM(1) GOSUB line`

Calls the subroutine when bytes arrive on the open COM1 (with COM(1) ON).

See also: [COM(1)](#ref-com1), [OPEN](#ref-open)

### ON ERROR GOTO {#ref-on-error-goto}

*Statement* · `ON ERROR GOTO line | ON ERROR GOTO 0`

Sends errors in a running program to an error handler at `line`. `ON ERROR GOTO 0` turns that off; inside a handler it reports the error and stops.

- ERR and ERL say what and where; RESUME ends the handler.
- Ctrl-C can't be trapped.
- A program that ends inside a handler stops with `No RESUME` (19).

Example:

```
10 ON ERROR GOTO 100
20 OPEN "NONE" FOR INPUT AS #1
30 END
100 PRINT "no file": RESUME 30
RUN
```

Shows:

```
no file
```

See also: [RESUME](#ref-resume), [ERR](#ref-err), [ERROR](#ref-error)

### ON KEY {#ref-on-key}

*Statement* · `ON KEY(n) GOSUB line`

Calls the subroutine when key `n` is pressed (with KEY(n) ON): 1-10 are F1-F10, 11 Up, 12 Left, 13 Right, 14 Down.

- `KEY 15`-`KEY 20` user keys are accepted but never trigger.

Example:

```
10 ON KEY(1) GOSUB 100: KEY(1) ON
20 GOTO 20
100 PRINT "F1": RETURN
RUN
```

Shows:

```
F1
```

See also: [KEY(n)](#ref-keyn), [INKEY$](#ref-inkey)

### ON PLAY {#ref-on-play}

*Statement* · `ON PLAY(n) GOSUB line`

Calls the subroutine when PLAY's background queue drops below `n` notes (with PLAY ON): for music that never stops.

See also: [PLAY](#ref-play), [PLAY ON](#ref-play-on)

### ON TIMER {#ref-on-timer}

*Statement* · `ON TIMER(s) GOSUB line`

Calls the subroutine every `s` seconds (1-86400), with TIMER ON.

Example:

```
10 ON TIMER(1) GOSUB 100: TIMER ON
20 GOTO 20
100 PRINT TIME$: RETURN
RUN
```

Shows:

```
12:00:01
12:00:02
```

See also: [TIMER ON](#ref-timer-on)

### ON...GOSUB {#ref-ongosub}

*Statement* · `ON n GOSUB line1[, line2...]`

Calls the n-th subroutine in the list. If `n` is 0 or past the list, it goes on with the next statement.

Example:

```
10 ON 2 GOSUB 100, 200: END
100 PRINT "A": RETURN
200 PRINT "B": RETURN
RUN
```

Shows:

```
B
```

See also: [ON...GOTO](#ref-ongoto), [GOSUB](#ref-gosub)

### ON...GOTO {#ref-ongoto}

*Statement* · `ON n GOTO line1[, line2...]`

Goes to the n-th line in the list; with `n` 0 or past the list, goes on.

Example:

```
ON 1 GOTO 100, 200
```

See also: [ON...GOSUB](#ref-ongosub)

### OPEN {#ref-open}

*Statement* · `OPEN file$ FOR INPUT | OUTPUT | APPEND | RANDOM AS [#]n [LEN = size] | OPEN mode$, [#]n, file$[, size] | OPEN "COM1:..." AS [#]n`

Opens a file as number `n` (1-4). OUTPUT creates or empties it, APPEND adds to its end, INPUT reads it, RANDOM reads and writes fixed-size records (LEN 1-1024, default 128; also the default without FOR).

- The short form takes "I", "O", "A" or "R".
- A "COM1:" name opens the serial port: see the manual's COM1 section.
- Errors: `File not found` (53), `File already open` (55), `Bad file number` (52).

Example:

```
OPEN "SCORES.TXT" FOR OUTPUT AS #1: PRINT #1, "Ann", 120: CLOSE #1
```

See also: [CLOSE](#ref-close), [PRINT #](#ref-print-1), [INPUT #](#ref-input-1)

### OPTION BASE {#ref-option-base}

*Statement* · `OPTION BASE 0 | 1`

The lowest array subscript, before any array is used.

Example:

```
OPTION BASE 1: DIM A(3)
```

See also: [DIM](#ref-dim)

### OR {#ref-or}

*Operator* · `a OR b`

Bitwise OR: true if either is.

Example:

```
PRINT 12 OR 3
```

Shows:

```
 15
```

See also: [AND](#ref-and), [XOR](#ref-xor)

### PAINT {#ref-paint}

*Not available* · `PAINT (x, y)[, colour[, border]]`

Flood fill. Not available in this build: it gives **Feature not available** (error 73), as there is no screen of its own.

### PALETTE {#ref-palette}

*Not available* · `PALETTE [attr, colour]`

Changes colours. Not available in this build: it gives **Feature not available** (error 73), as there is no screen of its own.

### PAUSE {#ref-pause}

*Statement* · `PAUSE ms`

Waits `ms` milliseconds (an extension); Ctrl-C still stops it.

Example:

```
PAUSE 500
```

See also: [TIMER](#ref-timer)

### PCOPY {#ref-pcopy}

*Not available* · `PCOPY from, to`

Copies screen pages. Not available in this build: it gives **Feature not available** (error 73), as there is no screen of its own.

### PEEK {#ref-peek}

*Function* · `PEEK(offset)`

A byte of the emulated memory chosen by DEF SEG: the text screen (&HB800: reads spaces and colour 7, as a terminal can't be read) or BIOS data (DEF SEG = 0: &H449 mode 3, &H44A columns 80, &H484 rows-1 29, &H46C-&H46E timer ticks).

- Errors: `Illegal function call` (5) anywhere else.

Example:

```
DEF SEG = 0: PRINT PEEK(&H44A)
```

Shows:

```
 80
```

See also: [POKE](#ref-poke), [DEF SEG](#ref-def-seg)

### PIN {#ref-pin}

*Statement / function* · `PIN pin, level | PIN(pin)`

`PIN pin, level` drives a pin high (1) or low (0), making it an output first. `PIN(pin)` reads it (0 or 1), making an unused pin an input. `LED` stands for the board's LED (GP25).

- Pins BASIC uses (the speaker, SD card, PSRAM, open I2C/SPI/COM1/NeoPixel) are refused: `GPn is used by ...` then `Illegal function call`.
- On the Maker Pi Pico the buttons GP20-22 read 0 when pressed (turn their pull-ups on with PINMODE).

Example:

```
PIN LED, 1: PRINT PIN(20)
```

Shows:

```
 1
```

See also: [PINMODE](#ref-pinmode), [PINS](#ref-pins), [PWM](#ref-pwm)

### PINMODE {#ref-pinmode}

*Statement* · `PINMODE pin, INPUT | OUTPUT | PULLUP | PULLDOWN`

Sets a pin up as an input, an output, or an input with its pull-up or pull-down (or modes 0-3).

- On the RP2350 a pull-down may not pull a pin low (its erratum E9); pull-ups are fine.

Example:

```
PINMODE 20, PULLUP: PRINT PIN(20)
```

Shows:

```
 1
```

See also: [PIN](#ref-pin)

### PINS {#ref-pins}

*Command* · `PINS`

Lists every pin and what it is doing (free, in, out, PWM, ADC, the speaker, SD card, I2C, SPI, COM1, NeoPixel...).

Example:

```
PINS
```

Shows:

```
GP0  free           GP8  free  ...
```

See also: [PIN](#ref-pin)

### PLAY {#ref-play}

*Statement* · `PLAY music$`

Plays notes in GW-BASIC's music language (see the manual's Sound section: A-G, O, L, T, P, MF/MB...).

- After `MB` the notes play in the background (up to 32 queued) while the program goes on.
- `X` substrings aren't supported; `=variable;` is.

Example:

```
PLAY "T160 O3 L8 CDEFGAB > C"
```

See also: [SOUND](#ref-sound), [PLAY(n)](#ref-playn), [ON PLAY](#ref-on-play)

### PLAY ON {#ref-play-on}

*Statement* · `PLAY ON | OFF | STOP`

Turns the PLAY event (ON PLAY) on, off, or holds it.

See also: [ON PLAY](#ref-on-play)

### PLAY(n) {#ref-playn}

*Function* · `PLAY(n)`

The number of notes still waiting in the background queue (`n` is ignored).

Example:

```
PLAY "MB CDE": PRINT PLAY(0)
```

Shows:

```
 3
```

See also: [PLAY](#ref-play), [ON PLAY](#ref-on-play)

### PMAP {#ref-pmap}

*Not available* · `PMAP(v, n)`

Converts WINDOW coordinates. Not available in this build: it gives **Feature not available** (error 73), as there is no screen of its own.

### POINT {#ref-point}

*Not available* · `POINT(x, y) | POINT(n)`

A pixel's colour, or the graphics cursor. Not available in this build: it gives **Feature not available** (error 73), as there is no screen of its own.

### POKE {#ref-poke}

*Statement* · `POKE offset, byte`

Writes a byte to the emulated memory (DEF SEG). The text screen and BIOS bytes accept it and change nothing here.

See also: [PEEK](#ref-peek)

### POS {#ref-pos}

*Function* · `POS(n)`

The column the cursor is in, 1-80 (`n` is ignored).

Example:

```
PRINT "ABC";: PRINT POS(0)
```

Shows:

```
ABC 4
```

See also: [CSRLIN](#ref-csrlin)

### PRESET {#ref-preset}

*Not available* · `PRESET (x, y)[, colour]`

Plots a point in the background colour. Not available in this build: it gives **Feature not available** (error 73), as there is no screen of its own.

### PRINT {#ref-print}

*Statement* · `PRINT [items]`

Prints numbers and strings. `;` between items prints them together, `,` jumps to the next 14-column zone; a `;` or `,` at the end keeps the cursor on the line.

- Numbers print with a space before them (or a `-`) and one after.
- `TAB(n)` and `SPC(n)` position the text.
- Lines wrap at 80 columns.

Example:

```
PRINT "A"; 1, "B"; 2
```

Shows:

```
A 1           B 2
```

See also: [PRINT USING](#ref-print-using), [WRITE](#ref-write), [TAB](#ref-tab)

### PRINT # {#ref-print-1}

*Statement* · `PRINT #n, [USING fmt$;] items`

Like PRINT, into file `n`. Lines end in CR LF (CR alone on COM1 unless LF was asked for), and there is no 80-column wrap.

Example:

```
PRINT #1, "TOTAL"; 42
```

See also: [WRITE #](#ref-write-1), [PRINT](#ref-print)

### PRINT USING {#ref-print-using}

*Statement* · `PRINT USING fmt$; items[;]`

Prints numbers and strings in the fields of `fmt$` (see PRINT USING formats); the format is used again from its start for more items.

- Also `PRINT #n, USING`, and after other items: `PRINT "X ="; USING "##.#"; X`.
- A number too big for its field is printed in full after a `%`.

Example:

```
PRINT USING "###.##"; 3.14159; -2
```

Shows:

```
  3.14 -2.00
```

See also: [PRINT](#ref-print)

### PSET {#ref-pset}

*Not available* · `PSET (x, y)[, colour]`

Plots a point. Not available in this build: it gives **Feature not available** (error 73), as there is no screen of its own.

### PUT {#ref-put}

*Statement* · `PUT [#]n[, record]`

Writes random file `n`'s buffer as record `record` (from 1; default the one after the last).

- `PUT (x, y), a` (graphics) is not available: Feature not available.

Example:

```
LSET NM$ = "Ada": PUT #1, 1
```

See also: [GET](#ref-get), [FIELD](#ref-field)

### PWM {#ref-pwm}

*Statement* · `PWM pin, freq[, duty]`

Runs PWM on a pin at `freq` (8 Hz-10 MHz) with a duty of 0-100 % (default 50); `PWM pin, 0` stops it.

- Pins in pairs (GP0/1, GP2/3...) share one frequency, and GP16-31 share GP0-15's slices: changing one warns about its partner.

Example:

```
PWM LED, 1000, 10
```

See also: [SERVO](#ref-servo), [PIN](#ref-pin)

### RANDOMIZE {#ref-randomize}

*Statement* · `RANDOMIZE [seed]`

Starts RND's sequence afresh: from the clock without a seed, else from `seed` (the same seed, the same numbers).

Example:

```
RANDOMIZE: PRINT INT(RND * 6) + 1
```

Shows:

```
 4
```

See also: [RND](#ref-rnd)

### READ {#ref-read}

*Statement* · `READ var[, var...]`

Takes the next items from DATA lines, in order.

- Errors: `Out of DATA` (4); `Syntax error` reading a string into a number.

Example:

```
10 READ A, B$: PRINT A; B$
20 DATA 5, CATS
RUN
```

Shows:

```
 5 CATS
```

See also: [DATA](#ref-data), [RESTORE](#ref-restore)

### REM {#ref-rem}

*Statement* · `REM text | ' text`

A remark: the rest of the line is ignored. `'` is short for `:REM`.

Example:

```
10 REM Setup
20 X = 1 ' start
```

### RENUM {#ref-renum}

*Command* · `RENUM [new][, [old][, step]]`

Renumbers program lines from line `old` (default the first) as `new`, `new+step`... (default 10 and 10), and changes the line numbers after GOTO, GOSUB, THEN, ELSE, RESTORE and RUN to match.

Example:

```
RENUM 100, , 10
```

See also: [AUTO](#ref-auto), [DELETE](#ref-delete)

### RESET {#ref-reset}

*Statement* · `RESET`

Closes every open file.

See also: [CLOSE](#ref-close)

### RESTORE {#ref-restore}

*Statement* · `RESTORE [line]`

Makes READ start again from the first DATA (or from `line`).

Example:

```
RESTORE 500
```

See also: [READ](#ref-read), [DATA](#ref-data)

### RESUME {#ref-resume}

*Statement* · `RESUME | RESUME NEXT | RESUME line`

Ends an error handler: `RESUME` retries the statement that failed, `RESUME NEXT` goes on after it, `RESUME line` goes to the line.

- Errors: `RESUME without error` (20).

Example:

```
100 PRINT "skipped": RESUME NEXT
```

See also: [ON ERROR GOTO](#ref-on-error-goto)

### RETURN {#ref-return}

*Statement* · `RETURN`

Returns from a GOSUB (or an event subroutine) to the statement after the call. It also ends FOR and WHILE loops started in the subroutine.

- Errors: `RETURN without GOSUB` (3).

See also: [GOSUB](#ref-gosub)

### RGB {#ref-rgb}

*Function / statement* · `RGB(r, g, b)`

A colour for the graphics statements (256 + an RGB332 value): it works, but nothing here draws with it.

### RIGHT$ {#ref-right}

*Function* · `RIGHT$(s$, n)`

The last `n` characters of `s$`.

Example:

```
PRINT RIGHT$("HELLO", 3)
```

Shows:

```
LLO
```

See also: [LEFT$](#ref-left)

### RMDIR {#ref-rmdir}

*Statement* · `RMDIR path$`

Removes an empty folder.

- Errors: `Path/File Access Error` (75) if it isn't empty.

Example:

```
RMDIR "GAMES"
```

See also: [MKDIR](#ref-mkdir)

### RND {#ref-rnd}

*Function* · `RND | RND(x)`

A random number from 0 to just under 1. `RND(0)` repeats the last one; a negative `x` starts a sequence from it.

- RUN always starts the same sequence: RANDOMIZE for a different one.

Example:

```
PRINT INT(RND * 6) + 1
```

Shows:

```
 5
```

See also: [RANDOMIZE](#ref-randomize)

### RSET {#ref-rset}

*Statement* · `RSET var$ = s$`

Like LSET, right-aligned.

Example:

```
RSET N$ = STR$(42)
```

See also: [LSET](#ref-lset)

### RUN {#ref-run}

*Command* · `RUN [line] | RUN file$`

Runs the program from the start (or from `line`), after clearing the variables, closing files and resetting DATA; `RUN file$` loads the file first.

- Pins used by the last run are set back to inputs.

Example:

```
RUN "PRIMES"
```

Shows:

```
 168 primes below 1000
```

See also: [LOAD](#ref-load), [CONT](#ref-cont)

### SAVE {#ref-save}

*Command* · `SAVE file$[, A]`

Saves the program as plain text. `.BAS` is added if the name has no extension; `,A` is accepted (text is the only format).

- `SAVE "SD:GAME"` saves to the SD card; `SAVE "AUTORUN"` makes it run at power-up.

Example:

```
SAVE "HELLO"
```

See also: [LOAD](#ref-load), [FILES](#ref-files)

### SAVEIMAGE {#ref-saveimage}

*Not available* · `SAVEIMAGE file$`

Saves the screen as a BMP. Not available in this build: it gives **Feature not available** (error 73), as there is no screen of its own.

### SCREEN {#ref-screen}

*Not available* · `SCREEN mode[, , apage[, vpage]]`

Graphics modes (SCREEN 0 is accepted). Not available in this build: it gives **Feature not available** (error 73), as there is no screen of its own.

### SCREEN(r, c) {#ref-screenr-c}

*Function* · `SCREEN(row, col[, 1])`

The character at a screen position, or with 1 its colours. A terminal can't be read back, so it gives 32 (a space), or 7.

Example:

```
PRINT SCREEN(1, 1)
```

Shows:

```
 32
```

See also: [PEEK](#ref-peek)

### SERVO {#ref-servo}

*Statement* · `SERVO pin, angle`

Drives a hobby servo to `angle` 0-180 degrees (0.5-2.5 ms pulses at 50 Hz).

Example:

```
SERVO 16, 90
```

See also: [PWM](#ref-pwm)

### SGN {#ref-sgn}

*Function* · `SGN(x)`

1 if `x` is positive, -1 if negative, 0 if 0.

Example:

```
PRINT SGN(-7); SGN(0); SGN(3)
```

Shows:

```
-1  0  1
```

See also: [ABS](#ref-abs)

### SHELL {#ref-shell}

*Not available* · `SHELL command$`

Runs a pico-os shell command. Not available in this build: it gives **Feature not available** (error 73), as there is no screen of its own.

### SIN {#ref-sin}

*Function* · `SIN(x)`

The sine of `x` radians.

Example:

```
PRINT SIN(3.14159 / 2)
```

Shows:

```
 1
```

See also: [COS](#ref-cos), [TAN](#ref-tan), [ATN](#ref-atn)

### SOUND {#ref-sound}

*Statement* · `SOUND freq, ticks`

A tone of `freq` Hz (37-32767; 32767 is silence) for `ticks` (18.2 make a second). After PLAY "MB" it joins the background queue; ticks 0 stops the sound.

- Errors: `Illegal function call` (5) out of range.

Example:

```
SOUND 440, 18
```

See also: [BEEP](#ref-beep), [PLAY](#ref-play)

### SPACE$ {#ref-space}

*Function* · `SPACE$(n)`

A string of `n` (0-255) spaces.

Example:

```
PRINT "["; SPACE$(3); "]"
```

Shows:

```
[   ]
```

See also: [STRING$](#ref-string), [SPC](#ref-spc)

### SPC {#ref-spc}

*Function* · `SPC(n)`

In PRINT, prints `n` spaces.

Example:

```
PRINT "A"; SPC(3); "B"
```

Shows:

```
A   B
```

See also: [TAB](#ref-tab), [SPACE$](#ref-space)

### SPI {#ref-spi}

*Statement* · `SPI OPEN [sck, mosi, miso[, khz[, mode]]] | SPI WRITE item[, item...] | SPI CLOSE`

The chip's SPI. OPEN starts it (default SPI0: GP6 SCK, GP7 MOSI, GP4 MISO; 1000 kHz, 1-50000; mode 0-3), WRITE sends bytes ignoring what comes back, CLOSE frees the pins. Drive chip select yourself with PIN (GP5 is free for it).

- SPI1 (GP10-15) is the SD card's while a card is in.
- Errors: `Device unavailable` (68) before OPEN.

Example:

```
PIN 5, 1: SPI OPEN: PIN 5, 0: SPI WRITE &H9F: ID$ = SPI$(STRING$(3, 0)): PIN 5, 1
```

See also: [SPI$](#ref-spi-1), [I2C](#ref-i2c)

### SPI$ {#ref-spi-1}

*Function* · `SPI$(out$)`

Sends the bytes of `out$` on the open SPI and returns the bytes that came back meanwhile (as many).

Example:

```
R$ = SPI$(STRING$(4, 0))
```

See also: [SPI](#ref-spi)

### SPRITE {#ref-sprite}

*Not available* · `SPRITE n, x, y... | SPRITE UPDATE...`

Moving pictures. Not available in this build: it gives **Feature not available** (error 73), as there is no screen of its own.

### SPRITEHIT {#ref-spritehit}

*Function / statement* · `SPRITEHIT(n[, first, last]) | SPRITEX(n) | SPRITEY(n) | SPRITEON(n)`

Sprites' positions and collisions: there are none (SPRITEHIT gives -1, the rest 0).

### SQR {#ref-sqr}

*Function* · `SQR(x)`

The square root of `x` (0 or more).

- Errors: `Illegal function call` (5) for a negative `x`.

Example:

```
PRINT SQR(2)
```

Shows:

```
 1.41421356
```

### STOP {#ref-stop}

*Statement* · `STOP`

Stops the program with `Break in <line>`; CONT carries on. Files stay open.

See also: [END](#ref-end), [CONT](#ref-cont)

### STR$ {#ref-str}

*Function* · `STR$(x)`

The number `x` as PRINT shows it, with a space before a positive number.

Example:

```
PRINT "<"; STR$(42); ">"
```

Shows:

```
< 42>
```

See also: [VAL](#ref-val)

### STRING$ {#ref-string}

*Function* · `STRING$(n, c | c$)`

`n` (0-255) copies of character code `c` or of the first character of `c$`.

Example:

```
PRINT STRING$(5, "*"); STRING$(3, 65)
```

Shows:

```
*****AAA
```

See also: [SPACE$](#ref-space)

### SWAP {#ref-swap}

*Statement* · `SWAP a, b`

Exchanges two variables of the same type.

- Errors: `Type mismatch` (13).

Example:

```
A = 1: B = 2: SWAP A, B: PRINT A; B
```

Shows:

```
 2  1
```

See also: [LET](#ref-let)

### SYSTEM {#ref-system}

*Command* · `SYSTEM`

Leaves BASIC and starts it afresh, as after a reset (the files on the disks stay; AUTORUN.BAS isn't run again).

See also: [NEW](#ref-new)

### TAB {#ref-tab}

*Function* · `TAB(n)`

In PRINT, moves to column `n` (1-80); to the next line first if the cursor is already past it.

Example:

```
PRINT "A"; TAB(10); "B"
```

Shows:

```
A        B
```

See also: [SPC](#ref-spc), [LOCATE](#ref-locate)

### TAN {#ref-tan}

*Function* · `TAN(x)`

The tangent of `x` radians.

Example:

```
PRINT TAN(.7854)
```

Shows:

```
 1.00000367
```

See also: [ATN](#ref-atn)

### TEMP {#ref-temp}

*Function* · `TEMP()`

The chip's temperature in °C (uncalibrated: within a few degrees).

Example:

```
PRINT INT(TEMP() * 10) / 10
```

Shows:

```
 27.3
```

See also: [ADC](#ref-adc)

### TIME$ {#ref-time}

*Statement / function* · `TIME$ = "hh:mm:ss" | TIME$`

Sets the time of day, or gives it as "hh:mm:ss" (24-hour, UTC).

Example:

```
TIME$ = "09:30:00": PRINT TIME$
```

Shows:

```
09:30:00
```

See also: [DATE$](#ref-date), [TIMER](#ref-timer)

### TIMER {#ref-timer}

*Function* · `TIMER`

Seconds since the board started, with fractions: for timing things.

Example:

```
T = TIMER: PAUSE 500: PRINT TIMER - T
```

Shows:

```
 .500118999
```

See also: [PAUSE](#ref-pause), [ON TIMER](#ref-on-timer)

### TIMER ON {#ref-timer-on}

*Statement* · `TIMER ON | OFF | STOP`

Turns the timer event (ON TIMER) on, off, or holds it.

See also: [ON TIMER](#ref-on-timer)

### TROFF {#ref-troff}

*Command* · `TROFF`

Turns line tracing off.

See also: [TRON](#ref-tron)

### TRON {#ref-tron}

*Command* · `TRON`

Turns line tracing on: each line's number is printed in brackets as it runs.

Example:

```
TRON: RUN
```

Shows:

```
[10][20][10][20]...
```

See also: [TROFF](#ref-troff)

### UCASE$ {#ref-ucase}

*Function* · `UCASE$(s$)`

`s$` in upper case.

Example:

```
PRINT UCASE$("Hello")
```

Shows:

```
HELLO
```

See also: [LCASE$](#ref-lcase)

### VAL {#ref-val}

*Function* · `VAL(s$)`

The number at the start of `s$` (spaces skipped; `&H` hex and `&O` octal too); 0 if there isn't one.

Example:

```
PRINT VAL("12.5KG"); VAL("&HFF"); VAL("X")
```

Shows:

```
 12.5  255  0
```

See also: [STR$](#ref-str)

### VIEW {#ref-view}

*Not available* · `VIEW (x1, y1)-(x2, y2)...`

A graphics viewport (VIEW PRINT works). Not available in this build: it gives **Feature not available** (error 73), as there is no screen of its own.

### VIEW PRINT {#ref-view-print}

*Statement* · `VIEW PRINT [top TO bottom]`

Makes only rows `top` to `bottom` scroll (an ANSI scroll region); `VIEW PRINT` alone undoes it.

Example:

```
VIEW PRINT 5 TO 20
```

See also: [LOCATE](#ref-locate), [CLS](#ref-cls)

### WEND {#ref-wend}

*Statement* · `WEND`

Ends a WHILE loop.

- Errors: `WEND without WHILE` (30).

See also: [WHILE](#ref-while)

### WHILE {#ref-while}

*Statement* · `WHILE cond ... WEND`

Repeats the lines up to WEND while `cond` is true (not 0); tested before each pass.

- Up to 32 nested.
- Errors: `WHILE without WEND` (29).

Example:

```
I = 1: WHILE I < 100: I = I * 2: WEND: PRINT I
```

Shows:

```
 128
```

See also: [FOR](#ref-for), [IF](#ref-if)

### WIDTH {#ref-width}

*Statement* · `WIDTH n`

Accepted for old programs; the screen stays 80 columns.

### WINDOW {#ref-window}

*Not available* · `WINDOW (x1, y1)-(x2, y2)`

Graphics coordinates. Not available in this build: it gives **Feature not available** (error 73), as there is no screen of its own.

### WRITE {#ref-write}

*Statement* · `WRITE [items]`

Prints the items separated by commas, strings in quotes, numbers without spaces: what INPUT # reads back exactly.

Example:

```
WRITE "Ann", 120
```

Shows:

```
"Ann",120
```

See also: [WRITE #](#ref-write-1), [PRINT](#ref-print)

### WRITE # {#ref-write-1}

*Statement* · `WRITE #n, items`

Writes the items to file `n` separated by commas, strings in quotes: INPUT # reads them back exactly.

Example:

```
WRITE #1, "Ann", 120
```

See also: [INPUT #](#ref-input-1)

### XOR {#ref-xor}

*Operator* · `a XOR b`

Bitwise exclusive OR: true if just one is.

Example:

```
PRINT 12 XOR 10
```

Shows:

```
 6
```

See also: [OR](#ref-or)

### XRECV {#ref-xrecv}

*Statement* · `XRECV file$`

Receives a file from the terminal by XMODEM and saves it as `file$`, replacing one of that name. Start the terminal's XMODEM send within a minute.

- CRC-16 or checksum; 128- or 1K-byte blocks. XMODEM's ^Z padding is taken off the end.
- A failed transfer deletes the partial file and gives `Device I/O error` (57). Ctrl-X a few times cancels.
- Works to either drive: `XRECV "SD:PIC.BMP"`.

Example:

```
XRECV "GAME.BAS"
```

Shows:

```
Start the XMODEM send on the terminal now.
Received 1 file, 2048 bytes.
```

See also: [XSEND](#ref-xsend), [LOAD](#ref-load)

### XSEND {#ref-xsend}

*Statement* · `XSEND file$`

Sends a file to the terminal by XMODEM (1K blocks with CRC-16, or 128-byte ones with a checksum if the receiver asks).

- Errors: `Device I/O error` (57) if it fails or the file can't be read.

Example:

```
XSEND "GAME.BAS"
```

Shows:

```
Start the XMODEM receive on the terminal now.
Sent 1 file, 2048 bytes.
```

See also: [XRECV](#ref-xrecv)

### \ and MOD {#ref-and-mod}

*Operator* · `a \ b, a MOD b`

Integer division (both rounded to whole numbers first, the result cut towards 0) and its remainder.

Example:

```
PRINT 17 \ 5; 17 MOD 5
```

Shows:

```
 3  2
```

See also: [+ - * / ^](#ref--)

## Appendix A: error codes

| Code | Message | Code | Message |
|---|---|---|---|
| 1 | NEXT without FOR | 26 | FOR without NEXT |
| 2 | Syntax error | 29 | WHILE without WEND |
| 3 | RETURN without GOSUB | 30 | WEND without WHILE |
| 4 | Out of DATA | 50 | FIELD overflow |
| 5 | Illegal function call | 52 | Bad file number |
| 6 | Overflow | 53 | File not found |
| 7 | Out of memory | 54 | Bad file mode |
| 8 | Undefined line number | 55 | File already open |
| 9 | Subscript out of range | 57 | Device I/O error |
| 10 | Duplicate Definition | 58 | File already exists |
| 11 | Division by zero | 61 | Disk full |
| 12 | Illegal direct | 62 | Input past end |
| 13 | Type mismatch | 63 | Bad record number |
| 15 | String too long | 64 | Bad file name |
| 17 | Can't continue | 67 | Too many files |
| 18 | Undefined user function | 68 | Device unavailable |
| 19 | No RESUME | 70 | Permission denied |
| 20 | RESUME without error | 71 | Disk not ready |
| 22 | Missing operand | 73 | Feature not available |
| 24 | Device Timeout | 75 | Path/File Access Error |
| 25 | Device Fault | 76 | Path not found |

(Other GW-BASIC codes, 14, 16, 23, 27, 51, 66, 69, 72 and 74, have their messages for ERROR n; unknown codes print `Unprintable error`.)

## Appendix B: PRINT USING formats

| Field | Meaning |
|---|---|
| `#` | a digit position |
| `.` | the decimal point |
| `,` before the point | commas between thousands |
| `+` first or last | always show the sign there |
| `-` last | a minus after negative numbers |
| `**` | fill the space before the number with `*` |
| `$$` | a `$` just before the number (`**$` does both) |
| `^^^^` | exponent form (`##.##^^^^` prints ` 2.35E+02`) |
| `!` | the first character of a string |
| `\  \` | the first n + 2 characters of a string |
| `&` | the whole string |
| `_` | prints the next character as it is |

## Appendix C: INKEY$ key codes

Ordinary keys give their character. These give CHR$(0) followed by the PC scan code:

| Key | Code | Key | Code |
|---|---|---|---|
| Up | 72 | Home | 71 |
| Down | 80 | End | 79 |
| Left | 75 | PgUp | 73 |
| Right | 77 | PgDn | 81 |
| F1-F10 | 59-68 | Ins | 82 |
| | | Del | 83 |

Esc gives CHR$(27), Enter CHR$(13), Backspace CHR$(8), Tab CHR$(9), Ctrl-A to Ctrl-Z CHR$(1)-CHR$(26) (Ctrl-C stops the program).

## Appendix D: characters

BASIC uses the IBM PC character set (code page 437) and sends each character to the terminal as its Unicode picture in UTF-8: CHR$(1) ☺, CHR$(3) ♥, CHR$(176)-CHR$(178) ░▒▓, CHR$(179) │, CHR$(196) ─, CHR$(218) ┌, CHR$(219) █, CHR$(227) π, CHR$(248) °. Control codes print their pictures too, except 7 (beep), 8 (backspace), 9 (tab), 10 (line feed), 12 (clear screen), 13 (carriage return) and 27 (Esc, for ANSI codes); 11 moves the cursor home and 28-31 move it right, left, up and down.

## Appendix E: pins BASIC uses

| Pin | Used by | Boards |
|---|---|---|
| GP18 | the speaker (BEEP, SOUND, PLAY) | all (the Maker Pi Pico's buzzer) |
| GP10-15 | the SD card, while one is in | all (the Maker Pi Pico's slot) |
| GP47 | the PSRAM | Pico Plus 2 |
| GP28 | NEOPIXEL's default (the Maker Pi Pico's RGB LED) | while open |
| GP20 | held low at power-up: skip AUTORUN.BAS | all (the Maker Pi Pico's first button) |
| GP2/3, GP4-7, GP8/9 | I2C, SPI and COM1's defaults | while open |

The console is the board's USB port, so GP0 and GP1 are free. PINS lists what each pin is doing.

