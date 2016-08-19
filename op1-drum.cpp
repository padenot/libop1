#include "sndfile.h"
#include "json.hpp"
#include "cli.hpp"
#include <vector>
#include <cstdlib>

using namespace std;
using json = nlohmann::json;

#define DEBUG

#define FATAL(str) \
  fprintf(stderr, "Fatal: %s\n", (str)); \
  abort();

#define WARN(str) \
  fprintf(stderr, "Warning: %s\n", (str));

template<typename T>
void PodZero(T blob)
{
  memset(&blob, 0, sizeof(T));
}

struct audio_file
{
  audio_file()
  {
    PodZero(info);
  }

  SF_INFO info;
  vector<int16_t> data;
};

void load_file(const char* name, audio_file * afile)
{
  SF_INFO info;

  PodZero(info);

  SNDFILE* file = sf_open(name, SFM_READ, &info);
  if (!file) {
    FATAL("Could not open file.");
  }

#ifdef DEBUG
  printf("%s - rate: %d - frame count: %lld\n", name, info.samplerate, info.frames);
#endif

  size_t samples = info.frames * info.channels;
  afile->info = info;
  afile->data.resize(samples);

  sf_count_t count = sf_read_short(file, &(afile->data[0]), info.frames);
  if (count != info.frames) {
    WARN("Unexpected number of frames.");
  }

  int rv = sf_close(file);
  if (rv != 0) {
    FATAL("Could not close file.");
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
    Usage: op1-drum [options] audio-file [audio-file]...)";

  auto output = parser.option("output")
    .alias("o")
    .description("Output file")
    .required()
    .getValue();

  if (parser.hasErrors()) {
    return EXIT_FAILURE;
  }

  parser.getRemainingArguments(argc, argv);
  // load all files
  vector<audio_file> files;

  if (argc >= 25) {
    FATAL("No more than 24 files on an op-1.");
  }

  if (argc == 1) {
    FATAL("Need some audio files as arguments.");
  }

  for (uint32_t i = 1; i < argc; i++) {
    files.push_back(audio_file());
    load_file(argv[i], &(files[i - 1]));
  }


  // compute start an end time
  vector<int32_t> start;
  vector<int32_t> end;
  int32_t acc = 0;

  start.resize(24, 0);
  end.resize(24, 0);

  for (uint32_t i = 0; i < files.size(); i++) {
    start[i] = acc;
    acc += files[i].data.size();
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
  j["fx_active"] = false;
  j["fx_type"] = "cwo";
  j["fx_params"] = fx_params;
  j["lfo_active"] = false;
  j["lfo_type"] = "tremolo";
  j["lfo_params"] = lfo_params;

  string serialized = j.dump();

#ifdef DEBUG
  printf("json chunk: %s\n", serialized.c_str());
#endif

  // check zero crossing
  // TODO

  // write aiff
  const char * temp_file = "op1-drum-temp.aiff";
  SF_INFO info;
  info.samplerate = files[0].info.samplerate;
  info.channels = 1; // drums are mono
  info.format = SF_FORMAT_AIFF | SF_FORMAT_PCM_16 | SF_ENDIAN_BIG;
  SNDFILE* outfile = sf_open(temp_file, SFM_WRITE, &info);
  if (!outfile) {
    FATAL("Could not open file for writing");
  }

  // set string
  sf_set_string(outfile, SF_STR_SOFTWARE, serialized.c_str());

  for (int32_t i = 0; i < files.size(); i++) {
    sf_count_t count = sf_writef_short(outfile, &(files[i].data[0]), files[i].data.size()) ;
    if (count != files[i].data.size()) {
      WARN("Weird write.");
    }
    int16_t d = 0;
    count = sf_writef_short(outfile, &d, 1) ;
    if (count != 1) {
      WARN("Weird write.");
    }
  }

  int rv = sf_close(outfile);
  if (rv != 0) {
    FATAL("Could not close output file.");
  }

  printf("%s\n", temp_file);

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
#ifdef DEBUG
    printf("'%c' '%c' '%c' '%c'\n", buf[i], buf[i+1], buf[i+2], buf[i+3]);
#endif
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

#ifdef DEBUG
  printf("APPL => '%02X' '%02X' '%02X' '%02X'\n", buf[i], buf[i+1], buf[i+2], buf[i+3]);
#endif

  i += 4;

#ifdef DEBUG
  printf("hexa chunk size: '%02X' '%02X' '%02X' '%02X'\n", buf[i], buf[i+1], buf[i+2], buf[i+3]);
#endif

  uint32_t chunk_size_index = i;

#ifdef DEBUG
  printf("chunk_size_index: %d\n", chunk_size_index);
#endif

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
#ifdef DEBUG
    printf("'%c' '%c' '%c' '%c'\n", buf[i], buf[i+1], buf[i+2], buf[i+3]);
#endif
    if (buf[i + 0] == ' ' &&
        buf[i + 1] == '(' &&
        buf[i + 2] == 'l' &&
        buf[i + 3] == 'i') {
      break;
    }
    i++;
  }

  uint32_t end_json = i;
#ifdef DEBUG
  printf("end json index: %zu\n", i);
#endif

  uint32_t removed_char = 0;
  while (buf[i] != ')') {
    removed_char++;
    i++;
  }

  removed_char++;
  i++;

#ifdef DEBUG
  printf("'%d' '%d' '%d' '%d'\n", buf[chunk_size_index], buf[chunk_size_index+1], buf[chunk_size_index+2], buf[chunk_size_index+3]);
#endif

  int32_t chunk_size = (buf[chunk_size_index + 3] << 0)
                     | (buf[chunk_size_index + 2] << 8)
                     | (buf[chunk_size_index + 1] << 16)
                     | (buf[chunk_size_index + 0] << 24);
#ifdef DEBUG
  printf("chunk_size: %d\n", chunk_size);
#endif

  // move back the data and update the chunk size
  memmove(buf + end_json, buf + end_json + removed_char, fsize - end_json - removed_char);

  chunk_size -= removed_char;

#ifdef DEBUG
  printf("new chunk size: %d\n", chunk_size);
#endif

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
