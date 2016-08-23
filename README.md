# op1-drum

A program to generate a drum file in AIFF format for an
[OP-1](https://www.teenageengineering.com/products/op-1), including the OP-1
proprietary metadata.

# Dependencies

- [json.h](https://github.com/nlohmann/json) (vendored)
- libsndfile with a patch on top ([here for now](http://github.com/padenot/libsndfile))
- [cli.cpp](https://github.com/KoltesDigital/cli.cpp) (vendored)
- CMake for building this project
- libtool for building the custom libsndfile
- `git` to get the modified `libsndfile`
- `doxygen` for the documentation (optional)

# Usage

For now, command line:

```sh
op1-drum
  Usage: op1-drum [options] audio-file [audio-file ...] -o output.aif

  Creates an AIFF file for use with an OP-1, with start and end marker
  included in the file.

Flags:
  -help, -h, -?
    Show help
  -fxon
    Whether the effect is on by default or not.
  -lfoon
    Whether the LFO is on by default or not.
  -normalize, -n
    Normalize each sample before creating the output file.
  -debug, -d
    Enabled console debug print outs.

Options:
  -output, -o (required)
    Output file
  -fxtype, -fx
    Effect type, one of 'cwo', 'delay', 'grid', 'nitro', 'phone', 'punch' or
    'spring'. [default: cwo]
  -lfotype, -lfo
    LFO type, one of 'bend', 'crank', 'element', 'midi', 'random', 'tremolo',
    'value'.  [default: element]
  ```

```sh
op1-dump
  Usage: op1-drum audio-file.aif [audio-file2.aif...]

  Dumps on stdout the proprietary JSON of a OP-1 drum or synth sample.

Flags:
  -help, -h, -?  Show help
```

# Building

OSX or Linux for now.

Run `./deps.sh`. That gets the source for the modified `libsndfile` version,
compiles it, and puts it at the right location.

Run `cmake .`, and `make`.

Run `make doc` to build the documentation. It is generated in `doc`.

# Installation

TODO

# License

Copyright (c) 2016 Paul Adenot
[MIT](https://opensource.org/licenses/MIT)
