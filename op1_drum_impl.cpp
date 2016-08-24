#include <cstring>
#include "sndfile.h"
#include "json.hpp"

#include "op1.h"

using json = nlohmann::json;
using namespace std;

bool g_logging_enabled;

struct audio_file
{
  audio_file()
  {
    PodZero(info);
  }

  SF_INFO info;
  vector<int16_t> data;
};

struct op1_drum
{
  op1_drum()
  {
    end_times.fill(0);
    enveloppe = { 0, 8192, 0, 8192, 0, 0, 0, 0 };
    fx_params.fill(8000);
    lfo_params.fill(16000);
    pitches.fill(OP1_PITCH_CENTER);
    playback_direction.fill(OP1_PLAYBACK_FORWARD);
    playmode.fill(OP1_PLAYMODE_ONE_SHOT);
    start_times.fill(0);
    volumes.fill(OP1_VOLUME_FLAT);
  }

  vector<audio_file> audio_samples;

  array<int, 24> end_times;
  array<int, 24> pitches;
  array<int, 24> playback_direction;
  array<int, 24> playmode;
  array<int, 24> start_times;
  array<int, 24> volumes;
  array<int, 8> enveloppe;
  array<int, 8> fx_params;
  array<int, 8> lfo_params;

  const char * fx_type;
  const char * lfo_type;

  int fx_active;
  int lfo_active;
};

namespace {
uint32_t frame_to_op1_time(uint32_t frame)
{
  // Maximum amount of data for a drum sample on an op-1
  const int BYTES_IN_12_SECS = 44100 * 2 * 12;
  const int OP1_DRUMKIT_END = 0x7FFFFFFE;
  return OP1_DRUMKIT_END / BYTES_IN_12_SECS * frame * sizeof(uint16_t);
}
}


int op1_sample_load(const char * file_name, audio_file ** sample)
{
  SF_INFO info;

  ENSURE_VALID(file_name);
  ENSURE_VALID(sample);

  *sample = new audio_file;

  PodZero(info);

  SNDFILE* file = sf_open(file_name, SFM_READ, &info);
  if (!file) {
    return OP1_ERROR;
  }

  LOG("%s - rate: %d - frame count: %lld\n", file_name, info.samplerate, info.frames);

  size_t samples = info.frames * info.channels;
  (*sample)->info = info;
  (*sample)->data.resize(samples);

  sf_count_t count = sf_read_short(file, &((*sample)->data[0]), info.frames);
  if (count != info.frames) {
    WARN("Unexpected number of frames.");
  }

  int rv = sf_close(file);
  if (rv != 0) {
    return OP1_ERROR;
  }

  return OP1_SUCCESS;
}

int op1_sample_get_data(audio_file * sample, int16_t ** data, size_t * frame_count)
{
  ENSURE_VALID(sample);
  ENSURE_VALID(data);
  ENSURE_VALID(frame_count);

  if (!sample->data.size()) {
    return OP1_ERROR;
  }

  *data = &(sample->data[0]);
  *frame_count = sample->data.size();

  return OP1_SUCCESS;
}

int op1_sample_get_length(audio_file * sample, size_t * frame_count)
{
  ENSURE_VALID(sample);
  ENSURE_VALID(frame_count);

  if (!sample->data.size()) {
    return OP1_ERROR;
  }

  *frame_count = sample->data.size();

  return OP1_SUCCESS;
}

int op1_sample_get_rate(audio_file * sample, int * rate)
{
  ENSURE_VALID(sample);
  ENSURE_VALID(rate);

  *rate = sample->info.samplerate;

  return OP1_SUCCESS;
}

int op1_sample_destroy(audio_file * sample)
{
  delete sample;

  return OP1_SUCCESS;
}

int op1_drum_init(op1_drum ** ctx)
{
  ENSURE_VALID(ctx);

  *ctx = new op1_drum;

  return OP1_SUCCESS;
}

int op1_drum_destroy(op1_drum * ctx)
{
  ENSURE_VALID(ctx);

  delete ctx;

  return OP1_SUCCESS;
}

int op1_drum_write(op1_drum * ctx, const char * file_name)
{
  ENSURE_VALID(ctx);
  ENSURE_VALID(file_name);

  std::array<int, 24> converted_start;
  std::array<int, 24> converted_end;

  bool start_or_end_arrays_set = false;

  for (uint32_t i = 0; i < 24; i++) {
    if (ctx->start_times[i] != 0 || ctx->end_times[i] != 0) {
      start_or_end_arrays_set = true;
    }
  }

  if (!start_or_end_arrays_set) {
    int acc = 0;
    // compute start and end time
    for (uint32_t i = 0; i < ctx->audio_samples.size(); i++) {
      size_t sample_count;
      op1_sample_get_length(&ctx->audio_samples[i], &sample_count);

      converted_start[i] = acc;
      acc += sample_count;
      converted_end[i] = acc + 1;
    }

    for (uint32_t i = ctx->audio_samples.size(); i < 24; i++) {
      converted_start[i] = converted_start[ctx->audio_samples.size() - 1];
      converted_end[i] = converted_end[ctx->audio_samples.size() - 1];
    }
  } else {
    converted_start = ctx->start_times;
    converted_end = ctx->end_times;
  }

  for (uint32_t i = 0; i < 24; i++) {
    converted_start[i] = frame_to_op1_time(converted_start[i]);
    converted_end[i] = frame_to_op1_time(converted_end[i]);
  }

  // make string
  json j;

  j["drum_version"] = 1;
  j["type"] = "drum";
  j["name"] = "user";
  j["octave"] = 0;
  j["pitch"] = ctx->pitches;
  j["start"] = converted_start;
  j["end"] = converted_end;
  j["playmode"] = ctx->playmode;
  j["reverse"] = ctx->playback_direction;
  j["volume"] = ctx->volumes;
  j["dyna_env"] = ctx->enveloppe;
  j["fx_active"] = ctx->fx_active;
  j["fx_type"] = ctx->fx_type;
  j["fx_params"] = ctx->fx_params;
  j["lfo_active"] = ctx->lfo_active;
  j["lfo_type"] = ctx->lfo_type;
  j["lfo_params"] = ctx->lfo_params;

  string serialized = j.dump();

  LOG("json chunk: %s\n", serialized.c_str());

  int rate = ctx->audio_samples[0].info.samplerate;
  for (int i = 1; i < ctx->audio_samples.size(); i++) { 
    if (rate != ctx->audio_samples[i].info.samplerate) {
      return OP1_ERROR;
    }
  }

  // write aiff
  const char * temp_file = "op1-drum-temp.aiff";
  SF_INFO info;
  info.samplerate = rate;
  info.channels = 1; // drums are mono
  info.format = SF_FORMAT_AIFF | SF_FORMAT_PCM_16 | SF_ENDIAN_BIG;
  SNDFILE* outfile = sf_open(temp_file, SFM_WRITE, &info);
  if (!outfile) {
    FATAL("Could not open file for writing");
  }

  // set string
  sf_set_string(outfile, SF_STR_SOFTWARE, serialized.c_str());

  for (int32_t i = 0; i < ctx->audio_samples.size(); i++) {
    int16_t * samples;
    size_t sample_count;
    op1_sample_get_data(&(ctx->audio_samples[i]), &samples, &sample_count);

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

  int rv = sf_close(outfile);
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
  f = fopen(file_name, "w");
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
  return OP1_SUCCESS;
}

int op1_drum_add_sample(op1_drum * ctx, audio_file * file)
{
  ENSURE_VALID(ctx);
  ENSURE_VALID(file);

  ctx->audio_samples.push_back(*file);

  return OP1_SUCCESS;
}

int op1_drum_set_fx(op1_drum * ctx, const char * fx)
{
  ENSURE_VALID(ctx);
  ENSURE_VALID(fx);

  vector<string> valid_effects = {
    "cwo", "delay", "grid", "nitro", "phone", "punch", "spring"
  };

  if (find(valid_effects.begin(), valid_effects.end(), fx) == valid_effects.end()) {
    return OP1_ERROR;
  }

  ctx->fx_type = fx;

  return OP1_SUCCESS;
}

int op1_drum_set_fx_active(op1_drum * ctx, int active)
{
  ENSURE_VALID(ctx);

  ctx->fx_active = active;

  return OP1_SUCCESS;
}

int op1_drum_set_fx_params(op1_drum * ctx, int params[8])
{
  ENSURE_VALID(ctx);

  ArrayCopy(ctx->fx_params, params);

  return OP1_SUCCESS;
}

int op1_drum_set_lfo(op1_drum * ctx, const char * lfo)
{
  ENSURE_VALID(ctx);
  ENSURE_VALID(lfo);

  vector<string> valid_lfo = {
    "bend", "crank", "element", "midi", "random", "tremolo", "value"
  };

  if (find(valid_lfo.begin(), valid_lfo.end(), lfo) == valid_lfo.end()) {
    return OP1_ERROR;
  }

  ctx->lfo_type = lfo;

  return OP1_SUCCESS;
}

int op1_drum_set_lfo_active(op1_drum * ctx, int active)
{
  ENSURE_VALID(ctx);

  ctx->lfo_active = active;

  return OP1_SUCCESS;
}

int op1_drum_set_lfo_params(op1_drum * ctx, int params[8])
{
  ENSURE_VALID(ctx);

  ArrayCopy(ctx->lfo_params, params);

  return OP1_SUCCESS;
}

int op1_drum_set_playmode(op1_drum * ctx, int params[24])
{
  ENSURE_VALID(ctx);

  ArrayCopy(ctx->playmode, params);

  return OP1_SUCCESS;
}

int op1_drum_set_playback_direction(op1_drum * ctx, int params[24])
{
  ENSURE_VALID(ctx);

  ArrayCopy(ctx->playback_direction, params);

  return OP1_SUCCESS;
}

int op1_drum_set_enveloppe(op1_drum * ctx, int enveloppe[8])
{
  ENSURE_VALID(ctx);

  ArrayCopy(ctx->enveloppe, enveloppe);

  return OP1_SUCCESS;
}

int op1_drum_set_pitches(op1_drum * ctx, int pitches[24])
{
  ENSURE_VALID(ctx);

  ArrayCopy(ctx->pitches, pitches);

  return OP1_SUCCESS;
}

int op1_drum_set_volumes(op1_drum * ctx, int volumes[24])
{
  ENSURE_VALID(ctx);

  ArrayCopy(ctx->volumes, volumes);

  return OP1_SUCCESS;
}

int op1_drum_set_start_times(op1_drum * ctx, int start_times[24])
{
  ENSURE_VALID(ctx);

  ArrayCopy(ctx->start_times, start_times);

  return OP1_SUCCESS;
}

int op1_drum_set_end_times(op1_drum * ctx, int end_times[24])
{
  ENSURE_VALID(ctx);

  ArrayCopy(ctx->end_times, end_times);

  return OP1_SUCCESS;
}

