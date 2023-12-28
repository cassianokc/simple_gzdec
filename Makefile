
CC=gcc
CCFLAG=-Wall -Wextra -std=c11 $$(pkg-config --cflags --libs gstreamer-1.0 gstreamer-app-1.0 ) -DHAVE_CONFIG_H -DFILE_OFFSET_BITS=64 -fPIC -O0
CLFLAG=-shared -Wl,--no-undefined -Wl,--as-needed -Wl,--no-undefined -shared -fPIC `pkg-config --libs gstreamer-1.0` -lz

libgstgzdec.so: gstgzdec.o
	$(CC) -o $@ $< $(CLFLAG)


gstgzdec.o: gstgzdec.c gstgzdec.h
	$(CC) $(CCFLAG) -c $<


clean:
	rm gstgzdec.o libgstgzdec.so
