10 REM NEODEMO.BAS: eight light effects on WS2812B LEDs (NeoPixels).
20 REM The Maker Pi Pico's RGB LED (GP28) by default. For a strip, set PN to
30 REM its pin and N to its length in line 60 (5 V strips: their own power,
40 REM GND joined to the Pico's). BT = 0 if there are no buttons on GP20-22.
50 REM Keys: Space or N next, P previous, + and - brightness, Q quits.
55 REM It moves on by itself every 8 seconds until a key or button is used.
60 PN = 28: N = 1: BT = 1: AU = 1
65 IF BT THEN PINMODE 20, PULLUP: PINMODE 21, PULLUP: PINMODE 22, PULLUP
70 NEOPIXEL OPEN PN, N
80 BR = 64: NF = 8: E = 1: T = 0
90 DIM NM$(8): FOR I = 1 TO NF: READ NM$(I): NEXT
100 DATA Rainbow, Breathe, Colour wipe, Chase, Candle, Sparkle, Police, Heartbeat
110 CLS: PRINT "NeoPixel demo:"; N; "LED(s) on GP"; PN
120 PRINT "Space/N next, P previous, + - brightness, Q quits"
130 IF BT THEN PRINT "Buttons: GP20 next, GP21 previous, GP22 brightness"
140 GOSUB 900: GOSUB 950
200 REM The main loop: a frame of the effect, then the keys and buttons
210 ON E GOSUB 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1700
220 NEOPIXEL SHOW: PAUSE 20: T = T + 1
225 IF AU AND T >= 400 THEN E = E MOD NF + 1: GOSUB 900
230 K$ = UCASE$(INKEY$)
240 IF K$ = "Q" OR K$ = CHR$(27) THEN 800
245 IF K$ <> "" THEN AU = 0
250 IF K$ = " " OR K$ = "N" THEN E = E MOD NF + 1: GOSUB 900
260 IF K$ = "P" THEN E = (E + NF - 2) MOD NF + 1: GOSUB 900
270 IF K$ = "+" OR K$ = "=" THEN BR = BR + 32: GOSUB 950
280 IF K$ = "-" THEN BR = BR - 32: GOSUB 950
290 IF BT = 0 THEN 210
295 IF PIN(20) = 0 OR PIN(21) = 0 OR PIN(22) = 0 THEN AU = 0
300 IF PIN(20) = 0 THEN E = E MOD NF + 1: GOSUB 900: GOSUB 980
310 IF PIN(21) = 0 THEN E = (E + NF - 2) MOD NF + 1: GOSUB 900: GOSUB 980
320 IF PIN(22) = 0 THEN BR = BR + 64: GOSUB 950: GOSUB 980
330 GOTO 210
800 NEOPIXEL CLEAR: NEOPIXEL CLOSE: LOCATE 8, 1: PRINT "Bye.": END
900 REM Show the effect's name; start it from its beginning
910 LOCATE 5, 1: PRINT "Effect"; E; ": "; NM$(E); SPACE$(20); "": T = 0
920 LOCATE 7, 1: IF AU THEN PRINT "(changing every 8 seconds)" ELSE PRINT SPACE$(30)
930 SOUND 2000, .5: RETURN
950 REM Brightness 8-255 (the buttons go round)
955 IF BR > 255 AND BR < 300 THEN BR = 255 ELSE IF BR >= 300 THEN BR = 8
960 IF BR < 8 THEN BR = 8
965 NEOPIXEL BRIGHT BR: LOCATE 6, 1: PRINT "Brightness"; BR; "  ": RETURN
980 REM Wait for the buttons to be let go
985 PAUSE 20: IF PIN(20) = 0 OR PIN(21) = 0 OR PIN(22) = 0 THEN 985
990 RETURN
1000 REM Rainbow: the hues spread along the strip, turning
1010 FOR I = 0 TO N - 1: X = (T * 3 + I * 360 / N) MOD 360: GOSUB 2000
1020 NEOPIXEL I, R, G, B: NEXT: RETURN
1100 REM Breathe: one colour fading in and out, a new hue each breath
1110 L = (1 - COS(T * .06)) / 2: X = (INT(T / 105) * 47) MOD 360: GOSUB 2000
1120 NEOPIXEL FILL INT(R * L), INT(G * L), INT(B * L): RETURN
1200 REM Colour wipe: the LEDs fill one at a time, then the next colour
1210 S = INT(T / 4): W = S MOD (N + 4): X = (INT(S / (N + 4)) * 90) MOD 360: GOSUB 2000
1220 FOR I = 0 TO N - 1: IF I < W THEN NEOPIXEL I, R, G, B ELSE NEOPIXEL I, 0, 0, 0
1230 NEXT: RETURN
1300 REM Chase: every third LED lit, stepping along (theatre lights)
1310 S = INT(T / 5): X = (S * 7) MOD 360: GOSUB 2000
1320 FOR I = 0 TO N - 1: IF (I + S) MOD 3 = 0 THEN NEOPIXEL I, R, G, B ELSE NEOPIXEL I, 0, 0, 0
1330 NEXT: RETURN
1400 REM Candle: a warm, flickering flame
1410 FOR I = 0 TO N - 1: IF RND < .5 THEN F = 140 + INT(RND * 115): NEOPIXEL I, F, INT(F * .45), INT(F * .08)
1420 NEXT: RETURN
1500 REM Sparkle: LEDs flashing white over a dim blue
1510 NEOPIXEL FILL 0, 0, 20: IF RND < .3 THEN NEOPIXEL INT(RND * N), 255, 255, 255
1520 RETURN
1600 REM Police: red then blue flashes (on a strip, its two halves)
1610 S = INT(T / 3) MOD 8: V = (S MOD 2 = 0)
1620 FOR I = 0 TO N - 1: IF N = 1 THEN Z = (S >= 4) ELSE Z = (I * 2 >= N)
1630 R = 0: B = 0: IF V AND Z = (S >= 4) THEN R = 255 * (1 + Z): B = -255 * Z
1640 NEOPIXEL I, R, 0, B: NEXT: RETURN
1700 REM Heartbeat: a red double beat, about once a second
1710 S = T MOD 50: L = 0
1720 IF S < 4 THEN L = 1 - S / 4
1730 IF S >= 8 AND S < 14 THEN L = .7 * (1 - (S - 8) / 6)
1740 NEOPIXEL FILL INT(255 * L), 0, 0: RETURN
2000 REM Hue X (0-359) to R, G, B at full saturation
2010 S = INT(X / 60) MOD 6: F = INT((X MOD 60) * 255 / 60)
2020 ON S + 1 GOTO 2030, 2040, 2050, 2060, 2070, 2080
2030 R = 255: G = F: B = 0: RETURN
2040 R = 255 - F: G = 255: B = 0: RETURN
2050 R = 0: G = 255: B = F: RETURN
2060 R = 0: G = 255 - F: B = 255: RETURN
2070 R = F: G = 0: B = 255: RETURN
2080 R = 255: G = 0: B = 255 - F: RETURN
