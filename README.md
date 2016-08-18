# op1-drum

A work-in-progress program to generate a drum file for an op-1

# Dependencies
- [json.h](http://github.com/nlohmann/json) (vendored)
- libsndfile with a patch on top (http://github.com/padenot/libsndfile)

# Usage
For now, command line:

```sh
op1-drum kick.wav snare.wav clap.wav hh-open.wav [...]
```
