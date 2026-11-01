' FADE: fade the LED up and down with PWM. Ctrl-C stops.
PRINT "Fading the LED. Ctrl-C stops."
FOR R = 1 TO 5
  FOR D = 0 TO 100 STEP 2: PWM LED, 1000, D: PAUSE 10: NEXT D
  FOR D = 100 TO 0 STEP -2: PWM LED, 1000, D: PAUSE 10: NEXT D
NEXT R
PWM LED, 0
