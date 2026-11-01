' OLEDLIB.BAS: a library for a 128x64 SSD1309 (or SSD1306) OLED on I2C,
' for INCLUDE "OLEDLIB" in a program. SDA on GP2, SCL on GP3 (I2C1).
'
'   GOSUB oledstart    opens I2C, starts the display, clears it. The
'                      address is OLED.ADDR (&H3C if not set). No display:
'                      Device I/O error (57), which ON ERROR can catch
'   GOSUB oledclear    clears the display
'   GOSUB oledtext     prints TX$ on text row ROW (0-7) from pixel column
'                      COL (0-127): 21 characters of 6 pixels a row, upper
'                      case (lower case is shown as capitals)
'   GOSUB oledwhole    sets the write window to the whole display, for
'                      I2C WRITE OLED.ADDR, &H40, bytes: 8 pages of 128
'                      bytes, each byte a column of 8 pixels, bit 0 on top
'   GOSUB oledcontrast contrast OLED.V (0-255)
'   GOSUB oledinvert   inverted if OLED.V is not 0
'   GOSUB oledstop     turns the display off and closes I2C
'
' Its own variables start with OL. so they don't meet the program's.

oledstart:
  IF OLED.ADDR = 0 THEN OLED.ADDR = &H3C
  I2C OPEN 2, 3, 400
  ' Off, clock, 64 rows, no offset, start line 0, flipped both ways (pins
  ' at the top), COM pins, contrast, precharge, VCOMH, horizontal
  ' addressing, show RAM, not inverted, then on. For an SSD1306 put
  ' &H8D, &H14 (its charge pump) before the &HAF.
  I2C WRITE OLED.ADDR, 0, &HAE, &HD5, &HA0, &HA8, &H3F, &HD3, 0, &H40, &HA1, &HC8
  I2C WRITE OLED.ADDR, 0, &HDA, &H12, &H81, &HCF, &HD9, &HF1, &HDB, &H40, &H20, 0
  I2C WRITE OLED.ADDR, 0, &HA4, &HA6, &HAF
  IF OL.READY THEN oledclear
  ' The 5x7 font (space to Z) as 6-byte column strings
  DIM OL.G$(58): OL.H$ = "0123456789ABCDEF"
  RESTORE oledglyphs
  FOR OL.I = 0 TO 58
    READ OL.D$: OL.S$ = ""
    FOR OL.J = 1 TO 9 STEP 2
      OL.S$ = OL.S$ + CHR$((INSTR(OL.H$, MID$(OL.D$, OL.J, 1)) - 1) * 16 + INSTR(OL.H$, MID$(OL.D$, OL.J + 1, 1)) - 1)
    NEXT
    OL.G$(OL.I) = OL.S$ + CHR$(0)
  NEXT
  OL.READY = 1

oledclear:
  GOSUB oledwhole
  OL.S$ = STRING$(128, 0)
  FOR OL.I = 0 TO 7: I2C WRITE OLED.ADDR, &H40, OL.S$: NEXT
  RETURN

oledwhole:
  I2C WRITE OLED.ADDR, 0, &H21, 0, 127, &H22, 0, 7
  RETURN

oledtext:
  OL.S$ = ""
  FOR OL.I = 1 TO LEN(TX$)
    OL.K = ASC(MID$(TX$, OL.I, 1)): IF OL.K > 96 AND OL.K < 123 THEN OL.K = OL.K - 32
    IF OL.K < 32 OR OL.K > 90 THEN OL.K = 63
    OL.S$ = OL.S$ + OL.G$(OL.K - 32)
  NEXT
  I2C WRITE OLED.ADDR, 0, &H21, COL, 127, &H22, ROW, ROW
  I2C WRITE OLED.ADDR, &H40, LEFT$(OL.S$, 128 - COL)
  RETURN

oledcontrast:
  I2C WRITE OLED.ADDR, 0, &H81, OLED.V
  RETURN

oledinvert:
  IF OLED.V THEN I2C WRITE OLED.ADDR, 0, &HA7 ELSE I2C WRITE OLED.ADDR, 0, &HA6
  RETURN

oledstop:
  I2C WRITE OLED.ADDR, 0, &HAE
  I2C CLOSE
  RETURN

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
