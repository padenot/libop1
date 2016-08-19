# op1-drum

A work-in-progress program to generate a drum file for an op-1

# Dependencies
- [json.h](https://github.com/nlohmann/json) (vendored)
- libsndfile with a patch on top ([here for now](http://github.com/padenot/libsndfile))
- [cli.cpp](https://github.com/KoltesDigital/cli.cpp) (vendored)

# Usage
For now, command line:

```sh
op1-drum kick.wav snare.wav clap.wav hh-open.wav [...]
```
