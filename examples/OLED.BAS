' OLED.BAS: tries out a 128x64 SSD1309 (or SSD1306) OLED on I2C, with
' the OLEDLIB.BAS library (put both on the disk). Wire SDA to GP2, SCL
' to GP3 (I2C1), VCC to 3.3 V and GND. The display is at &H3C (some are
' &H3D: change OLED.ADDR). Shows text, a checkerboard, a sine wave,
' contrast, invert and a live counter.
INCLUDE "OLEDLIB"
OLED.ADDR = &H3C
ON ERROR GOTO nodisplay
GOSUB oledstart
ON ERROR GOTO 0

' Text: 21 characters by 8 rows
ROW = 0: COL = 0: TX$ = "PICO BASIC": GOSUB oledtext
ROW = 2: TX$ = "SSD1309 128X64 OLED": GOSUB oledtext
ROW = 4: TX$ = "I2C1 GP2/GP3 AT &H" + HEX$(OLED.ADDR): GOSUB oledtext
ROW = 5: TX$ = "ABCDEFGHIJKLMNOPQRSTU": GOSUB oledtext
ROW = 6: TX$ = "VWXYZ 0123456789": GOSUB oledtext
ROW = 7: TX$ = "!#$%&()*+,-./:;<=>?@": GOSUB oledtext
PAUSE 3000

' Checkerboard of 8x8 squares, then the same inverted
B$ = "": FOR I = 1 TO 8: B$ = B$ + STRING$(8, 255) + STRING$(8, 0): NEXT
W$ = MID$(B$, 9) + STRING$(8, 255)
GOSUB oledwhole
FOR P = 0 TO 7
  IF P MOD 2 THEN I2C WRITE OLED.ADDR, &H40, W$ ELSE I2C WRITE OLED.ADDR, &H40, B$
NEXT
PAUSE 1500: OLED.V = 1: GOSUB oledinvert
PAUSE 1500: OLED.V = 0: GOSUB oledinvert

' Sine wave and its axis, built a page (8 rows) at a time
DIM Y(127), BIT(7)
FOR I = 0 TO 7: BIT(I) = INT(2 ^ I + .5): NEXT
FOR X = 0 TO 127: Y(X) = 32 - INT(28 * SIN(X * 3.14159 / 32)): NEXT
GOSUB oledwhole
FOR P = 0 TO 7
  S$ = ""
  FOR X = 0 TO 127
    V = 0: IF Y(X) \ 8 = P THEN V = BIT(Y(X) MOD 8)
    IF P = 4 AND X MOD 2 = 0 THEN V = V OR 1
    S$ = S$ + CHR$(V)
  NEXT
  I2C WRITE OLED.ADDR, &H40, S$
NEXT
PAUSE 1000

' Contrast down and back up
FOR OLED.V = 255 TO 0 STEP -15: GOSUB oledcontrast: PAUSE 40: NEXT
FOR OLED.V = 0 TO 255 STEP 15: GOSUB oledcontrast: PAUSE 40: NEXT
OLED.V = &HCF: GOSUB oledcontrast: PAUSE 1000

' A box with a counter that updates in place
GOSUB oledclear
GOSUB oledwhole
I2C WRITE OLED.ADDR, &H40, CHR$(255) + STRING$(126, 1) + CHR$(255)
FOR P = 1 TO 6: I2C WRITE OLED.ADDR, &H40, CHR$(255) + STRING$(126, 0) + CHR$(255): NEXT
I2C WRITE OLED.ADDR, &H40, CHR$(255) + STRING$(126, 128) + CHR$(255)
ROW = 2: COL = 34: TX$ = "PICO BASIC": GOSUB oledtext
ROW = 3: COL = 10: TX$ = "SAYS HELLO TO YOU": GOSUB oledtext
T0 = TIMER
FOR N = 0 TO 100
  ROW = 5: COL = 40: TX$ = "N = " + RIGHT$("  " + MID$(STR$(N), 2), 3): GOSUB oledtext
NEXT
TX$ = STR$(INT((TIMER - T0) * 10) / 10) + " S": ROW = 6: COL = 64 - 3 * LEN(TX$): GOSUB oledtext
PRINT "Done: 101 counter updates took"; TX$
I2C CLOSE
END

nodisplay:
  PRINT "No display answered at &H"; HEX$(OLED.ADDR); " (check the wiring, or try &H3D)"
  I2C CLOSE
  END
