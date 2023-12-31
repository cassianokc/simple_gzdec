
CC=gcc
CCFLAG=-Wall -std=c11 $$(pkg-config --cflags --libs gstreamer-1.0 gstreamer-app-1.0 ) -DHAVE_CONFIG_H -DFILE_OFFSET_BITS=64 -fPIC -O0
CLFLAG=-shared -Wl,--no-undefined -Wl,--as-needed -Wl,--no-undefined -shared -fPIC `pkg-config --libs gstreamer-1.0` -lz
export GST_PLUGIN_PATH := $(shell pwd)

libgstgzdec.so: gstgzdec.o
	$(CC) -o $@ $< $(CLFLAG)


gstgzdec.o: gstgzdec.c gstgzdec.h
	$(CC) $(CCFLAG) -c $<

clean:
	rm -f gstgzdec.o libgstgzdec.so out.txt

test: libgstgzdec.so
	gst-launch-1.0 filesrc location=resources/test.txt.gz ! gzdec silent=false ! filesink location="out.txt"
	diff -q out.txt resources/test.txt

