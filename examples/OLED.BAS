' OLED.BAS: tries out a 128x64 SSD1309 (or SSD1306) OLED on I2C.
' Wire SDA to GP2, SCL to GP3 (I2C1), VCC to 3.3 V and GND. The
' display is at &H3C (some are &H3D: change A). Shows text, a
' checkerboard, a sine wave, contrast, invert and a live counter.
A = &H3C
I2C OPEN 2, 3, 400
ON ERROR GOTO nodisplay
GOSUB startoled
ON ERROR GOTO 0
GOSUB loadfont
GOSUB clearoled

' Text: 21 characters by 8 rows
R = 0: C = 0: T$ = "PICO BASIC": GOSUB prt
R = 2: T$ = "SSD1309 128X64 OLED": GOSUB prt
R = 4: T$ = "I2C1 GP2/GP3 AT &H" + HEX$(A): GOSUB prt
R = 5: T$ = "ABCDEFGHIJKLMNOPQRSTU": GOSUB prt
R = 6: T$ = "VWXYZ 0123456789": GOSUB prt
R = 7: T$ = "!#$%&()*+,-./:;<=>?@": GOSUB prt
PAUSE 3000

' Checkerboard of 8x8 squares, then the same inverted
B$ = "": FOR I = 1 TO 8: B$ = B$ + STRING$(8, 255) + STRING$(8, 0): NEXT
W$ = MID$(B$, 9) + STRING$(8, 255)
GOSUB wholescreen
FOR P = 0 TO 7
  IF P MOD 2 THEN I2C WRITE A, &H40, W$ ELSE I2C WRITE A, &H40, B$
NEXT
PAUSE 1500: I2C WRITE A, 0, &HA7: PAUSE 1500: I2C WRITE A, 0, &HA6

' Sine wave and its axis, built a page (8 rows) at a time
DIM Y(127): FOR X = 0 TO 127: Y(X) = 32 - INT(28 * SIN(X * 3.14159 / 32)): NEXT
GOSUB wholescreen
FOR P = 0 TO 7
  S$ = ""
  FOR X = 0 TO 127
    V = 0: IF Y(X) \ 8 = P THEN V = BT(Y(X) MOD 8)
    IF P = 4 AND X MOD 2 = 0 THEN V = V OR 1
    S$ = S$ + CHR$(V)
  NEXT
  I2C WRITE A, &H40, S$
NEXT
PAUSE 1000

' Contrast down and back up
FOR V = 255 TO 0 STEP -15: I2C WRITE A, 0, &H81, V: PAUSE 40: NEXT
FOR V = 0 TO 255 STEP 15: I2C WRITE A, 0, &H81, V: PAUSE 40: NEXT
I2C WRITE A, 0, &H81, &HCF: PAUSE 1000

' A box with a counter that updates in place
GOSUB clearoled
GOSUB wholescreen
I2C WRITE A, &H40, CHR$(255) + STRING$(126, 1) + CHR$(255)
FOR P = 1 TO 6: I2C WRITE A, &H40, CHR$(255) + STRING$(126, 0) + CHR$(255): NEXT
I2C WRITE A, &H40, CHR$(255) + STRING$(126, 128) + CHR$(255)
R = 2: C = 34: T$ = "PICO BASIC": GOSUB prt
R = 3: C = 10: T$ = "SAYS HELLO TO YOU": GOSUB prt
T0 = TIMER
FOR N = 0 TO 100
  R = 5: C = 40: T$ = "N = " + RIGHT$("  " + MID$(STR$(N), 2), 3): GOSUB prt
NEXT
T$ = STR$(INT((TIMER - T0) * 10) / 10) + " S": R = 6: C = 64 - 3 * LEN(T$): GOSUB prt
PRINT "Done: 101 counter updates took"; T$
I2C CLOSE
END

nodisplay:
  PRINT "No display answered at &H"; HEX$(A); " (check the wiring, or try &H3D)"
  I2C CLOSE
  END

' Starts the display: off, clock, 64 rows, no offset, start line 0,
' flipped both ways (pins at the top), COM pins, contrast, precharge,
' VCOMH, horizontal addressing, show RAM, not inverted, then on. For
' an SSD1306 put &H8D, &H14 (its charge pump) before the &HAF.
startoled:
  I2C WRITE A, 0, &HAE, &HD5, &HA0, &HA8, &H3F, &HD3, 0, &H40, &HA1, &HC8
  I2C WRITE A, 0, &HDA, &H12, &H81, &HCF, &HD9, &HF1, &HDB, &H40, &H20, 0
  I2C WRITE A, 0, &HA4, &HA6, &HAF
  RETURN

' Loads the 5x7 font (space to Z) as 6-byte column strings G$(), and
' the bit values BT()
loadfont:
  DIM G$(58), BT(7): H$ = "0123456789ABCDEF"
  FOR I = 0 TO 7: BT(I) = INT(2 ^ I + .5): NEXT
  RESTORE glyphs
  FOR I = 0 TO 58
    READ D$: S$ = ""
    FOR J = 1 TO 9 STEP 2
      S$ = S$ + CHR$((INSTR(H$, MID$(D$, J, 1)) - 1) * 16 + INSTR(H$, MID$(D$, J + 1, 1)) - 1)
    NEXT
    G$(I) = S$ + CHR$(0)
  NEXT
  RETURN

' Clears the whole display
clearoled:
  GOSUB wholescreen
  Z$ = STRING$(128, 0)
  FOR P = 0 TO 7: I2C WRITE A, &H40, Z$: NEXT
  RETURN

' Sets the write window to the whole display (columns 0-127, pages 0-7)
wholescreen:
  I2C WRITE A, 0, &H21, 0, 127, &H22, 0, 7
  RETURN

' Prints T$ on text row R (0-7) from pixel column C (0-127), 6 pixels a
' character; lower case is shown as capitals
prt:
  S$ = ""
  FOR I = 1 TO LEN(T$)
    K = ASC(MID$(T$, I, 1)): IF K > 96 AND K < 123 THEN K = K - 32
    IF K < 32 OR K > 90 THEN K = 63
    S$ = S$ + G$(K - 32)
  NEXT
  I2C WRITE A, 0, &H21, C, 127, &H22, R, R
  I2C WRITE A, &H40, LEFT$(S$, 128 - C)
  RETURN

' Font: 5 columns a character, bit 0 at the top
glyphs:
  DATA "0000000000","00005F0000","0007000700","147F147F14","242A7F2A12"
  DATA "2313086462","3649552250","0005030000","001C224100","0041221C00"
  DATA "082A1C2A08","08083E0808","0050300000","0808080808","0060600000"
  DATA "2010080402","3E5149453E","00427F4000","4261514946","2141454B31"
  DATA "1814127F10","2745454539","3C4A494930","0171090503","3649494936"
  DATA "064949291E","0036360000","0056360000","0814224100","1414141414"
  DATA "0041221408","0201510906","324979413E","7E1111117E","7F49494936"
  DATA "3E41414122","7F4141221C","7F49494941","7F09090101","3E41415132"
  DATA "7F0808087F","00417F4100","2040413F01","7F08142241","7F40404040"
  DATA "7F0204027F","7F0408107F","3E4141413E","7F09090906","3E4151215E"
  DATA "7F09192946","4649494931","01017F0101","3F4040403F","1F2040201F"
  DATA "7F2018207F","6314081463","0304780403","6151494543"
