10 REM NEOPIXEL.BAS: red, green, blue, then a rainbow on WS2812B LEDs.
20 REM The Maker Pi Pico's RGB LED is one, on GP28. For a strip, give its
30 REM pin and length: N = 30: NEOPIXEL OPEN 16, N (5 V strips need power of
40 REM their own, and GND joined to the Pico's). Any key stops.
50 N = 1: NEOPIXEL OPEN 28, N
60 NEOPIXEL BRIGHT 64
70 DATA 255,0,0, 0,255,0, 0,0,255
80 FOR K = 1 TO 3: READ R, G, B: NEOPIXEL FILL R, G, B: NEOPIXEL SHOW: PAUSE 700: NEXT
90 H = 0
100 FOR I = 0 TO N - 1
110   X = (H + I * 360 / N) MOD 360: GOSUB 200
120   NEOPIXEL I, R, G, B
130 NEXT I
140 NEOPIXEL SHOW: PAUSE 20
150 H = (H + 3) MOD 360
160 IF INKEY$ = "" THEN 100
170 NEOPIXEL CLEAR: NEOPIXEL CLOSE: END
200 REM Hue X (0-359) to R, G, B at full saturation
210 S = INT(X / 60): F = (X MOD 60) * 255 / 60
220 ON S + 1 GOTO 230, 240, 250, 260, 270, 280
230 R = 255: G = F: B = 0: RETURN
240 R = 255 - F: G = 255: B = 0: RETURN
250 R = 0: G = 255: B = F: RETURN
260 R = 0: G = 255 - F: B = 255: RETURN
270 R = F: G = 0: B = 255: RETURN
280 R = 255: G = 0: B = 255 - F: RETURN
