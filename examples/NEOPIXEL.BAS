' NEOPIXEL.BAS: red, green, blue, then a rainbow on WS2812B LEDs.
' The Maker Pi Pico's RGB LED is one, on GP28. For a strip, give its
' pin and length: N = 30: NEOPIXEL OPEN 16, N (5 V strips need power of
' their own, and GND joined to the Pico's). Any key stops.
N = 1: NEOPIXEL OPEN 28, N
NEOPIXEL BRIGHT 64
DATA 255,0,0, 0,255,0, 0,0,255
FOR K = 1 TO 3: READ R, G, B: NEOPIXEL FILL R, G, B: NEOPIXEL SHOW: PAUSE 700: NEXT
H = 0
rainbow:
  FOR I = 0 TO N - 1
    X = (H + I * 360 / N) MOD 360: GOSUB hue
    NEOPIXEL I, R, G, B
  NEXT I
  NEOPIXEL SHOW: PAUSE 20
  H = (H + 3) MOD 360
  IF INKEY$ = "" THEN rainbow
NEOPIXEL CLEAR: NEOPIXEL CLOSE: END

' Hue X (0-359) to R, G, B at full saturation
hue:
  S = INT(X / 60): F = (X MOD 60) * 255 / 60
  ON S + 1 GOTO hue0, hue1, hue2, hue3, hue4, hue5
hue0: R = 255: G = F: B = 0: RETURN
hue1: R = 255 - F: G = 255: B = 0: RETURN
hue2: R = 0: G = 255: B = F: RETURN
hue3: R = 0: G = 255 - F: B = 255: RETURN
hue4: R = F: G = 0: B = 255: RETURN
hue5: R = 255: G = 0: B = 255 - F: RETURN
