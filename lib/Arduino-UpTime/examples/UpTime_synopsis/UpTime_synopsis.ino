/*
=head1 DESCRIPTION

synopsis section example from UpTime.h

=cut
*/

#include <Arduino.h>
#include <UpTime.h>     // https://github.com/jozef/Arduino-UpTime

uptime_interval fire2(2);
uptime_interval fire5(5,UPTIME_RIGHT_AWAY);

void setup () {
    Serial.begin(9600);
}

void loop () {
    Serial.println("uptime: "+uptime_as_string()+" or "+uptime()+"s");
    if (fire2.check()) Serial.println("2s elapsed");
    if (fire5.check()) Serial.println("5s elapsed");
    delay(1400);
}
