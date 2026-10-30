10 REM ANALOG: read the ADC inputs and the chip temperature
15 REM Pico 2: GP26-29 (GP29 is VSYS / 3). Pico Plus 2: change to 40 and 46.
16 FIRST = 26: LAST = 29
20 CLS: PRINT "ADC inputs, 0-4095 = 0-3.3 V. Any key stops."
30 LOCATE 3, 1
40 FOR P = FIRST TO LAST
50   V = ADC(P)
60   PRINT "GP"; P; ":"; V; TAB(20); INT(V * 3300 / 4095) / 1000; "V   "
70 NEXT P
80 PRINT: PRINT "Chip temperature:"; INT(TEMP() * 10) / 10; "C   "
90 PAUSE 500
100 IF INKEY$ = "" THEN 30
