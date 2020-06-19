/* VERSION 0.01; 14.1.2019; see below for description and documentation */
#ifndef UpTime_h
#define UpTime_h

#define UPTIME_RIGHT_AWAY false

unsigned long uptime();
String uptime_as_string();

class uptime_interval {
    private:
        unsigned long next;
        unsigned int interval;
    public:
        uptime_interval(unsigned int inte, bool postpone = true);
        bool check();
        void reset(bool postpone = true);
};

#endif

/*

=head1 NAME

    UpTime.h - Arduino uptime and events in seconds

=head1 SYNOPSIS

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

will output:

    uptime: 00:00:00 or 0s
    5s elapsed
    uptime: 00:00:01 or 1s
    uptime: 00:00:02 or 2s
    2s elapsed
    uptime: 00:00:04 or 4s
    2s elapsed
    uptime: 00:00:05 or 5s
    5s elapsed
    uptime: 00:00:07 or 7s
    2s elapsed
    uptime: 00:00:08 or 8s
    2s elapsed
    uptime: 00:00:09 or 9s
    uptime: 00:00:11 or 11s
    2s elapsed
    5s elapsed
    …
    uptime: 04:41:23 or 16883s
    uptime: 04:41:25 or 16885s
    2s elapsed
    5s elapsed
    uptime: 04:41:26 or 16886s
    2s elapsed
    uptime: 04:41:28 or 16888s
    2s elapsed
    uptime: 04:41:29 or 16889s
    uptime: 04:41:30 or 16890s
    2s elapsed
    5s elapsed

=head1 DESCRIPTION

Uptime class is made to to track uptime of Arduino in seconds. The uptime()
or check() functions has to be called at least once for 0xFFFF seconds
(once in 18h) to work. Seconds will be counted even after Arduinos
millis() overrun and the seconds of unsigned long are enough to not overrun
sooner then in 136+ years. Once 0xFFFF0000 seconds is elapsed, will trigger
reset so that Arduino program clearly starts all over again.

=head1 METHODS

=head2 bool check()

Returns true/false if the interval elapset.

=head2 void reset(bool postpone = true)

Will reset the time to count from current moment in until interval. If
C<postpone> is set to false, check() will return true with next call.

=head1 INSTALL

    git clone https://github.com/jozef/Arduino-UpTime sketchbook/libraries/UpTime

=head1 EXAMPLES

=head2 examples/UpTime_tick/UpTime_synopsis.ino

synopsis section example

=head2 examples/UpTime_tick/UpTime_tick.ino

print formatted and raw uptime in seconds und 4x interval

=head1 LICENSE

This is free software, licensed under the MIT License.

=head1 AUTHOR

    Jozef Kutej

=cut

*/
