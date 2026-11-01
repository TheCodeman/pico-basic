' SPIFLASH.BAS: reads the JEDEC ID (maker, type, size) and the first
' 16 bytes of an SPI flash chip (W25Q16 and the like) on SPI0:
' SCK GP6, MOSI (DI) GP7, MISO (DO) GP4, CS GP5, 3.3 V and GND.
CS = 5: PIN CS, 1
SPI OPEN 6, 7, 4, 1000, 0
PIN CS, 0: SPI WRITE &H9F: ID$ = SPI$(STRING$(3, 0)): PIN CS, 1
PRINT "JEDEC ID:";
FOR I = 1 TO 3: PRINT " &H"; HEX$(ASC(MID$(ID$, I, 1)));: NEXT: PRINT
IF ID$ = STRING$(3, 255) OR ID$ = STRING$(3, 0) THEN PRINT "No chip answered.": GOTO finish
IF ASC(ID$) = &HEF THEN PRINT "Winbond,"; 2 ^ ASC(MID$(ID$, 3)) \ 1024; "KB"
PIN CS, 0: SPI WRITE 3, 0, 0, 0: D$ = SPI$(STRING$(16, 0)): PIN CS, 1
PRINT "Bytes 0-15:";
FOR I = 1 TO 16: PRINT " "; RIGHT$("0" + HEX$(ASC(MID$(D$, I, 1))), 2);: NEXT: PRINT
finish: SPI CLOSE
