' OLEDLIB.BAS: a library for a 128x64 SSD1309 (or SSD1306) OLED on I2C,
' for INCLUDE "OLEDLIB" in a program. SDA on GP2, SCL on GP3 (I2C1).
'
'   OLEDSTART addr          opens I2C, starts the display (at addr, or &H3C
'                           if 0) and clears it. No display: Device I/O
'                           error (57), which ON ERROR can catch
'   OLEDCLEAR               clears the display
'   OLEDTEXT row, col, t$   prints t$ on text row 0-7 from pixel column
'                           0-127: 21 characters of 6 pixels a row, upper
'                           case (lower case is shown as capitals)
'   OLEDCENTER(t$)          the column that centres t$ (a FUNCTION)
'   OLEDWHOLE               sets the write window to the whole display;
'   OLEDDATA bytes$         then sends display bytes: 8 pages of 128, each
'                           byte a column of 8 pixels, bit 0 on top
'   OLEDCONTRAST v          contrast 0-255
'   OLEDINVERT on           inverted if on is not 0
'   OLEDSTOP                turns the display off and closes I2C
'
' The variables it shares between its SUBs start with OL.

SUB OLEDSTART(A)
  SHARED OL.ADDR, OL.READY
  OL.ADDR = A: IF A = 0 THEN OL.ADDR = &H3C
  I2C OPEN 2, 3, 400
  ' Off, clock, 64 rows, no offset, start line 0, flipped both ways (pins
  ' at the top), COM pins, contrast, precharge, VCOMH, horizontal
  ' addressing, show RAM, not inverted, then on. For an SSD1306 put
  ' &H8D, &H14 (its charge pump) before the &HAF.
  I2C WRITE OL.ADDR, 0, &HAE, &HD5, &HA0, &HA8, &H3F, &HD3, 0, &H40, &HA1, &HC8
  I2C WRITE OL.ADDR, 0, &HDA, &H12, &H81, &HCF, &HD9, &HF1, &HDB, &H40, &H20, 0
  I2C WRITE OL.ADDR, 0, &HA4, &HA6, &HAF
  IF OL.READY = 0 THEN OLEDFONT: OL.READY = 1
  OLEDCLEAR
END SUB

' Loads the 5x7 font (space to Z) as 6-byte column strings
SUB OLEDFONT
  SHARED OL.G$()
  DIM OL.G$(58): H$ = "0123456789ABCDEF"
  RESTORE oledglyphs
  FOR I = 0 TO 58
    READ D$: S$ = ""
    FOR J = 1 TO 9 STEP 2
      S$ = S$ + CHR$((INSTR(H$, MID$(D$, J, 1)) - 1) * 16 + INSTR(H$, MID$(D$, J + 1, 1)) - 1)
    NEXT
    OL.G$(I) = S$ + CHR$(0)
  NEXT
END SUB

SUB OLEDCLEAR
  SHARED OL.ADDR
  OLEDWHOLE
  Z$ = STRING$(128, 0)
  FOR I = 0 TO 7: I2C WRITE OL.ADDR, &H40, Z$: NEXT
END SUB

SUB OLEDWHOLE
  SHARED OL.ADDR
  I2C WRITE OL.ADDR, 0, &H21, 0, 127, &H22, 0, 7
END SUB

SUB OLEDDATA(B$)
  SHARED OL.ADDR
  I2C WRITE OL.ADDR, &H40, B$
END SUB

SUB OLEDTEXT(R, C, T$)
  SHARED OL.ADDR, OL.G$()
  S$ = ""
  FOR I = 1 TO LEN(T$)
    K = ASC(MID$(T$, I, 1)): IF K > 96 AND K < 123 THEN K = K - 32
    IF K < 32 OR K > 90 THEN K = 63
    S$ = S$ + OL.G$(K - 32)
  NEXT
  I2C WRITE OL.ADDR, 0, &H21, C, 127, &H22, R, R
  I2C WRITE OL.ADDR, &H40, LEFT$(S$, 128 - C)
END SUB

FUNCTION OLEDCENTER(T$)
  OLEDCENTER = (128 - 6 * LEN(T$)) \ 2
END FUNCTION

SUB OLEDCONTRAST(V)
  SHARED OL.ADDR
  I2C WRITE OL.ADDR, 0, &H81, V
END SUB

SUB OLEDINVERT(V)
  SHARED OL.ADDR
  IF V THEN I2C WRITE OL.ADDR, 0, &HA7 ELSE I2C WRITE OL.ADDR, 0, &HA6
END SUB

SUB OLEDSTOP
  SHARED OL.ADDR
  I2C WRITE OL.ADDR, 0, &HAE
  I2C CLOSE
END SUB

' Font: 5 columns a character, bit 0 at the top
oledglyphs:
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
