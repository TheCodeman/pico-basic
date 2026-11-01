' I2CSCAN.BAS: lists the I2C devices that answer, then reads a few
' bytes from the first one. Wire SDA to GP2 and SCL to GP3 (I2C1),
' with 3.3 V and GND; most breakout boards have their own pull-ups.
I2C OPEN 2, 3, 100
PRINT "Devices: ";: I2C SCAN
FOR A = 8 TO 119
  ON ERROR GOTO noanswer
  B = I2CREAD(A)
  ON ERROR GOTO 0
  PRINT "First at &H"; HEX$(A); ": registers 0-7 read";
  R$ = I2CREAD$(A, 8, 0)
  FOR I = 1 TO 8: PRINT " "; HEX$(ASC(MID$(R$, I, 1)));: NEXT: PRINT
  GOTO finish
noanswer: RESUME nextaddr
nextaddr: NEXT A
PRINT "No device to read."
finish: ON ERROR GOTO 0: I2C CLOSE
