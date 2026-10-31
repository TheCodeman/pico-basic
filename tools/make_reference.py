#!/usr/bin/env python3
# Makes docs/BASIC_REFERENCE.md, the A-Z reference of every statement,
# command, function and operator, from the entries below, and checks that
# every keyword in src/basic.c's tables has an entry:
#   python3 tools/make_reference.py
# tools/make_manual.py then puts it after the manual in the PDF and HTML.
import os, re, sys

import markdown.extensions.toc as toc

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')


# (heading, kind, syntax, explanation, [details], example, output, [see also], topic)
# An example's output is what the terminal shows (numbers print with a space
# before them, and one after).
E = []
def e(name, kind, syntax, text, details=(), example='', output='', see=(), topic=''):
    E.append(dict(name=name, kind=kind, syntax=syntax, text=text, details=list(details),
                  example=example, output=output, see=list(see), topic=topic))

# ---- Program and files -------------------------------------------------------------
P = 'Programs and the disks'
e('AUTO', 'Command', 'AUTO [start][, step]',
  'Numbers the lines you type: each new line starts with the next number, so you just type the statement.',
  ['`start` and `step` default to 10 and 10.',
   'A `*` after the number means that line already exists: Enter keeps it as it is.',
   'An empty line where no line exists, or Ctrl-C, stops it.'],
  'AUTO 100, 5', '100 ', ['RENUM', 'DELETE'], P)
e('BYE', 'Command', 'BYE', 'The same as SYSTEM.', [], '', '', ['SYSTEM'], P)
e('CHAIN', 'Statement', 'CHAIN [MERGE] file$[, [line][, ALL][, DELETE range]]',
  'Runs another program from a program, keeping the files that are open.',
  ['Only the variables named in COMMON are passed on, or all of them with `ALL`.',
   '`line` is where the new program starts (default its first line).',
   '`CHAIN MERGE` adds the file\'s lines to the program (after deleting `range`) and keeps every variable.',
   '`.BAS` is added if the name has no extension.'],
  '10 COMMON SCORE\n20 SCORE = 100: CHAIN "PART2"\nRUN', '', ['COMMON', 'MERGE', 'RUN'], P)
e('CHDIR', 'Statement', 'CHDIR path$',
  'Makes a folder the current one, or a drive the current drive.',
  ['`CHDIR "GAMES"`, `CHDIR ".."`, `CHDIR "/"` move between folders.',
   '`CHDIR "SD:"` makes the SD card the current drive, `CHDIR "SD:/GAMES"` its folder too, `CHDIR "FLASH:"` goes back.',
   'Errors: `Path not found` (76).'],
  'MKDIR "GAMES": CHDIR "GAMES": FILES', 'File not found\n3080192 Bytes free', ['MKDIR', 'RMDIR', 'FILES'], P)
e('CLEAR', 'Statement', 'CLEAR [, n[, m]]',
  'Deletes every variable and array, closes every file, and forgets DATA, FOR, GOSUB and WHILE positions and event traps. The program stays.',
  ['Any numbers after it (GW-BASIC\'s memory sizes) are accepted and ignored.'],
  'A = 5: CLEAR: PRINT A', ' 0', ['NEW', 'ERASE'], P)
e('COMMON', 'Statement', 'COMMON var[, var...]',
  'Names the variables CHAIN passes to the next program. Arrays are written with empty brackets: `COMMON A(), N$`.',
  ['Up to 64 names.'], 'COMMON SCORE, NAME$', '', ['CHAIN'], P)
e('CONT', 'Command', 'CONT',
  'Carries on with the program after Ctrl-C or STOP, from where it stopped.',
  ['Errors: `Can\'t continue` (17) after the program has been changed or after an error.'],
  '', '', ['STOP', 'END'], P)
e('DELETE', 'Command', 'DELETE range',
  'Deletes program lines. `range` is `n`, `n-`, `-n` or `n-m`.',
  ['Errors: `Illegal function call` (5) if there is nothing in the range.'],
  'DELETE 100-190', '', ['NEW', 'AUTO'], P)
e('EDIT', 'Command', 'EDIT | EDIT file$ | EDIT n | EDIT .',
  '`EDIT` opens the program in the full-screen editor (see the manual\'s Full-screen editor section); `EDIT file$` edits any text file there. `EDIT n` (or `.`, the last line typed or the one that failed) puts program line `n` on the input line to change; Enter stores it.',
  ['In the full-screen editor Ctrl-X/C/V cut, copy and paste lines, Ctrl-Z/R undo and redo, Ctrl-S saves and Esc leaves. A program is only taken if every non-blank line starts with a line number (0-65529, 255 characters at most): otherwise nothing is saved and the cursor goes to the bad line.',
   'Leaving without saving keeps the program as it was. Blank lines are left out.'],
  'EDIT 20', '20 PRINT "HELLO"', ['LIST', 'AUTO'], P)
e('FILES', 'Command', 'FILES [pattern$]',
  'Lists files, four to a line (folders marked `<DIR>`), then the free space on that drive.',
  ['The pattern may have `*` and `?` and a drive and folder in front: `FILES "*.BAS"`, `FILES "SD:"`, `FILES "SD:/GAMES/*.BAS"`.',
   'Without one, all files in the current folder.',
   '`File not found` is printed (not an error) when nothing matches.'],
  'FILES "*.BAS"', 'PRIMES.BAS          NEODEMO.BAS\n3084288 Bytes free', ['DIR$', 'KILL', 'CHDIR'], P)
e('KILL', 'Statement', 'KILL file$',
  'Deletes a file. No extension is added.',
  ['Errors: `File not found` (53); `File already open` (55) while it is open.'],
  'KILL "OLD.BAS"', '', ['NAME', 'FILES'], P)
e('LIST', 'Command', 'LIST [range]',
  'Lists the program, pausing after each 28 lines (any key goes on, Q stops). `range` is `n`, `n-`, `-n` or `n-m`.',
  [], 'LIST 10-30', '10 PRINT "HELLO"\n20 GOTO 10', ['LLIST', 'EDIT'], P)
e('LLIST', 'Command', 'LLIST [range]', 'The same as LIST: there is no printer.', [], '', '', ['LIST'], P)
e('LOAD', 'Command', 'LOAD file$[, R]',
  'Loads a program, replacing the one in memory; with `,R` runs it too.',
  ['`.BAS` is added if the name has no extension.',
   'Plain text (CR LF or LF lines) and GW-BASIC tokenised files are read.',
   'Errors: `File not found` (53).'],
  'LOAD "PRIMES": RUN', ' 168 primes below 1000', ['SAVE', 'RUN', 'MERGE'], P)
e('MERGE', 'Command', 'MERGE file$',
  'Adds the lines of a plain-text program to the one in memory; lines with the same number are replaced.',
  [], 'MERGE "SUBS"', '', ['LOAD', 'CHAIN'], P)
e('MKDIR', 'Statement', 'MKDIR path$', 'Makes a folder.',
  ['Errors: `Path/File Access Error` (75) if it exists.'], 'MKDIR "SD:/GAMES"', '', ['CHDIR', 'RMDIR'], P)
e('NAME', 'Statement', 'NAME old$ AS new$', 'Renames a file (or moves it to another folder on the same drive).',
  ['Errors: `File not found` (53), `File already exists` (58).'], 'NAME "A.BAS" AS "B.BAS"', '', ['KILL'], P)
e('FORMAT', 'Command', 'FORMAT ON | FORMAT OFF | FORMAT n',
  'Whether LIST, EDIT and SAVE indent the program: FOR ... NEXT and WHILE ... WEND bodies by `n` spaces a level (2 to start with, 1-8), and in a program without line numbers the lines under each label (the label stays at the left).',
  ['Only the look changes: loading a program takes the indent off again.', 'On at start-up.'],
  '10 FOR I = 1 TO 2\n20 PRINT I\n30 NEXT\nLIST', '10 FOR I = 1 TO 2\n20   PRINT I\n30 NEXT', ['LIST', 'EDIT', 'SAVE'], P)
e('NUMBERS', 'Command', 'NUMBERS OFF | NUMBERS ON',
  '`NUMBERS OFF` takes the line numbers off the program: each line a GOTO, GOSUB, THEN, ELSE, RESTORE, RESUME or RUN goes to gets a label, L and its old number (`L100:`), and those use the label. `NUMBERS ON` numbers the lines 10, 20, 30... again, keeping the labels.',
  ['A program without numbers is listed, saved and edited without them; its lines\' numbers (in errors, ERL, TRON) are their lines in the file.',
   'Numbers compared with ERL (`IF ERL = 100`) aren\'t changed.'],
  '10 PRINT "HI": GOTO 10\nNUMBERS OFF\nLIST', 'L10: PRINT "HI": GOTO L10', ['RENUM', 'GOTO', 'EDIT'], P)
e('NEW', 'Command', 'NEW', 'Deletes the program and its variables, closes files, and frees the pins BASIC used.',
  [], '', '', ['CLEAR', 'DELETE'], P)
e('RENUM', 'Command', 'RENUM [new][, [old][, step]]',
  'Renumbers program lines from line `old` (default the first) as `new`, `new+step`... (default 10 and 10), and changes the line numbers after GOTO, GOSUB, THEN, ELSE, RESTORE, RESUME and RUN to match.',
  ['Not for a program without line numbers (NUMBERS ON first).'], 'RENUM 100, , 10', '', ['AUTO', 'DELETE'], P)
e('RESET', 'Statement', 'RESET', 'Closes every open file.', [], '', '', ['CLOSE'], P)
e('RMDIR', 'Statement', 'RMDIR path$', 'Removes an empty folder.',
  ['Errors: `Path/File Access Error` (75) if it isn\'t empty.'], 'RMDIR "GAMES"', '', ['MKDIR'], P)
e('RUN', 'Command', 'RUN [line | label] | RUN file$',
  'Runs the program from the start (or from `line`), after clearing the variables, closing files and resetting DATA; `RUN file$` loads the file first.',
  ['Pins used by the last run are set back to inputs.'], 'RUN "PRIMES"', ' 168 primes below 1000', ['LOAD', 'CONT'], P)
e('SAVE', 'Command', 'SAVE file$[, A]',
  'Saves the program as plain text. `.BAS` is added if the name has no extension; `,A` is accepted (text is the only format).',
  ['`SAVE "SD:GAME"` saves to the SD card; `SAVE "AUTORUN"` makes it run at power-up.'],
  'SAVE "HELLO"', '', ['LOAD', 'FILES'], P)
e('SYSTEM', 'Command', 'SYSTEM', 'Leaves BASIC and starts it afresh, as after a reset (the files on the disks stay; AUTORUN.BAS isn\'t run again).',
  [], '', '', ['NEW'], P)
e('TROFF', 'Command', 'TROFF', 'Turns line tracing off.', [], '', '', ['TRON'], P)
e('TRON', 'Command', 'TRON', 'Turns line tracing on: each line\'s number is printed in brackets as it runs.',
  [], 'TRON: RUN', '[10][20][10][20]...', ['TROFF'], P)
e('XRECV', 'Statement', 'XRECV file$',
  'Receives a file from the terminal by XMODEM and saves it as `file$`, replacing one of that name. Start the terminal\'s XMODEM send within a minute.',
  ['CRC-16 or checksum; 128- or 1K-byte blocks. XMODEM\'s ^Z padding is taken off the end.',
   'A failed transfer deletes the partial file and gives `Device I/O error` (57). Ctrl-X a few times cancels.',
   'Works to either drive: `XRECV "SD:PIC.BMP"`.'],
  'XRECV "GAME.BAS"', 'Start the XMODEM send on the terminal now.\nReceived 1 file, 2048 bytes.', ['XSEND', 'LOAD'], P)
e('XSEND', 'Statement', 'XSEND file$',
  'Sends a file to the terminal by XMODEM (1K blocks with CRC-16, or 128-byte ones with a checksum if the receiver asks).',
  ['Errors: `Device I/O error` (57) if it fails or the file can\'t be read.'],
  'XSEND "GAME.BAS"', 'Start the XMODEM receive on the terminal now.\nSent 1 file, 2048 bytes.', ['XRECV'], P)

# ---- Flow of control --------------------------------------------------------------
F = 'Flow of control and errors'
e('END', 'Statement', 'END', 'Ends the program and closes its files.', [], '10 PRINT "DONE": END\nRUN', 'DONE', ['STOP'], F)
e('ERL', 'Function', 'ERL', 'The line number where the last error happened (0 in direct mode).', [],
  '100 PRINT "Error"; ERR; "in"; ERL: RESUME NEXT', '', ['ERR', 'ON ERROR GOTO'], F)
e('ERR', 'Function', 'ERR', 'The code of the last error (see Error codes).', [],
  '10 ON ERROR GOTO 100\n20 X = 1 / 0\n30 END\n100 PRINT ERR: RESUME NEXT\nRUN', ' 11', ['ERL', 'ERROR'], F)
e('ERROR', 'Statement', 'ERROR n', 'Raises error `n` (1-255), as if it had happened: a test, or a program\'s own error codes.',
  ['Codes without a message print `Unprintable error`.'], 'ERROR 53', 'File not found', ['ON ERROR GOTO', 'ERR'], F)
e('EXIT FOR', 'Statement', 'EXIT FOR', 'Leaves the innermost FOR loop at once, going on after its NEXT (from QuickBASIC).',
  [], '10 FOR I = 1 TO 10: IF I = 3 THEN EXIT FOR\n20 NEXT: PRINT I\nRUN', ' 3', ['FOR'], F)
e('FOR', 'Statement', 'FOR v = a TO b [STEP s] ... NEXT [v]',
  'Repeats the lines up to NEXT with `v` = a, a+s, ... while it hasn\'t passed `b`. STEP defaults to 1; a negative STEP counts down.',
  ['The loop body is skipped when `a` is already past `b`.', 'Up to 32 loops nested.'],
  'FOR I = 10 TO 1 STEP -3: PRINT I;: NEXT', ' 10  7  4  1', ['NEXT', 'EXIT FOR', 'WHILE'], F)
e('GOSUB', 'Statement', 'GOSUB line | GOSUB label', 'Calls the subroutine at `line` (or `label`); RETURN comes back to the statement after the GOSUB.',
  ['Up to 32 deep.', 'Errors: `Undefined line number` (8), `Undefined label` (80).'], '10 GOSUB 100: END\n100 PRINT "SUB": RETURN\nRUN', 'SUB', ['RETURN', 'ON...GOSUB'], F)
e('GOTO', 'Statement', 'GOTO line | GOTO label', 'Goes on at `line`, or at the line that starts with `label:` (a name and a colon).',
  ['Labels: letters, digits, `.` and `_`, up to 40, any case; a program can have them with line numbers or without.', 'Errors: `Undefined line number` (8), `Undefined label` (80), `Duplicate label` (81, at RUN).'],
  '10 PRINT "HI": GOTO 10\nRUN', 'HI\nHI\n...', ['GOSUB', 'IF'], F)
e('IF', 'Statement', 'IF cond THEN statements [ELSE statements] | IF cond THEN line | IF cond GOTO line',
  'Runs the statements after THEN when `cond` is not 0 (true), else those after ELSE. A line number or label after THEN or ELSE jumps there (a label when nothing else follows it).',
  ['Comparisons give -1 (true) or 0 (false); any number not 0 counts as true.', 'Everything to the end of the line belongs to THEN or ELSE.'],
  'A = 5: IF A > 3 THEN PRINT "BIG" ELSE PRINT "SMALL"', 'BIG', ['WHILE', 'ON...GOTO'], F)
e('NEXT', 'Statement', 'NEXT [v[, v...]]', 'Ends a FOR loop: adds the step and goes round again unless the end is passed. `NEXT J, I` ends two loops.',
  ['Errors: `NEXT without FOR` (1).'], 'FOR I = 1 TO 3: PRINT I;: NEXT I', ' 1  2  3', ['FOR'], F)
e('ON...GOSUB', 'Statement', 'ON n GOSUB line1[, line2...] (lines or labels)', 'Calls the n-th subroutine in the list. If `n` is 0 or past the list, it goes on with the next statement.',
  [], '10 ON 2 GOSUB 100, 200: END\n100 PRINT "A": RETURN\n200 PRINT "B": RETURN\nRUN', 'B', ['ON...GOTO', 'GOSUB'], F)
e('ON...GOTO', 'Statement', 'ON n GOTO line1[, line2...] (lines or labels)', 'Goes to the n-th line in the list; with `n` 0 or past the list, goes on.',
  [], 'ON 1 GOTO 100, 200', '', ['ON...GOSUB'], F)
e('ON ERROR GOTO', 'Statement', 'ON ERROR GOTO line | ON ERROR GOTO label | ON ERROR GOTO 0',
  'Sends errors in a running program to an error handler at `line`. `ON ERROR GOTO 0` turns that off; inside a handler it reports the error and stops.',
  ['ERR and ERL say what and where; RESUME ends the handler.', 'Ctrl-C can\'t be trapped.', 'A program that ends inside a handler stops with `No RESUME` (19).'],
  '10 ON ERROR GOTO 100\n20 OPEN "NONE" FOR INPUT AS #1\n30 END\n100 PRINT "no file": RESUME 30\nRUN', 'no file', ['RESUME', 'ERR', 'ERROR'], F)
e('PAUSE', 'Statement', 'PAUSE ms', 'Waits `ms` milliseconds (an extension); Ctrl-C still stops it.', [], 'PAUSE 500', '', ['TIMER'], F)
e('REM', 'Statement', "REM text | ' text", 'A remark: the rest of the line is ignored. `\'` is short for `:REM`.', [], "10 REM Setup\n20 X = 1 ' start", '', [], F)
e('RESUME', 'Statement', 'RESUME | RESUME NEXT | RESUME line | RESUME label',
  'Ends an error handler: `RESUME` retries the statement that failed, `RESUME NEXT` goes on after it, `RESUME line` goes to the line.',
  ['Errors: `RESUME without error` (20).'], '100 PRINT "skipped": RESUME NEXT', '', ['ON ERROR GOTO'], F)
e('RETURN', 'Statement', 'RETURN', 'Returns from a GOSUB (or an event subroutine) to the statement after the call. It also ends FOR and WHILE loops started in the subroutine.',
  ['Errors: `RETURN without GOSUB` (3).'], '', '', ['GOSUB'], F)
e('STOP', 'Statement', 'STOP', 'Stops the program with `Break in <line>`; CONT carries on. Files stay open.', [], '', 'Break in 40', ['END', 'CONT'], F)
e('WEND', 'Statement', 'WEND', 'Ends a WHILE loop.', ['Errors: `WEND without WHILE` (30).'], '', '', ['WHILE'], F)
e('WHILE', 'Statement', 'WHILE cond ... WEND', 'Repeats the lines up to WEND while `cond` is true (not 0); tested before each pass.',
  ['Up to 32 nested.', 'Errors: `WHILE without WEND` (29).'], 'I = 1: WHILE I < 100: I = I * 2: WEND: PRINT I', ' 128', ['FOR', 'IF'], F)

# ---- Variables and data ------------------------------------------------------------
V = 'Variables and data'
e('DATA', 'Statement', 'DATA item[, item...]', 'Holds numbers and strings for READ. Strings may be quoted (needed if they hold commas or start with spaces).',
  [], '10 DATA 3, "RED", 2.5', '', ['READ', 'RESTORE'], V)
e('DEF FN', 'Statement', 'DEF FNname[(arg[, arg...])] = expr', 'Defines a one-line function, called as FNname(...). String functions end in $.',
  ['Only in a program line: in direct mode it is `Illegal direct` (12).', 'Errors: `Undefined user function` (18) for one not defined.'], '10 DEF FNSQ(X) = X * X: PRINT FNSQ(7)\nRUN', ' 49', ['GOSUB'], V)
e('DEF SEG', 'Statement', 'DEF SEG [= segment]', 'Chooses the memory segment PEEK, POKE, BSAVE and BLOAD use: only `&HB800` (the text screen) and `0` (BIOS data) are emulated.',
  [], 'DEF SEG = 0: PRINT PEEK(&H44A)', ' 80', ['PEEK', 'POKE'], V)
e('DEFDBL', 'Statement', 'DEFDBL letters', 'Names starting with these letters (and no type character) are double precision: `DEFDBL A-C, X`.', ['RUN, NEW and CLEAR reset it.'], 'DEFDBL D: D = 1 / 3: PRINT D', ' .333333333', ['DEFINT', 'DEFSNG', 'DEFSTR'], V)
e('DEFINT', 'Statement', 'DEFINT letters', 'Names starting with these letters are integers (-32768 to 32767): `DEFINT A-Z` makes all plain names integers.', [], 'DEFINT I: I = 2.7: PRINT I', ' 3', ['DEFDBL', 'DEFSNG', 'DEFSTR'], V)
e('DEFSNG', 'Statement', 'DEFSNG letters', 'Names starting with these letters are single precision (the default).', [], '', '', ['DEFINT', 'DEFDBL'], V)
e('DEFSTR', 'Statement', 'DEFSTR letters', 'Names starting with these letters are strings, without a `$`.', [], 'DEFSTR S: S = "HI": PRINT S', 'HI', ['DEFINT'], V)
e('DIM', 'Statement', 'DIM name(size[, size...])[, ...]', 'Makes an array with subscripts 0 (or 1 after OPTION BASE 1) to `size`, up to 8 dimensions. An array used without DIM gets 0-10 in each dimension.',
  ['Errors: `Duplicate Definition` (10) if it exists; `Subscript out of range` (9) for a subscript too big; `Out of memory` (7).'],
  'DIM A(5), B$(2, 3): A(5) = 7: PRINT A(5)', ' 7', ['ERASE', 'OPTION BASE'], V)
e('ERASE', 'Statement', 'ERASE array[, array...]', 'Deletes arrays, freeing their memory, so they can be dimensioned again.', [], 'DIM A(100): ERASE A: DIM A(5)', '', ['DIM'], V)
e('LET', 'Statement', '[LET] var = expr', 'Gives a variable a value. `LET` itself is optional.', ['Errors: `Type mismatch` (13) for a string into a number or the other way; `Overflow` (6) out of an integer\'s range.'], 'LET A = 5: B$ = "X"', '', ['SWAP'], V)
e('MID$ =', 'Statement', 'MID$(v$, start[, n]) = x$', 'Replaces characters of `v$` in place, from `start`, with those of `x$` (at most `n`); `v$` keeps its length.', [], 'A$ = "HELLO": MID$(A$, 2, 3) = "ipp": PRINT A$', 'HippO', ['MID$'], V)
e('OPTION BASE', 'Statement', 'OPTION BASE 0 | 1', 'The lowest array subscript, before any array is used.', [], 'OPTION BASE 1: DIM A(3)', '', ['DIM'], V)
e('RANDOMIZE', 'Statement', 'RANDOMIZE [seed]', 'Starts RND\'s sequence afresh: from the clock without a seed, else from `seed` (the same seed, the same numbers).', [], 'RANDOMIZE: PRINT INT(RND * 6) + 1', ' 4', ['RND'], V)
e('READ', 'Statement', 'READ var[, var...]', 'Takes the next items from DATA lines, in order.', ['Errors: `Out of DATA` (4); `Syntax error` reading a string into a number.'], '10 READ A, B$: PRINT A; B$\n20 DATA 5, CATS\nRUN', ' 5 CATS', ['DATA', 'RESTORE'], V)
e('RESTORE', 'Statement', 'RESTORE [line | label]', 'Makes READ start again from the first DATA (or from `line` or `label`).', [], 'RESTORE 500', '', ['READ', 'DATA'], V)
e('SWAP', 'Statement', 'SWAP a, b', 'Exchanges two variables of the same type.', ['Errors: `Type mismatch` (13).'], 'A = 1: B = 2: SWAP A, B: PRINT A; B', ' 2  1', ['LET'], V)

# ---- Text in and out ---------------------------------------------------------------
T = 'The screen and keyboard'
e('?', 'Statement', '? items', 'Short for PRINT.', [], '? 1 + 1', ' 2', ['PRINT'], T)
e('CLS', 'Statement', 'CLS [n]', 'Clears the terminal and puts the cursor at the top left.', ['`n` (0-2) is accepted for old programs.'], '', '', ['LOCATE', 'COLOR'], T)
e('COLOR', 'Statement', 'COLOR [fg][, [bg][, border]]', 'Sets the text colours, as PC colours 0-15 sent as ANSI codes: 0 black, 1 blue, 2 green, 3 cyan, 4 red, 5 magenta, 6 brown, 7 white, 8-15 bright. 7 (fg) and 0 (bg) give the terminal\'s own colours.',
  ['fg 16-31 counts as 0-15 (no blinking). `border` is ignored.', 'Errors: `Illegal function call` (5) out of range.'], 'COLOR 14, 1: PRINT "YELLOW ON BLUE": COLOR 7, 0', 'YELLOW ON BLUE', ['CLS'], T)
e('CSRLIN', 'Function', 'CSRLIN', 'The row the cursor is on, 1-30.', [], 'LOCATE 10, 1: PRINT CSRLIN', ' 10', ['POS', 'LOCATE'], T)
e('INKEY$', 'Function', 'INKEY$', 'The key waiting, or "" if none: it doesn\'t wait. Cursor and function keys give CHR$(0) and the PC scan code (see INKEY$ key codes).',
  [], 'K$ = INKEY$: IF K$ = CHR$(0) + CHR$(72) THEN PRINT "UP"', '', ['INPUT$', 'ON KEY'], T)
e('INPUT', 'Statement', 'INPUT ["prompt"{; | ,}] var[, var...]', 'Waits for a line typed at the terminal and puts its comma-separated items into the variables. A `;` after the prompt adds `? `, a `,` doesn\'t.',
  ['If the items don\'t fit (too few, or text for a number) it says `?Redo from start` and asks again.', 'Ctrl-C stops the program.'],
  'INPUT "Age"; A: PRINT A * 12; "months"', 'Age? 3\n 36 months', ['LINE INPUT', 'INPUT #'], T)
e('INPUT$', 'Function', 'INPUT$(n[, [#]f])', 'Reads `n` characters: from file `f`, or from the keyboard (waiting, without showing them).', [], 'K$ = INPUT$(1): PRINT "You pressed "; K$', 'You pressed A', ['INKEY$', 'INPUT'], T)
e('KEY', 'Statement', 'KEY n, text$ | KEY LIST | KEY ON | KEY OFF',
  'Sets what function key F1-F10 types (`CHR$(13)` in it is Enter), lists them, or shows their labels on row 25 (`KEY ON`) as on a PC.',
  ['Defaults: F1 `LIST `, F2 `RUN`+Enter, F3 `LOAD"`, F4 `SAVE"`, F5 `CONT`+Enter, F7 `TRON`, F8 `TROFF`, F9 `KEY `, F10 `SCREEN 0,0,0`.', '`KEY(n) ON/OFF/STOP` is for key events: see ON KEY.'],
  'KEY 5, "FILES" + CHR$(13)', '', ['ON KEY'], T)
e('LINE INPUT', 'Statement', 'LINE INPUT ["prompt";] var$', 'Reads a whole line typed at the terminal, commas and quotes included, into a string.', [], 'LINE INPUT "Name: "; N$', 'Name: Ada Lovelace', ['INPUT'], T)
e('LOCATE', 'Statement', 'LOCATE [row][, [col][, cursor]]', 'Moves the cursor to `row` (1-30) and `col` (1-80); `cursor` 0 hides it, 1 shows it.', ['Errors: `Illegal function call` (5) off the screen.'], 'LOCATE 12, 35: PRINT "MIDDLE"', '', ['CSRLIN', 'POS'], T)
e('LPRINT', 'Statement', 'LPRINT items | LPRINT USING fmt$; items', 'The same as PRINT: there is no printer.', [], '', '', ['PRINT'], T)
e('POS', 'Function', 'POS(n)', 'The column the cursor is in, 1-80 (`n` is ignored).', [], 'PRINT "ABC";: PRINT POS(0)', 'ABC 4', ['CSRLIN'], T)
e('PRINT', 'Statement', 'PRINT [items]', 'Prints numbers and strings. `;` between items prints them together, `,` jumps to the next 14-column zone; a `;` or `,` at the end keeps the cursor on the line.',
  ['Numbers print with a space before them (or a `-`) and one after.', '`TAB(n)` and `SPC(n)` position the text.', 'Lines wrap at 80 columns.'],
  'PRINT "A"; 1, "B"; 2', 'A 1           B 2', ['PRINT USING', 'WRITE', 'TAB'], T)
e('PRINT USING', 'Statement', 'PRINT USING fmt$; items[;]', 'Prints numbers and strings in the fields of `fmt$` (see PRINT USING formats); the format is used again from its start for more items.',
  ['Also `PRINT #n, USING`, and after other items: `PRINT "X ="; USING "##.#"; X`.', 'A number too big for its field is printed in full after a `%`.'],
  'PRINT USING "###.##"; 3.14159; -2', '  3.14 -2.00', ['PRINT'], T)
e('SPC', 'Function', 'SPC(n)', 'In PRINT, prints `n` spaces.', [], 'PRINT "A"; SPC(3); "B"', 'A   B', ['TAB', 'SPACE$'], T)
e('TAB', 'Function', 'TAB(n)', 'In PRINT, moves to column `n` (1-80); to the next line first if the cursor is already past it.',
  ['As in GW-BASIC: at column `n` already, it does nothing; past it, it goes to column `n` of the next line. So `PRINT T; TAB(4); "hello"` stays on one line for T = 5 (" 5 " fills columns 1-3) but not for T = 10 (" 10 " fills 1-4).',
   'Give TAB room for the widest number (`TAB(6)` for up to 999), or a fixed width: `PRINT USING "### "; T;: PRINT "hello"`.'], 'PRINT "A"; TAB(10); "B"', 'A        B', ['SPC', 'LOCATE'], T)
e('VIEW PRINT', 'Statement', 'VIEW PRINT [top TO bottom]', 'Makes only rows `top` to `bottom` scroll (an ANSI scroll region); `VIEW PRINT` alone undoes it.', [], 'VIEW PRINT 5 TO 20', '', ['LOCATE', 'CLS'], T)
e('WIDTH', 'Statement', 'WIDTH n', 'Accepted for old programs; the screen stays 80 columns.', [], '', '', [], T)
e('WRITE', 'Statement', 'WRITE [items]', 'Prints the items separated by commas, strings in quotes, numbers without spaces: what INPUT # reads back exactly.', [], 'WRITE "Ann", 120', '"Ann",120', ['WRITE #', 'PRINT'], T)

# ---- Functions: numbers -----------------------------------------------------------
M = 'Numbers'
e('ABS', 'Function', 'ABS(x)', 'The absolute value of `x` (`x` without its sign).', [], 'PRINT ABS(-3.5)', ' 3.5', ['SGN'], M)
e('ATN', 'Function', 'ATN(x)', 'The arctangent of `x`, in radians (-π/2 to π/2).', [], 'PRINT ATN(1) * 4', ' 3.14159265', ['TAN', 'SIN', 'COS'], M)
e('CDBL', 'Function', 'CDBL(x)', '`x` as a double-precision number.', [], 'PRINT CDBL(1) / 3', ' .333333333', ['CSNG', 'CINT'], M)
e('CINT', 'Function', 'CINT(x)', '`x` rounded to the nearest whole number (halves away from 0), -32768 to 32767.', ['Errors: `Overflow` (6) out of range.'], 'PRINT CINT(2.5); CINT(-2.5); CINT(2.4)', ' 3 -3  2', ['INT', 'FIX'], M)
e('COS', 'Function', 'COS(x)', 'The cosine of `x` radians.', [], 'PRINT COS(0)', ' 1', ['SIN', 'TAN'], M)
e('CSNG', 'Function', 'CSNG(x)', '`x` as a single-precision number (7 digits).', [], 'PRINT CSNG(1#/3)', ' .3333333', ['CDBL'], M)
e('EXP', 'Function', 'EXP(x)', 'e (2.718...) to the power `x`.', ['Errors: `Overflow` (6) if too big.'], 'PRINT EXP(1)', ' 2.71828183', ['LOG'], M)
e('FIX', 'Function', 'FIX(x)', '`x` with its fraction cut off (towards 0).', [], 'PRINT FIX(-2.7); FIX(2.7)', '-2  2', ['INT', 'CINT'], M)
e('INT', 'Function', 'INT(x)', 'The largest whole number not more than `x` (rounds down).', [], 'PRINT INT(-2.5); INT(2.5)', '-3  2', ['FIX', 'CINT'], M)
e('LOG', 'Function', 'LOG(x)', 'The natural logarithm of `x` (> 0). LOG(x) / LOG(10) is the base-10 log.', ['Errors: `Illegal function call` (5) for 0 or less.'], 'PRINT LOG(100) / LOG(10)', ' 2', ['EXP'], M)
e('RND', 'Function', 'RND | RND(x)', 'A random number from 0 to just under 1. `RND(0)` repeats the last one; a negative `x` starts a sequence from it.',
  ['RUN always starts the same sequence: RANDOMIZE for a different one.'], 'PRINT INT(RND * 6) + 1', ' 5', ['RANDOMIZE'], M)
e('SGN', 'Function', 'SGN(x)', '1 if `x` is positive, -1 if negative, 0 if 0.', [], 'PRINT SGN(-7); SGN(0); SGN(3)', '-1  0  1', ['ABS'], M)
e('SIN', 'Function', 'SIN(x)', 'The sine of `x` radians.', [], 'PRINT SIN(3.14159 / 2)', ' 1', ['COS', 'TAN', 'ATN'], M)
e('SQR', 'Function', 'SQR(x)', 'The square root of `x` (0 or more).', ['Errors: `Illegal function call` (5) for a negative `x`.'], 'PRINT SQR(2)', ' 1.41421356', [], M)
e('TAN', 'Function', 'TAN(x)', 'The tangent of `x` radians.', [], 'PRINT TAN(.7854)', ' 1.00000367', ['ATN'], M)

# ---- Functions: strings -----------------------------------------------------------
S = 'Strings'
e('ASC', 'Function', 'ASC(s$)', 'The code (0-255) of the first character of `s$`.', ['Errors: `Illegal function call` (5) for "".'], 'PRINT ASC("A")', ' 65', ['CHR$'], S)
e('CHR$', 'Function', 'CHR$(n)', 'The character with code `n` (0-255), in the IBM PC character set (see Characters).', ['Errors: `Illegal function call` (5) out of range.'], 'PRINT CHR$(65); CHR$(219); CHR$(1)', 'A█☺', ['ASC', 'STRING$'], S)
e('HEX$', 'Function', 'HEX$(n)', '`n` (-32768 to 65535) in hexadecimal; negative numbers as their 16-bit two\'s complement.', ['Errors: `Overflow` (6) out of range.'], 'PRINT HEX$(255); " "; HEX$(-1)', 'FF FFFF', ['OCT$', 'VAL'], S)
e('INSTR', 'Function', 'INSTR([start,] s$, find$)', 'Where `find$` first appears in `s$` (from position `start`, default 1); 0 if it doesn\'t.', [], 'PRINT INSTR("HELLO", "L"); INSTR(4, "HELLO", "L")', ' 3  4', ['MID$'], S)
e('LCASE$', 'Function', 'LCASE$(s$)', '`s$` in lower case.', [], 'PRINT LCASE$("Hello")', 'hello', ['UCASE$'], S)
e('LEFT$', 'Function', 'LEFT$(s$, n)', 'The first `n` characters of `s$`.', [], 'PRINT LEFT$("HELLO", 2)', 'HE', ['RIGHT$', 'MID$'], S)
e('LEN', 'Function', 'LEN(s$)', 'The number of characters in `s$` (0-255).', [], 'PRINT LEN("HELLO")', ' 5', [], S)
e('MID$', 'Function', 'MID$(s$, start[, n])', 'Up to `n` characters of `s$` from position `start` (all the rest without `n`).', ['Errors: `Illegal function call` (5) for `start` < 1.'], 'PRINT MID$("HELLO", 2, 3)', 'ELL', ['LEFT$', 'RIGHT$', 'MID$ ='], S)
e('OCT$', 'Function', 'OCT$(n)', '`n` (-32768 to 65535) in octal.', [], 'PRINT OCT$(8)', '10', ['HEX$'], S)
e('RIGHT$', 'Function', 'RIGHT$(s$, n)', 'The last `n` characters of `s$`.', [], 'PRINT RIGHT$("HELLO", 3)', 'LLO', ['LEFT$'], S)
e('SPACE$', 'Function', 'SPACE$(n)', 'A string of `n` (0-255) spaces.', [], 'PRINT "["; SPACE$(3); "]"', '[   ]', ['STRING$', 'SPC'], S)
e('STR$', 'Function', 'STR$(x)', 'The number `x` as PRINT shows it, with a space before a positive number.', [], 'PRINT "<"; STR$(42); ">"', '< 42>', ['VAL'], S)
e('STRING$', 'Function', 'STRING$(n, c | c$)', '`n` (0-255) copies of character code `c` or of the first character of `c$`.', [], 'PRINT STRING$(5, "*"); STRING$(3, 65)', '*****AAA', ['SPACE$'], S)
e('UCASE$', 'Function', 'UCASE$(s$)', '`s$` in upper case.', [], 'PRINT UCASE$("Hello")', 'HELLO', ['LCASE$'], S)
e('VAL', 'Function', 'VAL(s$)', 'The number at the start of `s$` (spaces skipped; `&H` hex and `&O` octal too); 0 if there isn\'t one.', [], 'PRINT VAL("12.5KG"); VAL("&HFF"); VAL("X")', ' 12.5  255  0', ['STR$'], S)

# ---- Files -------------------------------------------------------------------------
D = 'Data files'
e('CLOSE', 'Statement', 'CLOSE [[#]n[, [#]n...]]', 'Closes the files numbered, or all of them. Writes still in memory go to the disk.', [], 'CLOSE #1', '', ['OPEN', 'RESET'], D)
e('CVD', 'Function', 'CVD(s$)', 'The double-precision number in an 8-byte string made by MKD$ (GW-BASIC\'s format).', ['Errors: `Illegal function call` (5) for a string shorter than 8.'], '', '', ['MKD$'], D)
e('CVI', 'Function', 'CVI(s$)', 'The integer in a 2-byte string made by MKI$ (low byte first).', [], 'PRINT CVI(MKI$(1234))', ' 1234', ['MKI$'], D)
e('CVS', 'Function', 'CVS(s$)', 'The single-precision number in a 4-byte string made by MKS$.', [], 'PRINT CVS(MKS$(2.5))', ' 2.5', ['MKS$'], D)
e('DIR$', 'Function', 'DIR$(pattern$[, n])', 'The `n`-th (default 1st) file name matching the pattern, e.g. `DIR$("*.BAS", 2)`; "" after the last. For going through a folder\'s files.', [], '10 I = 1\n20 F$ = DIR$("*.BAS", I): IF F$ = "" THEN END\n30 PRINT F$: I = I + 1: GOTO 20\nRUN', 'PRIMES.BAS\nNEODEMO.BAS', ['FILES'], D)
e('EOF', 'Function', 'EOF(n)', '-1 when file `n` has nothing more to read (for a random file: the last GET was past the end; for COM1: nothing waiting), else 0.', [], 'WHILE NOT EOF(1): LINE INPUT #1, L$: PRINT L$: WEND', '', ['LOF', 'LOC'], D)
e('FIELD', 'Statement', 'FIELD #n, width AS var$[, width AS var$...]', 'Divides random file `n`\'s record buffer into string fields (at most 16, no more than the record length). GET fills them; LSET/RSET set them for PUT.',
  ['Errors: `FIELD overflow` (50).'], 'FIELD #1, 20 AS NM$, 2 AS AGE$', '', ['GET', 'PUT', 'LSET'], D)
e('FREEFILE', 'Function', 'FREEFILE', 'The lowest file number (1-4) not in use.', [], 'F = FREEFILE: OPEN "LOG.TXT" FOR APPEND AS #F', '', ['OPEN'], D)
e('GET', 'Statement', 'GET [#]n[, record]', 'Reads record `record` (from 1; default the next) of random file `n` into its buffer and FIELD variables.',
  ['Errors: `Bad record number` (63).'], 'GET #1, 3: PRINT NM$', '', ['PUT', 'FIELD'], D)
e('INPUT #', 'Statement', 'INPUT #n, var[, var...]', 'Reads items from file `n`: quoted strings, or text up to a comma or the end of the line (numbers stop at a space too).', ['Errors: `Input past end` (62).'], 'INPUT #1, N$, S', '', ['WRITE #', 'LINE INPUT #'], D)
e('LINE INPUT #', 'Statement', 'LINE INPUT #n, var$', 'Reads a whole line of file `n` into a string.', [], 'LINE INPUT #1, L$', '', ['INPUT #'], D)
e('LOC', 'Function', 'LOC(n)', 'For a random file: the last record read or written. For COM1: the bytes waiting. Otherwise the position in 128-byte blocks.', [], '', '', ['LOF', 'EOF'], D)
e('LOF', 'Function', 'LOF(n)', 'The length of file `n` in bytes (for COM1: the room left in its buffer).', [], 'OPEN "PRIMES.BAS" FOR INPUT AS #1: PRINT LOF(1): CLOSE', ' 329', ['LOC'], D)
e('LSET', 'Statement', 'LSET var$ = s$', 'Puts `s$` in a FIELD variable, left-aligned and padded with spaces (cut if too long). Use it, not LET, for field variables.', [], 'LSET NM$ = "Grace"', '', ['RSET', 'FIELD'], D)
e('MKD$', 'Function', 'MKD$(x)', 'The number `x` as an 8-byte double-precision string, for random files.', [], '', '', ['CVD'], D)
e('MKI$', 'Function', 'MKI$(n)', 'The integer `n` (-32768 to 32767) as a 2-byte string, low byte first.', ['Errors: `Overflow` (6).'], 'PRINT LEN(MKI$(5))', ' 2', ['CVI'], D)
e('MKS$', 'Function', 'MKS$(x)', 'The number `x` as a 4-byte single-precision string (GW-BASIC\'s format).', [], '', '', ['CVS'], D)
e('OPEN', 'Statement', 'OPEN file$ FOR INPUT | OUTPUT | APPEND | RANDOM AS [#]n [LEN = size] | OPEN mode$, [#]n, file$[, size] | OPEN "COM1:..." AS [#]n',
  'Opens a file as number `n` (1-4). OUTPUT creates or empties it, APPEND adds to its end, INPUT reads it, RANDOM reads and writes fixed-size records (LEN 1-1024, default 128; also the default without FOR).',
  ['The short form takes "I", "O", "A" or "R".', 'A "COM1:" name opens the serial port: see the manual\'s COM1 section.', 'Errors: `File not found` (53), `File already open` (55), `Bad file number` (52).'],
  'OPEN "SCORES.TXT" FOR OUTPUT AS #1: PRINT #1, "Ann", 120: CLOSE #1', '', ['CLOSE', 'PRINT #', 'INPUT #'], D)
e('PRINT #', 'Statement', 'PRINT #n, [USING fmt$;] items', 'Like PRINT, into file `n`. Lines end in CR LF (CR alone on COM1 unless LF was asked for), and there is no 80-column wrap.', [], 'PRINT #1, "TOTAL"; 42', '', ['WRITE #', 'PRINT'], D)
e('PUT', 'Statement', 'PUT [#]n[, record]', 'Writes random file `n`\'s buffer as record `record` (from 1; default the one after the last).',
  [], 'LSET NM$ = "Ada": PUT #1, 1', '', ['GET', 'FIELD'], D)
e('RSET', 'Statement', 'RSET var$ = s$', 'Like LSET, right-aligned.', [], 'RSET N$ = STR$(42)', '', ['LSET'], D)
e('WRITE #', 'Statement', 'WRITE #n, items', 'Writes the items to file `n` separated by commas, strings in quotes: INPUT # reads them back exactly.', [], 'WRITE #1, "Ann", 120', '', ['INPUT #'], D)

# ---- Events -------------------------------------------------------------------------
X = 'Events'
e('COM(1)', 'Statement', 'COM(1) ON | OFF | STOP', 'Turns the COM1 event (ON COM) on, off, or holds it (remembered until ON).', [], '', '', ['ON COM'], X)
e('KEY(n)', 'Statement', 'KEY(n) ON | OFF | STOP', 'Turns key `n`\'s event (ON KEY) on, off, or holds it.', [], 'KEY(1) ON', '', ['ON KEY'], X)
e('ON COM', 'Statement', 'ON COM(1) GOSUB line', 'Calls the subroutine when bytes arrive on the open COM1 (with COM(1) ON).', [], '', '', ['COM(1)', 'OPEN'], X)
e('ON KEY', 'Statement', 'ON KEY(n) GOSUB line', 'Calls the subroutine when key `n` is pressed (with KEY(n) ON): 1-10 are F1-F10, 11 Up, 12 Left, 13 Right, 14 Down.', ['`KEY 15`-`KEY 20` user keys are accepted but never trigger.'], '10 ON KEY(1) GOSUB 100: KEY(1) ON\n20 GOTO 20\n100 PRINT "F1": RETURN\nRUN', 'F1', ['KEY(n)', 'INKEY$'], X)
e('ON PLAY', 'Statement', 'ON PLAY(n) GOSUB line', 'Calls the subroutine when PLAY\'s background queue drops below `n` notes (with PLAY ON): for music that never stops.', [], '', '', ['PLAY', 'PLAY ON'], X)
e('ON TIMER', 'Statement', 'ON TIMER(s) GOSUB line', 'Calls the subroutine every `s` seconds (1-86400), with TIMER ON.', [], '10 ON TIMER(1) GOSUB 100: TIMER ON\n20 GOTO 20\n100 PRINT TIME$: RETURN\nRUN', '12:00:01\n12:00:02', ['TIMER ON'], X)
e('PLAY ON', 'Statement', 'PLAY ON | OFF | STOP', 'Turns the PLAY event (ON PLAY) on, off, or holds it.', [], '', '', ['ON PLAY'], X)
e('TIMER ON', 'Statement', 'TIMER ON | OFF | STOP', 'Turns the timer event (ON TIMER) on, off, or holds it.', [], '', '', ['ON TIMER'], X)

# ---- Sound ---------------------------------------------------------------------------
N = 'Sound'
e('BEEP', 'Statement', 'BEEP', 'A short beep: 800 Hz for a quarter of a second, on the speaker pin (GP18, the Maker Pi Pico\'s buzzer).', [], 'BEEP', '', ['SOUND', 'PLAY'], N)
e('PLAY', 'Statement', 'PLAY music$', 'Plays notes in GW-BASIC\'s music language (see the manual\'s Sound section: A-G, O, L, T, P, MF/MB...).',
  ['After `MB` the notes play in the background (up to 32 queued) while the program goes on.', '`X` substrings aren\'t supported; `=variable;` is.'],
  'PLAY "T160 O3 L8 CDEFGAB > C"', '', ['SOUND', 'PLAY(n)', 'ON PLAY'], N)
e('PLAY(n)', 'Function', 'PLAY(n)', 'How much is still waiting in the background queue (`n` is ignored): each note counts twice, as the note and the short gap after it (MN, normal style).', [], 'PLAY "MB CDE": PRINT PLAY(0)', ' 6', ['PLAY', 'ON PLAY'], N)
e('SOUND', 'Statement', 'SOUND freq, ticks', 'A tone of `freq` Hz (37-32767; 32767 is silence) for `ticks` (18.2 make a second). After PLAY "MB" it joins the background queue; ticks 0 stops the sound.',
  ['Errors: `Illegal function call` (5) out of range.'], 'SOUND 440, 18', '', ['BEEP', 'PLAY'], N)

# ---- Hardware ------------------------------------------------------------------------
H = 'Pins and hardware'
e('ADC', 'Function', 'ADC(pin)', 'Reads an analog input: 0-4095 for 0-3.3 V. GP26-29 on the Pico and Pico 2 (GP29 is VSYS / 3), GP40-46 on the Pico Plus 2.', ['Errors: `Illegal function call` (5) for a pin without one.'], 'PRINT ADC(26) * 3.3 / 4095', ' 1.65040293', ['TEMP', 'PIN'], H)
e('I2C', 'Statement', 'I2C OPEN [sda, scl[, khz]] | I2C WRITE addr, item[, item...][, NOSTOP] | I2C SCAN | I2C CLOSE',
  'The chip\'s I2C. OPEN starts it (default GP2 SDA, GP3 SCL, I2C1, 100 kHz; 10-1000), WRITE sends bytes (numbers 0-255 and strings) to device `addr` (0-127) (NOSTOP keeps the bus for a read), SCAN lists the devices that answer, CLOSE frees the pins.',
  ['SDA on an even pin, SCL the next one.', 'Errors: `Device I/O error` (57) if no device answers, `Device unavailable` (68) before OPEN.'],
  'I2C OPEN: I2C SCAN', '', ['I2CREAD', 'SPI'], H)
e('I2CREAD', 'Function', 'I2CREAD(addr[, reg])', 'Reads one byte from I2C device `addr`; with `reg`, first writes the register number.', [], 'PRINT I2CREAD(&H68, &H75)', ' 104', ['I2CREAD$', 'I2C'], H)
e('I2CREAD$', 'Function', 'I2CREAD$(addr, n[, reg])', 'Reads `n` bytes (1-255) from I2C device `addr` as a string; with `reg`, from that register.', [], 'D$ = I2CREAD$(&H68, 6, &H3B)', '', ['I2CREAD'], H)
e('NEOPIXEL', 'Statement', 'NEOPIXEL OPEN [pin[, count[, order$]]] | NEOPIXEL n, r, g, b | NEOPIXEL FILL r, g, b | NEOPIXEL SHOW | NEOPIXEL BRIGHT level | NEOPIXEL CLEAR | NEOPIXEL CLOSE',
  'WS2812B LEDs ("NeoPixels") on one pin, through a PIO state machine. OPEN sets up `count` (1-1024, default 1) LEDs on `pin` (default GP28, the Maker Pi Pico\'s RGB LED), GRB order or "RGB"; `n, r, g, b` and FILL set colours (0-255) in the buffer; SHOW sends it; BRIGHT (0-255) scales the next SHOWs; CLEAR turns them all off; CLOSE frees the pin.',
  ['Errors: `Device unavailable` (68) before OPEN, `Illegal function call` (5) for a value out of range.', 'RUN, NEW and SYSTEM close it.'],
  'NEOPIXEL OPEN: NEOPIXEL BRIGHT 40: NEOPIXEL 0, 255, 0, 0: NEOPIXEL SHOW', '', ['PIN', 'PWM'], H)
e('PIN', 'Statement / function', 'PIN pin, level | PIN(pin)', '`PIN pin, level` drives a pin high (1) or low (0), making it an output first. `PIN(pin)` reads it (0 or 1), making an unused pin an input. `LED` stands for the board\'s LED (GP25).',
  ['Pins BASIC uses (the speaker, SD card, PSRAM, open I2C/SPI/COM1/NeoPixel) are refused: `GPn is used by ...` then `Illegal function call`.', 'On the Maker Pi Pico the buttons GP20-22 read 0 when pressed (turn their pull-ups on with PINMODE).'],
  'PIN LED, 1: PRINT PIN(20)', ' 1', ['PINMODE', 'PINS', 'PWM'], H)
e('PINMODE', 'Statement', 'PINMODE pin, INPUT | OUTPUT | PULLUP | PULLDOWN', 'Sets a pin up as an input, an output, or an input with its pull-up or pull-down (or modes 0-3).', ['On the RP2350 a pull-down may not pull a pin low (its erratum E9); pull-ups are fine.'], 'PINMODE 20, PULLUP: PRINT PIN(20)', ' 1', ['PIN'], H)
e('PINS', 'Command', 'PINS', 'Lists every pin and what it is doing (free, in, out, PWM, ADC, the speaker, SD card, I2C, SPI, COM1, NeoPixel...).', [], 'PINS', 'GP0  free           GP8  free  ...', ['PIN'], H)
e('PWM', 'Statement', 'PWM pin, freq[, duty]', 'Runs PWM on a pin at `freq` (8 Hz-10 MHz) with a duty of 0-100 % (default 50); `PWM pin, 0` stops it.',
  ['Pins in pairs (GP0/1, GP2/3...) share one frequency, and GP16-31 share GP0-15\'s slices: changing one warns about its partner.'], 'PWM LED, 1000, 10', '', ['SERVO', 'PIN'], H)
e('SERVO', 'Statement', 'SERVO pin, angle', 'Drives a hobby servo to `angle` 0-180 degrees (0.5-2.5 ms pulses at 50 Hz).', [], 'SERVO 16, 90', '', ['PWM'], H)
e('SPI', 'Statement', 'SPI OPEN [sck, mosi, miso[, khz[, mode]]] | SPI WRITE item[, item...] | SPI CLOSE',
  'The chip\'s SPI. OPEN starts it (default SPI0: GP6 SCK, GP7 MOSI, GP4 MISO; 1000 kHz, 1-50000; mode 0-3), WRITE sends bytes ignoring what comes back, CLOSE frees the pins. Drive chip select yourself with PIN (GP5 is free for it).',
  ['SPI1 (GP10-15) is the SD card\'s while a card is in.', 'Errors: `Device unavailable` (68) before OPEN.'],
  'PIN 5, 1: SPI OPEN: PIN 5, 0: SPI WRITE &H9F: ID$ = SPI$(STRING$(3, 0)): PIN 5, 1', '', ['SPI$', 'I2C'], H)
e('SPI$', 'Function', 'SPI$(out$)', 'Sends the bytes of `out$` on the open SPI and returns the bytes that came back meanwhile (as many).', [], 'R$ = SPI$(STRING$(4, 0))', '', ['SPI'], H)
e('TEMP', 'Function', 'TEMP()', 'The chip\'s temperature in °C (uncalibrated: within a few degrees).', [], 'PRINT INT(TEMP() * 10) / 10', ' 27.3', ['ADC'], H)

# ---- System ----------------------------------------------------------------------------
Y = 'System'
e('BLOAD', 'Statement', 'BLOAD file$[, offset]', 'Loads a file saved by BSAVE (GW-BASIC\'s 7-byte header) into the emulated memory (DEF SEG).', [], '', '', ['BSAVE', 'DEF SEG'], Y)
e('BSAVE', 'Statement', 'BSAVE file$, offset, length', 'Saves `length` bytes of the emulated memory (DEF SEG) to a file.', [], '', '', ['BLOAD'], Y)
e('DATE$', 'Statement / function', 'DATE$ = "mm-dd-yyyy" | DATE$', 'Sets the date, or gives it as "mm-dd-yyyy". `-` or `/` and two-digit years are accepted. There is no battery clock: it starts at 01-01-1970 at power-up.', [], 'DATE$ = "10-31-2026": PRINT DATE$', '10-31-2026', ['TIME$'], Y)
e('ENVIRON', 'Statement', 'ENVIRON "name=value"', 'Sets a string ENVIRON$ gives back (there is no DOS environment).', [], 'ENVIRON "USER=KEN"', '', ['ENVIRON$'], Y)
e('ENVIRON$', 'Function', 'ENVIRON$(name$) | ENVIRON$(n)', 'The value set for `name$` with ENVIRON, or the n-th "name=value" string; "" if none.', [], 'ENVIRON "USER=KEN": PRINT ENVIRON$("USER")', 'KEN', ['ENVIRON'], Y)
e('ERDEV', 'Function', 'ERDEV | ERDEV$', 'GW-BASIC\'s device error details: always 0 and "" here.', [], '', '', ['ERR'], Y)
e('EXTERR', 'Function', 'EXTERR(n)', 'DOS extended error information: always 0 here.', [], '', '', ['ERR'], Y)
e('FRE', 'Function', 'FRE(x)', 'The memory free for programs and data, in bytes (`x` is ignored).', [], 'PRINT FRE(0)', ' 431456', [], Y)
e('PEEK', 'Function', 'PEEK(offset)', 'A byte of the emulated memory chosen by DEF SEG: the text screen (&HB800: reads spaces and colour 7, as a terminal can\'t be read) or BIOS data (DEF SEG = 0: &H449 mode 3, &H44A columns 80, &H484 rows-1 29, &H46C-&H46E timer ticks).',
  ['Errors: `Illegal function call` (5) anywhere else.'], 'DEF SEG = 0: PRINT PEEK(&H44A)', ' 80', ['POKE', 'DEF SEG'], Y)
e('POKE', 'Statement', 'POKE offset, byte', 'Writes a byte to the emulated memory (DEF SEG). The text screen and BIOS bytes accept it and change nothing here.', [], '', '', ['PEEK'], Y)
e('SCREEN(r, c)', 'Function', 'SCREEN(row, col[, 1])', 'The character at a screen position, or with 1 its colours. A terminal can\'t be read back, so it gives 32 (a space), or 7.', [], 'PRINT SCREEN(1, 1)', ' 32', ['PEEK'], Y)
e('TIME$', 'Statement / function', 'TIME$ = "hh:mm:ss" | TIME$', 'Sets the time of day, or gives it as "hh:mm:ss" (24-hour, UTC).', [], 'TIME$ = "09:30:00": PRINT TIME$', '09:30:00', ['DATE$', 'TIMER'], Y)
e('TIMER', 'Function', 'TIMER', 'Seconds since the board started, with fractions: for timing things.', [], 'T = TIMER: PAUSE 500: PRINT TIMER - T', ' .500118999', ['PAUSE', 'ON TIMER'], Y)

# ---- Operators --------------------------------------------------------------------------
O = 'Operators'
e('+ - * / ^', 'Operator', 'a + b, a - b, a * b, a / b, a ^ b', 'Add, subtract, multiply, divide, power. `+` also joins strings. `-x` negates. Calculations are in double precision.',
  ['Errors: `Division by zero` (11), `Overflow` (6), `Type mismatch` (13) mixing strings and numbers.'], 'PRINT 2 ^ 10; 7 / 2; "AB" + "CD"', ' 1024  3.5 ABCD', ['\\ and MOD'], O)
e('\\ and MOD', 'Operator', 'a \\ b, a MOD b', 'Integer division (both rounded to whole numbers first, the result cut towards 0) and its remainder.', [], 'PRINT 17 \\ 5; 17 MOD 5', ' 3  2', ['+ - * / ^'], O)
e('= <> < > <= >=', 'Operator', 'a = b, a <> b, a < b, a > b, a <= b, a >= b', 'Comparisons, of numbers or of strings (by character codes): -1 for true, 0 for false.', [], 'PRINT 3 > 2; "A" < "B"; 1 = 2', '-1 -1  0', ['IF', 'AND'], O)
e('AND', 'Operator', 'a AND b', 'Bitwise AND of two integers: true (-1) only if both are. Also masks bits: `X AND 15`.', [], 'PRINT 12 AND 10; (1 < 2) AND (2 < 3)', ' 8 -1', ['OR', 'NOT'], O)
e('EQV', 'Operator', 'a EQV b', 'Bitwise equivalence: a bit is 1 where both are the same.', [], 'PRINT -1 EQV 0', ' 0', ['XOR'], O)
e('IMP', 'Operator', 'a IMP b', 'Bitwise implication: false only where `a` is true and `b` false.', [], 'PRINT 0 IMP 0', '-1', ['EQV'], O)
e('NOT', 'Operator', 'NOT a', 'Bitwise NOT: -1 becomes 0 and 0 becomes -1 (NOT x is -x - 1).', [], 'PRINT NOT 0; NOT 5', '-1 -6', ['AND'], O)
e('OR', 'Operator', 'a OR b', 'Bitwise OR: true if either is.', [], 'PRINT 12 OR 3', ' 15', ['AND', 'XOR'], O)
e('XOR', 'Operator', 'a XOR b', 'Bitwise exclusive OR: true if just one is.', [], 'PRINT 12 XOR 10', ' 6', ['OR'], O)

# ---- Writing it ---------------------------------------------------------------------------

def slug(s):
    return toc.slugify(s, '-')

def entry_md(x, anchors):
    out = ['### %s {#%s}' % (x['name'], x['anchor']), '',
           '*%s* · `%s`' % (x['kind'], x['syntax']), '', x['text'], '']
    for d in x['details']:
        out.append('- ' + d)
    if x['details']:
        out.append('')
    if x['example']:
        out += ['Example:', '', '```', x['example'], '```', '']
        if x['output']:
            out += ['Shows:', '', '```', x['output'], '```', '']
    if x['see']:
        links = []
        for s in x['see']:
            a = anchors.get(s)
            links.append('[%s](#%s)' % (s, a) if a else s)
        out += ['See also: ' + ', '.join(links), '']
    return out

def main():
    entries = sorted(E, key=lambda x: (x['name'].lstrip('?').upper() or '?'))
    # Anchors as the toc extension makes them (with _1, _2 for repeats).
    anchors, used = {}, {}
    for x in entries:
        # Ids of their own (the manual's headings make toc ids too).
        s = 'ref-' + (slug(x['name']) or 'op')
        if s in used:
            used[s] += 1
            s = '%s-%d' % (s, used[s])
        else:
            used[s] = 0
        x['anchor'] = s
        anchors[x['name']] = s
    md = ['# Pico BASIC Command Reference', '', '## 12. Command reference', '',
          'Every statement, command, function and operator, A to Z. Each entry gives its form '
          '(`[...]` is optional, `|` separates choices), what it does, its limits and errors, an '
          'example and what the terminal then shows. Numbers print with a space before them (or '
          'a `-`) and one after.', '',
          '### Index by topic', '']
    topics = []
    for x in E:
        if x['topic'] not in topics:
            topics.append(x['topic'])
    for t in topics:
        names = sorted((x for x in entries if x['topic'] == t), key=lambda x: x['name'].upper())
        md.append('**%s:** %s' % (t, ', '.join('[%s](#%s)' % (x['name'], x['anchor']) for x in names)))
        md.append('')
    for x in entries:
        md += entry_md(x, anchors)
    md += APPENDICES
    path = os.path.join(ROOT, 'docs', 'BASIC_REFERENCE.md')
    open(path, 'w', encoding='utf-8').write('\n'.join(md) + '\n')
    print('wrote', path, 'with', len(entries), 'entries')
    return entries

APPENDICES = '''## Appendix A: error codes

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
| `\\  \\` | the first n + 2 characters of a string |
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
'''.split('\n')

NOT_HERE = {'CIRCLE', 'DRAW', 'FONT', 'GPRINT', 'IMAGE', 'SAVEIMAGE', 'MOUSE', 'MUSIC', 'PAINT',
            'PALETTE', 'PCOPY', 'PRESET', 'PSET', 'SHELL', 'SPRITE', 'WINDOW', 'POINT', 'PMAP',
            'MOUSEX', 'MOUSEY', 'MOUSEB', 'MOUSEW', 'KEYDOWN', 'SPRITEHIT', 'SPRITEX', 'SPRITEY',
            'SPRITEON', 'FONTW', 'FONTH', 'GWIDTH', 'RGB', 'LOCK', 'UNLOCK', 'MOTOR', 'LPOS', 'STICK',
            'STRIG', 'PEN', 'CALL', 'USR', 'VARPTR', 'INP', 'OUT', 'WAIT', 'IOCTL'}

def coverage(entries):
    src = open(os.path.join(ROOT, 'src', 'basic.c'), encoding='utf-8').read()
    words = set()
    for table in ('builtin_words', 'statement_words'):
        m = re.search(r'static const char \*const %s\[\] = \{(.*?)\};' % table, src, re.S)
        words |= set(re.findall(r'"([A-Z$]+)"', m.group(1)))
    m = re.search(r'static const char \*const names\[\] = \{(.*?)\};', src, re.S)
    words |= {w.rstrip('(').rstrip('$') for w in re.findall(r'"([A-Z$]+)\(?"', m.group(1))}
    words.discard('R')  # LOAD's ,R
    # Left out on purpose: graphics, sprites, fonts, pictures, music, the
    # mouse and PC hardware, which a terminal build can't have (they give
    # Feature not available, a dummy value, or Illegal function call).
    words -= NOT_HERE
    covered = set()
    for x in entries:
        for part in re.split(r'[\s|,()=;\[\]]+', x['name'] + ' ' + x['syntax']):
            covered.add(part.rstrip('$').upper())
    missing = sorted(w for w in words if w.rstrip('$') not in covered)
    print('keywords in basic.c:', len(words), '- without an entry:', ', '.join(missing) or 'none')
    return missing

if __name__ == '__main__':
    ents = main()
    sys.exit(1 if coverage(ents) else 0)
