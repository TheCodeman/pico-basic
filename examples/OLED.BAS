' OLED.BAS: tries out a 128x64 SSD1309 (or SSD1306) OLED on I2C, with
' the OLEDLIB.BAS library (put both on the disk). Wire SDA to GP2, SCL
' to GP3 (I2C1), VCC to 3.3 V and GND. The display is at &H3C (some are
' &H3D: change ADDR). Shows text, a checkerboard, a sine wave, contrast,
' invert and a live counter.
INCLUDE "OLEDLIB"
ADDR = &H3C
ON ERROR GOTO nodisplay
OLEDSTART ADDR
ON ERROR GOTO 0

' Text: 21 characters by 8 rows
OLEDTEXT 0, 0, "PICO BASIC"
OLEDTEXT 2, 0, "SSD1309 128X64 OLED"
OLEDTEXT 4, 0, "I2C1 GP2/GP3 AT &H" + HEX$(ADDR)
OLEDTEXT 5, 0, "ABCDEFGHIJKLMNOPQRSTU"
OLEDTEXT 6, 0, "VWXYZ 0123456789"
OLEDTEXT 7, 0, "!#$%&()*+,-./:;<=>?@"
PAUSE 3000

' Checkerboard of 8x8 squares, then the same inverted
B$ = "": FOR I = 1 TO 8: B$ = B$ + STRING$(8, 255) + STRING$(8, 0): NEXT
W$ = MID$(B$, 9) + STRING$(8, 255)
OLEDWHOLE
FOR P = 0 TO 7
  IF P MOD 2 THEN OLEDDATA W$ ELSE OLEDDATA B$
NEXT
PAUSE 1500: OLEDINVERT 1: PAUSE 1500: OLEDINVERT 0

' Sine wave and its axis, built a page (8 rows) at a time
DIM Y(127), BIT(7)
FOR I = 0 TO 7: BIT(I) = INT(2 ^ I + .5): NEXT
FOR X = 0 TO 127: Y(X) = 32 - INT(28 * SIN(X * 3.14159 / 32)): NEXT
OLEDWHOLE
FOR P = 0 TO 7
  S$ = ""
  FOR X = 0 TO 127
    V = 0: IF Y(X) \ 8 = P THEN V = BIT(Y(X) MOD 8)
    IF P = 4 AND X MOD 2 = 0 THEN V = V OR 1
    S$ = S$ + CHR$(V)
  NEXT
  OLEDDATA S$
NEXT
PAUSE 1000

' Contrast down and back up
FOR V = 255 TO 0 STEP -15: OLEDCONTRAST V: PAUSE 40: NEXT
FOR V = 0 TO 255 STEP 15: OLEDCONTRAST V: PAUSE 40: NEXT
OLEDCONTRAST &HCF: PAUSE 1000

' A box with a counter that updates in place
OLEDCLEAR
OLEDWHOLE
OLEDDATA CHR$(255) + STRING$(126, 1) + CHR$(255)
FOR P = 1 TO 6: OLEDDATA CHR$(255) + STRING$(126, 0) + CHR$(255): NEXT
OLEDDATA CHR$(255) + STRING$(126, 128) + CHR$(255)
OLEDTEXT 2, OLEDCENTER("PICO BASIC"), "PICO BASIC"
OLEDTEXT 3, OLEDCENTER("SAYS HELLO TO YOU"), "SAYS HELLO TO YOU"
T0 = TIMER
FOR N = 0 TO 100
  OLEDTEXT 5, 40, "N = " + RIGHT$("  " + MID$(STR$(N), 2), 3)
NEXT
T$ = STR$(INT((TIMER - T0) * 10) / 10) + " S"
OLEDTEXT 6, OLEDCENTER(T$), T$
PRINT "Done: 101 counter updates took"; T$
OLEDSTOP
END

nodisplay:
  PRINT "No display answered at &H"; HEX$(ADDR); " (check the wiring, or try &H3D)"
  I2C CLOSE
  END
