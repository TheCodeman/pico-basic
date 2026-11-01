' SERIAL: a tiny terminal on COM1 (GP8 = TX, GP9 = RX, 3.3 V levels only)
' What you type is sent; what arrives is shown. Esc quits.
OPEN "COM1:9600,N,8,1" AS #1
PRINT "Terminal on COM1, 9600 baud 8N1 (TX GP8, RX GP9). Esc quits."
K$ = ""
WHILE K$ <> CHR$(27)
  IF K$ <> "" THEN PRINT #1, K$;
  IF LOC(1) > 0 THEN PRINT INPUT$(LOC(1), #1);
  K$ = INKEY$
WEND
CLOSE #1: PRINT: PRINT "Closed."
