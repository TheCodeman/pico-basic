' WATCHDOG.BAS: the board resets itself if the program stops feeding the
' watchdog (a hang, or a sensor read that never comes back). Put it in
' AUTORUN.BAS and an unattended board starts again by itself.
IF WDRESET THEN PRINT "The watchdog reset the board last time"
WATCHDOG 2000
PRINT "Watchdog on: 2 seconds. Feeding it 10 times..."
FOR I = 1 TO 10
  PAUSE 500
  WATCHDOG FEED
  PRINT I;
NEXT
PRINT
PRINT "Now it isn't fed: the board resets in 2 seconds (Ctrl-C stops it)"
loop:
  GOTO loop
