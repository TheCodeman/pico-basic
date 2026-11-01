' MQTT.BAS: publishes the time and an ADC reading every 5 seconds, and
' sets the LED from what comes on a topic (ON, OFF, or a number: 1 or 0).
' Wi-Fi builds; set BROKER$ (or MQTT_BROKER in CONFIG.TXT and BROKER$ = "").
BROKER$ = "192.168.1.227"
TOPIC$ = "test/topic"
MQTT CONNECT BROKER$
MQTT SUBSCRIBE TOPIC$ + "/led"
ON MQTT GOSUB message
MQTT ON
PRINT "Publishing to "; TOPIC$; "/status; send ON or OFF to "; TOPIC$; "/led"
PRINT "Ctrl-C stops"
loop:
  IF MQTT(0) THEN MQTT PUBLISH TOPIC$ + "/status", TIME$ + " ADC " + STR$(ADC(26))
  T = TIMER
  WHILE TIMER - T < 5: WEND
  GOTO loop

message:
  T$ = MQTT$(0): M$ = UCASE$(MQTT$(1))
  PRINT T$; " = "; M$
  IF M$ = "ON" OR M$ = "1" THEN PIN LED, 1
  IF M$ = "OFF" OR M$ = "0" THEN PIN LED, 0
  RETURN
