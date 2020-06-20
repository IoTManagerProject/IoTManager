/*

=head1 NAME

UpTime_tick.ino - print formatted and raw uptime in seconds und 4x interval

=head1 DESCRIPTION

On serial port will print current uptime in seconds with 4 interval checks.
Each loop has random 0-5s delay. Intervals will be printed once elapsed.

=cut

*/

#include <Arduino.h>
#include <UpTime.h>     // https://github.com/jozef/Arduino-UpTime

uptime_interval fire2(2);
uptime_interval fire5(5);
uptime_interval fire10(10, UPTIME_RIGHT_AWAY);
uptime_interval fire60(60, UPTIME_RIGHT_AWAY);

void setup () {
    Serial.begin(9600);
    while (Serial.available()) { Serial.read(); }
    randomSeed(analogRead(0));
}

void loop () {
    Serial.print("uptime: ");
    Serial.println(uptime_as_string());

    if (fire2.check()) {
        Serial.println("fire 2s");
    }
    if (fire5.check()) {
        Serial.println("fire 5s");
    }
    if (fire10.check()) {
        Serial.println("fire 10s");
    }
    if (fire60.check()) {
        Serial.println("fire 60s");
    }

    int rand_delay = random(5000);
    Serial.print("delay(");
    Serial.print(rand_delay);
    Serial.println(")");
    delay(rand_delay);
}
