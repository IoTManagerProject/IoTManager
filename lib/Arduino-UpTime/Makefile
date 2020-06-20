all: README examples/UpTime_tick/README examples/UpTime_synopsis/README

clean:
	rm -f README README.bak \
			examples/UpTime_tick/README examples/UpTime_tick/README.bak \
			examples/UpTime_synopsis/README examples/UpTime_synopsis/README.bak

README: UpTime.h
	pod2readme $< $@ && rm -f $@.bak

examples/UpTime_tick/README: examples/UpTime_tick/UpTime_tick.ino
	pod2readme $< $@ && rm -f $@.bak
	
examples/UpTime_synopsis/README: examples/UpTime_synopsis/UpTime_synopsis.ino
	pod2readme $< $@ && rm -f $@.bak
