' ANALOG: read the ADC inputs and the chip temperature
' Pico 2: GP26-29 (GP29 is VSYS / 3). Pico Plus 2: change to 40 and 46.
FIRST = 26: LAST = 29
CLS: PRINT "ADC inputs, 0-4095 = 0-3.3 V. Any key stops."
readings:
  LOCATE 3, 1
  FOR P = FIRST TO LAST
    V = ADC(P)
    PRINT "GP"; P; ":"; V; TAB(20); INT(V * 3300 / 4095) / 1000; "V   "
  NEXT P
  PRINT: PRINT "Chip temperature:"; INT(TEMP() * 10) / 10; "C   "
  PAUSE 500
  IF INKEY$ = "" THEN readings
