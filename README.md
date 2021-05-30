# simpeg: SIMple jPEG Decoder-Encoder

This repo does nothing expect for a decode-encode process of reading RGB data from a `jpg` image, and output as another `jpg` file.

## Dependency

- `libjpeg` or `libjpeg-turbo` to load and convert JPEG information into color space

## Build & Run

Require C++11.
```bash
$ make
$ ./simpeg demo/lena-color.jpg demo/lena-color-30.jpg 30
```

## Demo

![origin](./demo/lena-color.jpg)![compressed](./demo/lena-color-30.jpg)

The left one is the origin with 160KB size while the right is JPEG compressed image with quality equals to 30 and only 10KB size.

## References

- [read jpeg image with libjpeg](https://gist.github.com/kentakuramochi/f64e7646f1db8335c80f131be8359044)
- [jpeg compress algorithm](https://create.stephan-brumme.com/toojpeg/)
