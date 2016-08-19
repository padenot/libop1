# op1-drum

A program to generate a drum file in AIFF format for an
[OP-1](https://www.teenageengineering.com/products/op-1), including the OP-1
proprietary metadata.

# Dependencies

- [json.h](https://github.com/nlohmann/json) (vendored)
- libsndfile with a patch on top ([here for now](http://github.com/padenot/libsndfile))
- [cli.cpp](https://github.com/KoltesDigital/cli.cpp) (vendored)

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

# License

Copyright (c) 2016 Paul Adenot
[MIT](https://opensource.org/licenses/MIT)
