10 REM SPIFLASH.BAS: reads the JEDEC ID (maker, type, size) and the first
20 REM 16 bytes of an SPI flash chip (W25Q16 and the like) on SPI0:
30 REM SCK GP18, MOSI (DI) GP19, MISO (DO) GP16, CS GP17, 3.3 V and GND.
40 CS = 17: PIN CS, 1
50 SPI OPEN 18, 19, 16, 1000, 0
60 PIN CS, 0: SPI WRITE &H9F: ID$ = SPI$(STRING$(3, 0)): PIN CS, 1
70 PRINT "JEDEC ID:";
80 FOR I = 1 TO 3: PRINT " &H"; HEX$(ASC(MID$(ID$, I, 1)));: NEXT: PRINT
90 IF ID$ = STRING$(3, 255) OR ID$ = STRING$(3, 0) THEN PRINT "No chip answered.": GOTO 150
100 IF ASC(ID$) = &HEF THEN PRINT "Winbond,"; 2 ^ ASC(MID$(ID$, 3)) \ 1024; "KB"
110 PIN CS, 0: SPI WRITE 3, 0, 0, 0: D$ = SPI$(STRING$(16, 0)): PIN CS, 1
120 PRINT "Bytes 0-15:";
130 FOR I = 1 TO 16: PRINT " "; RIGHT$("0" + HEX$(ASC(MID$(D$, I, 1))), 2);: NEXT: PRINT
150 SPI CLOSE
