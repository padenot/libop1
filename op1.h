#ifndef OP1_H_
#define OP1_H_

#include <stdlib.h>
#include <stdint.h>

#include "op1_common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct audio_file;
struct op1_drum;

enum {
  OP1_SUCCESS = 0,
  OP1_ERROR = -1,
  OP1_ARGUMENT_ERROR = -2
};

enum {
  OP1_PLAYMODE_FORWARD = 0x1000,
  OP1_PLAYMODE_ONE_SHOT = 0x2000,
  OP1_PLAYMODE_LOOP = 0x4800
};

enum {
  OP1_PLAYBACK_FORWARD = 0x2000,
  OP1_PLAYBACK_REVERSE = 0x4800
};

enum {
  OP1_VOLUME_FLAT = 0x2000
};

enum {
  OP1_PITCH_CENTER = 0
};

int op1_sample_load(const char * file_name, audio_file ** output);
int op1_sample_destroy(audio_file * sample);
int op1_sample_get_data(audio_file * sample, int16_t ** data, size_t * frame_count);
int op1_sample_get_rate(audio_file * sample, int * rate);
int op1_sample_get_length(audio_file * sample, size_t * frame_count);

int op1_drum_init(op1_drum ** ctx);
int op1_drum_destroy(op1_drum * ctx);
int op1_drum_write(op1_drum * ctx, const char * file_name);
int op1_drum_add_sample(op1_drum * ctx, audio_file * file);
int op1_drum_set_fx(op1_drum * ctx, const char * fx);
int op1_drum_set_fx_active(op1_drum * ctx, int active);
int op1_drum_set_fx_params(op1_drum * ctx, int params[8]);
int op1_drum_set_lfo(op1_drum * ctx, const char * fx);
int op1_drum_set_lfo_active(op1_drum * ctx, int active);
int op1_drum_set_lfo_params(op1_drum * ctx, int params[8]);
int op1_drum_set_playmode(op1_drum * ctx, int params[24]);
int op1_drum_set_playback_direction(op1_drum * ctx, int params[24]);
int op1_drum_set_enveloppe(op1_drum * ctx, int enveloppe[8]);
int op1_drum_set_pitches(op1_drum * ctx, int pitches[24]);
int op1_drum_set_volumes(op1_drum * ctx, int volumes[24]);
int op1_drum_set_start_times(op1_drum * ctx, int start_times[24]);
int op1_drum_set_end_times(op1_drum * ctx, int end_times[24]);

#ifdef __cplusplus
}
#endif

#endif // OP1_H_
