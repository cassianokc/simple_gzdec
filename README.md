# gzdec

GStreamer zlib-compressed decoder plugin.

It uses zlib based compression for gzip instead of gzip implementation, as it was the one available as a C library.
The formats aren't compatible though, so compressing something with gzip command wouldn't work directly.

## Requirements
- Instructions assume Debian 10+ or Ubuntu 20.0+:

[Ubuntu]
```sh
sudo apt-get install build-essential
sudo apt-get install gstreamer1.0-*
sudo apt-get install zlib1g zlib1g-dev
```

## Build
- Simply use make command

```sh
make
```

## Tests
- Repository contains a file with a test example inside. In order to test it with expected output run:

```sh
make test
```

