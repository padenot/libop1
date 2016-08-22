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

uint32_t frame_to_op1_time(uint32_t frame)
{
  // Maximum amount of data for a drum sample on an op-1
  const int BYTES_IN_12_SECS = 44100 * 2 * 12;
  const int OP1_DRUMKIT_END = 0x7FFFFFFE;
  return OP1_DRUMKIT_END / BYTES_IN_12_SECS * frame * sizeof(uint16_t);
}

int main(int argc, const char ** argv)
{
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

  vector<string> valid_effects = {
    "cwo", "delay", "grid", "nitro", "phone", "punch", "spring"
  };

  if (find(valid_effects.begin(), valid_effects.end(), fx_type) == valid_effects.end()) {
    parser.showHelp();
    return EXIT_FAILURE;
  }

  vector<string> valid_lfo = {
    "bend", "crank", "element", "midi", "random", "tremolo", "value"
  };

  if (find(valid_lfo.begin(), valid_lfo.end(), lfo_type) == valid_lfo.end()) {
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

  // compute start an end time
  vector<int32_t> start;
  vector<int32_t> end;
  int32_t acc = 0;

  start.resize(24, 0);
  end.resize(24, 0);

  for (uint32_t i = 0; i < files.size(); i++) {
    size_t sample_count;
    op1_sample_get_length(files[i], &sample_count);

    start[i] = acc;
    acc += sample_count;
    end[i] = acc + 1;
  }

  for (uint32_t i = files.size(); i < 24; i++) {
    start[i] = acc;
    end[i] = acc;
  }

  for (uint32_t i = 0; i < 24; i++) {
    start[i] = frame_to_op1_time(start[i]);
    end[i] = frame_to_op1_time(end[i]);
  }


  std::vector<int> playmode;
  for (int32_t i = 0; i < 24; i++) {
    // playmode.push_back(4096); // playmode ->
    playmode.push_back(8192); // playmode ->|
    // playmode.push_back(20480); // playmode loop
  }

  std::vector<int> direction;
  for (int32_t i = 0; i < 24; i++) {
    direction.push_back(8192); // forward
    // direction.push_back(18432); // reverse;
  }

  std::vector<int> volume;
  for (int32_t i = 0; i < 24; i++) {
    volume.push_back(8192);  // TODO
  }

  std::vector<int> dyna_env;
  dyna_env.push_back(0);
  dyna_env.push_back(8192);
  dyna_env.push_back(0);
  dyna_env.push_back(8192);
  dyna_env.push_back(0);
  dyna_env.push_back(0);
  dyna_env.push_back(0);
  dyna_env.push_back(0);

  std::vector<int> fx_params;
  for (int32_t i = 0; i < 8; i++) {
    fx_params.push_back(8000);
  }

  std::vector<int> lfo_params;
  for (int32_t i = 0; i < 4; i++) {
    lfo_params.push_back(16000);
  }
  for (int32_t i = 0; i < 4; i++) {
    lfo_params.push_back(0);
  }

  // make string
  json j;

  j["drum_version"] = 1;
  j["type"] = "drum";
  j["name"] = "user";
  j["octave"] = 0;
  j["pitch"] = std::vector<int>(24);
  j["start"] = start;
  j["end"] = end;
  j["playmode"] = playmode;
  j["reverse"] = direction;
  j["volume"] = volume;
  j["dyna_env"] = dyna_env;
  j["fx_active"] = fx_on;
  j["fx_type"] = fx_type;
  j["fx_params"] = fx_params;
  j["lfo_active"] = lfo_on;
  j["lfo_type"] = lfo_type;
  j["lfo_params"] = lfo_params;

  string serialized = j.dump();

  LOG("json chunk: %s\n", serialized.c_str());

  // write aiff
  const char * temp_file = "op1-drum-temp.aiff";
  SF_INFO info;
  int rate, rv;
  rv = op1_sample_get_rate(files[0], &rate);
  if (!rv) {
    FATAL("Can't get rate.");
  }
  info.samplerate = rate;
  info.channels = 1; // drums are mono
  info.format = SF_FORMAT_AIFF | SF_FORMAT_PCM_16 | SF_ENDIAN_BIG;
  SNDFILE* outfile = sf_open(temp_file, SFM_WRITE, &info);
  if (!outfile) {
    FATAL("Could not open file for writing");
  }

  // set string
  sf_set_string(outfile, SF_STR_SOFTWARE, serialized.c_str());

  for (int32_t i = 0; i < files.size(); i++) {
    int16_t * samples;
    size_t sample_count;
    op1_sample_get_data(files[i], &samples, &sample_count);

    sf_count_t count = sf_writef_short(outfile, samples, sample_count) ;
    if (count != sample_count) {
      WARN("Weird write.");
    }
    int16_t d = 0;
    count = sf_writef_short(outfile, &d, 1) ;
    if (count != 1) {
      WARN("Weird write.");
    }
  }

  rv = sf_close(outfile);
  if (rv != 0) {
    FATAL("Could not close output file.");
  }

  // reopen the file and fix the APPL string
  FILE* f = fopen(temp_file, "r");

  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  uint8_t* buf = (uint8_t*)malloc(fsize);
  fread(buf, fsize, 1, f);
  fclose(f);

  // find AAPL capture pattern
  size_t i = 0;
  while (i < fsize - 4) {
    if (buf[i + 0] == 'A' &&
        buf[i + 1] == 'P' &&
        buf[i + 2] == 'P' &&
        buf[i + 3] == 'L') {
      break;
    }
    i++;
  }

  assert(buf[i + 0] == 'A' &&
         buf[i + 1] == 'P' &&
         buf[i + 2] == 'P' &&
         buf[i + 3] == 'L');

  i += 4;

  uint32_t chunk_size_index = i;

  LOG("chunk_size_index: %d\n", chunk_size_index);

  i += 4;

  assert(buf[i + 0] == 'm' &&
         buf[i + 1] == '3' &&
         buf[i + 2] == 'g' &&
         buf[i + 3] == 'a');

  // write the APPL for the op-1, which is 'op-1'
  buf[i + 0] = 'o';
  buf[i + 1] = 'p';
  buf[i + 2] = '-';
  buf[i + 3] = '1';

  // capture the libsnd version string, and remove it
  while (i < fsize - 4) {
    if (buf[i + 0] == ' ' &&
        buf[i + 1] == '(' &&
        buf[i + 2] == 'l' &&
        buf[i + 3] == 'i') {
      break;
    }
    i++;
  }

  uint32_t end_json = i;

  uint32_t removed_char = 0;
  while (buf[i] != ')') {
    removed_char++;
    i++;
  }

  removed_char++;
  i++;

  int32_t chunk_size = (buf[chunk_size_index + 3] << 0)
                     | (buf[chunk_size_index + 2] << 8)
                     | (buf[chunk_size_index + 1] << 16)
                     | (buf[chunk_size_index + 0] << 24);
  LOG("APPL chunk_size: %d\n", chunk_size);

  // move back the data and update the chunk size
  memmove(buf + end_json, buf + end_json + removed_char, fsize - end_json - removed_char);

  chunk_size -= removed_char;

  LOG("new chunk size: %d\n", chunk_size);

  // udpate the AAPL chunk size
  buf[chunk_size_index + 0] = chunk_size >> 24;
  buf[chunk_size_index + 1] = chunk_size >> 16;
  buf[chunk_size_index + 2] = chunk_size >>  8;
  buf[chunk_size_index + 3] = chunk_size;

  // write the new file
  f = fopen(output, "w");
  if (!f) {
    FATAL("Could not open final file for writing.");
  }
  long written = fwrite(buf, fsize, 1, f);
  if (written != 1) {
    FATAL("Did not write all the data.");
  }
  rv = fclose(f);

  if (rv) {
    WARN("Could not close temporary file.");
  }

  rv = remove(temp_file);
  if (rv) {
    WARN("Could not remove temporary file.");
  }

  return 0;
}
