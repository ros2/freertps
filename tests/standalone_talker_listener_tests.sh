#!/bin/bash
TALKER=build/native-posix/apps/standalone_talk_n/standalone_talk_n
LISTENER=build/native-posix/apps/standalone_listen_for_n/standalone_listen_for_n
# wait for 10 messages or a 30-second timeout
$LISTENER 10 2 &
LISTENER_PID=$!
# send 10 messages at 0.5-second intervals
$TALKER 10 0.1 &
wait -n $LISTENER_PID
RC=$?
# wait for terminal debris to stop
sleep 0.5
# print success or failure
echo "===================================================="
if [ $RC -eq 0 ]; then
  echo "HOORAY! IT WORKS! HOORAY! CONFETTI! KAZOOS!"
else
  echo "bogus........ it didn't work."
fi
