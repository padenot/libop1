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
  op1_drum();
  virtual ~op1_drum();
};

int op1_sample_load(const char * file_name, audio_file ** sample)
{
  SF_INFO info;

  if (!sample || !file_name) {
    return OP1_ERROR;
  }

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
  if (!sample || !data || !frame_count) {
    return OP1_ERROR;
  }

  if (!sample->data.size()) {
    return OP1_ERROR;
  }

  *data = &(sample->data[0]);
  *frame_count = sample->data.size();

  return OP1_SUCCESS;
}

int op1_sample_get_length(audio_file * sample, size_t * frame_count)
{
  if (!sample || !frame_count) {
    return OP1_ERROR;
  }
  if (!sample->data.size()) {
    return OP1_ERROR;
  }

  *frame_count = sample->data.size();

  return OP1_SUCCESS;
}

int op1_sample_get_rate(audio_file * sample, int * rate)
{
  if (!sample || !rate) {
    return OP1_ERROR;
  }
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
  assert(*ctx);
  return OP1_SUCCESS;
}

int op1_drum_destroy(op1_drum * ctx)
{
  return OP1_SUCCESS;
}

int op1_drum_write(const char * file_name)
{
  return OP1_SUCCESS;
}

int op1_drum_add_sample(op1_drum * ctx, audio_file * file)
{
  return OP1_SUCCESS;
}

int op1_drum_set_fx(op1_drum * ctx, const char * fx)
{
  return OP1_SUCCESS;
}

int op1_drum_set_fx_active(op1_drum * ctx, int active)
{
  return OP1_SUCCESS;
}

int op1_drum_set_fx_params(op1_drum * ctx, int params[8])
{
  return OP1_SUCCESS;
}

int op1_drum_set_lfo(op1_drum * ctx, int active)
{
  return OP1_SUCCESS;
}

int op1_drum_set_lfo_active(op1_drum * ctx, const char * fx)
{
  return OP1_SUCCESS;
}

int op1_drum_set_lfo_params(op1_drum * ctx, int params[8])
{
  return OP1_SUCCESS;
}

int op1_drum_set_playmode(op1_drum * ctx, int params[24])
{
  return OP1_SUCCESS;
}

int op1_drum_set_playback_direction(op1_drum * ctx, int params[24])
{
  return OP1_SUCCESS;
}

int op1_drum_set_enveloppe(op1_drum * ctx, int enveloppe[8])
{
  return OP1_SUCCESS;
}

int op1_drum_set_pitches(op1_drum * ctx, int pitches[24])
{
  return OP1_SUCCESS;
}

int op1_drum_set_volumes(op1_drum * ctx, int volumes[24])
{
  return OP1_SUCCESS;
}

int op1_drum_set_start_time(op1_drum * ctx, int start_times[24])
{
  return OP1_SUCCESS;
}

int op1_drum_set_endtime(op1_drum * ctx, int end_times[24])
{
  return OP1_SUCCESS;
}

