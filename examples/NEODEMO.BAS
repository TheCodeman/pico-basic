' NEODEMO.BAS: eight light effects on WS2812B LEDs (NeoPixels).
' The Maker Pi Pico's RGB LED (GP28) by default. For a strip, set PN to
' its pin and N to its length below (5 V strips: their own power, GND
' joined to the Pico's). BT = 0 if there are no buttons on GP20-22.
' Keys: Space or N next, P previous, + and - brightness, Q quits.
' It moves on by itself every 8 seconds until a key or button is used.
PN = 28: N = 1: BT = 1: AU = 1
IF BT THEN PINMODE 20, PULLUP: PINMODE 21, PULLUP: PINMODE 22, PULLUP
NEOPIXEL OPEN PN, N
BR = 64: NF = 8: E = 1: T = 0
DIM NM$(8): FOR I = 1 TO NF: READ NM$(I): NEXT
DATA Rainbow, Breathe, Colour wipe, Chase, Candle, Sparkle, Police, Heartbeat
CLS: PRINT "NeoPixel demo:"; N; "LED(s) on GP"; PN
PRINT "Space/N next, P previous, + - brightness, Q quits"
IF BT THEN PRINT "Buttons: GP20 next, GP21 previous, GP22 brightness"
GOSUB showeffect: GOSUB setbright

' The main loop: a frame of the effect, then the keys and buttons
frame:
  ON E GOSUB rainbow, breathe, wipe, chase, candle, sparkle, police, heartbeat
  NEOPIXEL SHOW: PAUSE 20: T = T + 1
  IF AU AND T >= 400 THEN E = E MOD NF + 1: GOSUB showeffect
  K$ = UCASE$(INKEY$)
  IF K$ = "Q" OR K$ = CHR$(27) THEN finish
  IF K$ <> "" THEN AU = 0
  IF K$ = " " OR K$ = "N" THEN E = E MOD NF + 1: GOSUB showeffect
  IF K$ = "P" THEN E = (E + NF - 2) MOD NF + 1: GOSUB showeffect
  IF K$ = "+" OR K$ = "=" THEN BR = BR + 32: GOSUB setbright
  IF K$ = "-" THEN BR = BR - 32: GOSUB setbright
  IF BT = 0 THEN frame
  IF PIN(20) = 0 OR PIN(21) = 0 OR PIN(22) = 0 THEN AU = 0
  IF PIN(20) = 0 THEN E = E MOD NF + 1: GOSUB showeffect: GOSUB letgo
  IF PIN(21) = 0 THEN E = (E + NF - 2) MOD NF + 1: GOSUB showeffect: GOSUB letgo
  IF PIN(22) = 0 THEN BR = BR + 64: GOSUB setbright: GOSUB letgo
  GOTO frame
finish:
  NEOPIXEL CLEAR: NEOPIXEL CLOSE: LOCATE 8, 1: PRINT "Bye.": END

' Show the effect's name; start it from its beginning
showeffect:
  LOCATE 5, 1: PRINT "Effect"; E; ": "; NM$(E); SPACE$(20); "": T = 0
  LOCATE 7, 1: IF AU THEN PRINT "(changing every 8 seconds)" ELSE PRINT SPACE$(30)
  SOUND 2000, .5: RETURN

' Brightness 8-255 (the buttons go round)
setbright:
  IF BR > 255 AND BR < 300 THEN BR = 255 ELSE IF BR >= 300 THEN BR = 8
  IF BR < 8 THEN BR = 8
  NEOPIXEL BRIGHT BR: LOCATE 6, 1: PRINT "Brightness"; BR; "  ": RETURN

' Wait for the buttons to be let go
letgo:
  PAUSE 20: IF PIN(20) = 0 OR PIN(21) = 0 OR PIN(22) = 0 THEN letgo
  RETURN

' Rainbow: the hues spread along the strip, turning
rainbow:
  FOR I = 0 TO N - 1: X = (T * 3 + I * 360 / N) MOD 360: GOSUB hue
  NEOPIXEL I, R, G, B: NEXT: RETURN

' Breathe: one colour fading in and out, a new hue each breath
breathe:
  L = (1 - COS(T * .06)) / 2: X = (INT(T / 105) * 47) MOD 360: GOSUB hue
  NEOPIXEL FILL INT(R * L), INT(G * L), INT(B * L): RETURN

' Colour wipe: the LEDs fill one at a time, then the next colour
wipe:
  S = INT(T / 4): W = S MOD (N + 4): X = (INT(S / (N + 4)) * 90) MOD 360: GOSUB hue
  FOR I = 0 TO N - 1: IF I < W THEN NEOPIXEL I, R, G, B ELSE NEOPIXEL I, 0, 0, 0
  NEXT: RETURN

' Chase: every third LED lit, stepping along (theatre lights)
chase:
  S = INT(T / 5): X = (S * 7) MOD 360: GOSUB hue
  FOR I = 0 TO N - 1: IF (I + S) MOD 3 = 0 THEN NEOPIXEL I, R, G, B ELSE NEOPIXEL I, 0, 0, 0
  NEXT: RETURN

' Candle: a warm, flickering flame
candle:
  FOR I = 0 TO N - 1: IF RND < .5 THEN F = 140 + INT(RND * 115): NEOPIXEL I, F, INT(F * .45), INT(F * .08)
  NEXT: RETURN

' Sparkle: LEDs flashing white over a dim blue
sparkle:
  NEOPIXEL FILL 0, 0, 20: IF RND < .3 THEN NEOPIXEL INT(RND * N), 255, 255, 255
  RETURN

' Police: red then blue flashes (on a strip, its two halves)
police:
  S = INT(T / 3) MOD 8: V = (S MOD 2 = 0)
  FOR I = 0 TO N - 1: IF N = 1 THEN Z = (S >= 4) ELSE Z = (I * 2 >= N)
  R = 0: B = 0: IF V AND Z = (S >= 4) THEN R = 255 * (1 + Z): B = -255 * Z
  NEOPIXEL I, R, 0, B: NEXT: RETURN

' Heartbeat: a red double beat, about once a second
heartbeat:
  S = T MOD 50: L = 0
  IF S < 4 THEN L = 1 - S / 4
  IF S >= 8 AND S < 14 THEN L = .7 * (1 - (S - 8) / 6)
  NEOPIXEL FILL INT(255 * L), 0, 0: RETURN

' Hue X (0-359) to R, G, B at full saturation
hue:
  S = INT(X / 60) MOD 6: F = INT((X MOD 60) * 255 / 60)
  ON S + 1 GOTO hue0, hue1, hue2, hue3, hue4, hue5
hue0: R = 255: G = F: B = 0: RETURN
hue1: R = 255 - F: G = 255: B = 0: RETURN
hue2: R = 0: G = 255: B = F: RETURN
hue3: R = 0: G = 255 - F: B = 255: RETURN
hue4: R = F: G = 0: B = 255: RETURN
hue5: R = 255: G = 0: B = 255 - F: RETURN
