#include "sndfile.h"
#include "json.hpp"

#include "cli.hpp"
#include "op1.h"
#include <vector>
#include <cstdlib>
#include <cmath>

using namespace std;
using json = nlohmann::json;

void normalize_buffer(int16_t * samples, size_t sample_count)
{
  int16_t max_abs = 0;

  for (int i = 0; i < sample_count; i++) {
    max_abs = max<int16_t>(abs(samples[i]), max_abs);
  }
  float gain = (2 << 14) / max_abs;

  for (int i = 0; i < sample_count; i++) {
    samples[i] = samples[i] * gain;
  }
}

int main(int argc, const char ** argv) {
  cli::Parser parser(argc, argv);

  parser.help() << R"(op1-drum
    Usage: op1-drum [options] audio-file [audio-file ...] -o output.aif

    Creates an AIFF file for use with an OP-1, with start and end marker included in the file.)";

  auto output = parser.option("output")
                      .alias("o")
                      .description("Output file")
                      .required()
                      .getValue();

  auto fx_type = parser.option("fxtype")
                       .alias("fx")
                       .description("Effect type, one of 'cwo', 'delay', 'grid', 'nitro', 'phone', 'punch' or 'spring'.")
                       .defaultValue("cwo")
                       .getValue();

  auto fx_on = parser.flag("fxon")
                     .description("Whether the effect is on by default or not.")
                     .getValue();

  auto lfo_type = parser.option("lfotype")
                        .alias("lfo")
                        .description("LFO type, one of 'bend', 'crank', 'element', 'midi', 'random', 'tremolo', 'value'.")
                        .defaultValue("element")
                        .getValue();

  auto lfo_on = parser.flag("lfoon")
                     .description("Whether the LFO is on by default or not.")
                     .getValue();

  auto normalize = parser.flag("normalize")
                         .alias("n")
                         .description("Normalize each sample before creating the output file.")
                         .getValue();

  g_logging_enabled = parser.flag("debug")
                            .alias("d")
                            .description("Enabled console debug print outs.")
                            .getValue();

  op1_drum * drum;

  if (op1_drum_init(&drum)) {
    FATAL("Could not allocate memory.");
    return EXIT_FAILURE;
  }

  if (op1_drum_set_fx(drum, fx_type)) {
    parser.showHelp();
    return EXIT_FAILURE;
  }

  if (parser.hasErrors()) {
    return EXIT_FAILURE;
  }

  parser.getRemainingArguments(argc, argv);
  // load all files
  vector<audio_file*> files;

  if (argc >= 25) {
    parser.showHelp();
    FATAL("No more than 24 files on an op-1.");
  }

  if (argc == 1) {
    parser.showHelp();
    FATAL("Need some audio files as arguments.");
  }

  for (uint32_t i = 1; i < argc; i++) {
    audio_file * file;
    op1_sample_load(argv[i], &file);
    files.push_back(file);
  }

  if (normalize) {
    for (uint32_t i = 0; i < files.size(); i++) {
      int16_t * samples;
      size_t sample_count;
      op1_sample_get_data(files[i], &samples, &sample_count);
      normalize_buffer(samples, sample_count);
    }
  }

  for (uint32_t i = 0; i < files.size(); i++) {
    op1_drum_add_sample(drum, files[i]);
  }

  int rv;

  rv = op1_drum_set_fx(drum, fx_type);
  if (rv) {
    WARN("Could not set fx type.");
    parser.showHelp();
  }

  rv = op1_drum_set_fx_active(drum, fx_on);
  if (rv) {
    WARN("Could not set fx active.");
    parser.showHelp();
  }

  rv = op1_drum_set_lfo(drum, lfo_type);
  if (rv) {
    WARN("Could not set lfo type.");
    parser.showHelp();
  }

  rv = op1_drum_set_lfo_active(drum, lfo_on);
  if (rv) {
    WARN("Could not set lfo active.");
    parser.showHelp();
  }

  rv = op1_drum_write(drum, output);
  if (rv) {
    WARN("Could not write the output file.");
    return 1;
  }

  return 0;
}
