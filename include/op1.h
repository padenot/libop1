#ifndef OP1_H_
#define OP1_H_

/** @file
 *     The <tt>libop1</tt> C API. */


#include <stdlib.h>
#include <stdint.h>

#include "op1_common.h"

#ifndef EMSCRIPTEN_KEEPALIVE
#define EMSCRIPTEN_KEEPALIVE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * An opaque struct that represents an audio file.
 */
struct audio_file;
/**
 * An opaque struct that represents a drum sample being created.
 */
struct op1_drum;

/**
 * An enum that represents all the error codes the library can return.
 */
enum {
  OP1_SUCCESS = 0, ///< The API call succeeded.
  OP1_ERROR = -1,  ///< Generic error
  OP1_ARGUMENT_ERROR = -2 ///< One or more arguments passed was invalid.
};

/**
 * Special values to pass to `op1_drum_set_playmode`.
 *
 * @see op1_drum_set_playmode
 */
enum OP1_PLAYMODE {
  /**
   * Play the sample until the key is released.
   */
  OP1_PLAYMODE_FORWARD = 0x1000,
  /**
   * Play the sample once.
   */
  OP1_PLAYMODE_ONE_SHOT = 0x2000,
  /**
   * Play the sample in a loop untile the key is released.
   */
  OP1_PLAYMODE_LOOP = 0x4800
};

/**
 * The direction of playback, to pass to op1_drum_set_playback_direction.
 *
 * @see op1_drum_set_playback_direction
 */
enum OP1_PLAYBACK_DIRECTION {
  /**
   * Play the sample forward.
   */
  OP1_PLAYBACK_FORWARD = 0x2000,
  /**
   * Play the sample in reverse.
   */
  OP1_PLAYBACK_REVERSE = 0x4800
};

/**
 * The base volume for a sample, i.e. a gain of 1.0.
 */
enum OP1_VOLUME {
  OP1_VOLUME_FLAT = 0x2000
};

/**
 * The base pitch for a sample, so that it plays unmodified.
 */
enum OP1_PITCH {
  OP1_PITCH_CENTER = 0
};

/**
 * Load a sample from a file name. All the file type supported by libsndfile are
 * supported.
 *
 * @param file_name A file name, has to be non-null.
 * @param output An opaque handle to an audio file.
 *
 * @returns an error code in case of error, OP1_SUCCESS otherwise.
 */
int EMSCRIPTEN_KEEPALIVE op1_sample_load(const char * file_name, audio_file ** output);

/**
 * Load a sample from a buffer. All the file type supported by libsndfile are
 * supported.
 *
 * @param data A buffer containing raw audio file data.
 * @param length The size of the buffer.
 * @param output An opaque handle to an audio file.
 *
 * @returns an error code in case of error, OP1_SUCCESS otherwise.
 */
int EMSCRIPTEN_KEEPALIVE op1_sample_load_buffer(const uint8_t * data, size_t length, audio_file ** output);

/**
 * Destroy a sample previously loaded with `op1_sample_load`.
 *
 * @see op1_sample_load
 *
 * @param sample The sample to destroy, has to be non-null.
 *
 * @returns an error code in case of error, OP1_SUCCESS otherwise.
 */
int EMSCRIPTEN_KEEPALIVE op1_sample_destroy(audio_file * sample);

/**
 * Get raw data, as a buffer of int16_t representing the mono file.
 *
 * @param sample An opaque handle to an audio file, has to be non-null.
 * @param data A pointer to a valid int16_t*, set to the raw data.
 * @param frame_count Filled with the number of frames of this file.
 *
 * @returns an error code in case of error, OP1_SUCCESS otherwise.
 */
int EMSCRIPTEN_KEEPALIVE op1_sample_get_data(audio_file * sample, int16_t ** data, size_t * frame_count);

/**
 * Get the sample-rate of the file.
 *
 * @param sample An opaque handle to an audio file, has to be non-null.
 * @param rate Filled with the sample-rate of this file.
 *
 * @returns an error code in case of error, OP1_SUCCESS otherwise.
 */
int EMSCRIPTEN_KEEPALIVE op1_sample_get_rate(audio_file * sample, int * rate);
/**
 * Get the length, in samples, of this file.
 *
 * @param sample An opaque handle to an audio file, has to be non-null.
 * @param frame_count Filled with the length, in samples, of this file.
 *
 * @returns an error code in case of error, OP1_SUCCESS otherwise.
 */
int EMSCRIPTEN_KEEPALIVE op1_sample_get_length(audio_file * sample, size_t * frame_count);

/** Initialize a new `op1_drum` context.
 *
 * @param ctx A pointer to a valid pointer to an `op1_drum`.
 *
 * @returns an error code in case of error, OP1_SUCCESS otherwise.
 */
int EMSCRIPTEN_KEEPALIVE op1_drum_init(op1_drum ** ctx);

/** Destroys an `op1_drum` context.
 *
 * @param ctx A pointer to a valid `op1_drum`.
 *
 * @returns an error code in case of error, OP1_SUCCESS otherwise.
 */
int EMSCRIPTEN_KEEPALIVE op1_drum_destroy(op1_drum * ctx);

/** Write the final audio file to disk. If any of `op1_drum_set_start_times` or
 * `op1_drum_set_end_times` have been called with array that are not all zeros,
 * start and end times will be computed and will be the start and end of each
 * sample, with exactly one sample in between.
 *
 * @param ctx A pointer to a valid `op1_drum`.
 * @param file_name A string containing the file name of the file to be written.
 *
 * @returns an error code in case of error, OP1_SUCCESS otherwise.
 */
int EMSCRIPTEN_KEEPALIVE op1_drum_write(op1_drum * ctx, const char * file_name);

/** Write the final audio file to a buffer. If any of `op1_drum_set_start_times`
 * or `op1_drum_set_end_times` have been called with array that are not all
 * zeros, start and end times will be computed and will be the start and end of
 * each sample, with exactly one sample in between.
 *
 * @param ctx A pointer to a valid `op1_drum`.
 * @param output A pointer to an array containing the output data.
 * @param length Filled in with the length of the array.
 *
 * @returns an error code in case of error, OP1_SUCCESS otherwise.
 */
int EMSCRIPTEN_KEEPALIVE op1_drum_write_buffer(op1_drum * ctx, uint8_t ** output, size_t * length);

/** Add a sample to an `op1_drum` context.
 *
 * @param ctx A pointer to a valid `op1_drum`.
 * @param file A pointer to a valid `audio_file`.
 *
 * @see op1_sample_load
 *
 * @returns an error code in case of error, OP1_SUCCESS otherwise.
 */
int EMSCRIPTEN_KEEPALIVE op1_drum_add_sample(op1_drum * ctx, audio_file * file);
/**
 * Set the effect to be used for this drum sample.
 *
 * @param ctx A pointer to a valid `op1_drum`.
 * @param fx A string containing the effect to set. Valid values are
 * "cwo", "delay", "grid", "nitro", "phone", "punch", "spring".
 *
 * @returns an error code in case of error, OP1_SUCCESS otherwise.
 */
int EMSCRIPTEN_KEEPALIVE op1_drum_set_fx(op1_drum * ctx, const char * fx);

/**
 * Set wether the effect is active by default.
 *
 * @param ctx A pointer to a valid `op1_drum`.
 * @param active 0 for inactive, anything else for active.
 *
 * @returns an error code in case of error, OP1_SUCCESS otherwise.
 */
int EMSCRIPTEN_KEEPALIVE op1_drum_set_fx_active(op1_drum * ctx, int active);

/**
 * Set the effect parameters, as a array of 8 integers.
 *
 * @param ctx A pointer to a valid `op1_drum`.
 * @param params An array of parameters.
 *
 * @returns an error code in case of error, OP1_SUCCESS otherwise.
 */
int EMSCRIPTEN_KEEPALIVE op1_drum_set_fx_params(op1_drum * ctx, int params[8]);

/**
 * Set the LFO type to be used for this drum sample.
 *
 * @param ctx A pointer to a valid `op1_drum`.
 * @param fx A string containing the LFO type to set. Valid values are
 * "bend", "crank", "element", "midi", "random", "tremolo", "value".
 *
 * @returns an error code in case of error, OP1_SUCCESS otherwise.
 */
int EMSCRIPTEN_KEEPALIVE op1_drum_set_lfo(op1_drum * ctx, const char * fx);

/**
 * Set wether the LFO is active by default.
 *
 * @param ctx A pointer to a valid `op1_drum`.
 * @param active 0 for inactive, anything else for active.
 *
 * @returns an error code in case of error, OP1_SUCCESS otherwise.
 */
int EMSCRIPTEN_KEEPALIVE op1_drum_set_lfo_active(op1_drum * ctx, int active);

/**
 * Set the LFO parameters, as a array of 8 integers.
 *
 * @param ctx A pointer to a valid `op1_drum`.
 * @param params An array of parameters.
 *
 * @returns an error code in case of error, OP1_SUCCESS otherwise.
 */
int EMSCRIPTEN_KEEPALIVE op1_drum_set_lfo_params(op1_drum * ctx, int params[8]);

/**
 * Set the play-mode for each samples.
 *
 * @param ctx A pointer to a valid `op1_drum`.
 * @param params An array of parameters that have a value specified by
 * `OP1_PLAYMODE`.
 *
 * @see OP1_PLAYMODE
 *
 * @returns an error code in case of error, OP1_SUCCESS otherwise.
 */
int EMSCRIPTEN_KEEPALIVE op1_drum_set_playmode(op1_drum * ctx, int params[24]);

/**
 * Set the playback direction for each samples.
 *
 * @param ctx A pointer to a valid `op1_drum`.
 * @param params An array of parameters that have a value specified by
 * `OP1_PLAYBACK_DIRECTION`.
 *
 * @see OP1_PLAYBACK_DIRECTION
 *
 * @returns an error code in case of error, OP1_SUCCESS otherwise.
 */
int EMSCRIPTEN_KEEPALIVE op1_drum_set_playback_direction(op1_drum * ctx, int params[24]);

/**
 * Set the enveloppe for this drum sample.
 *
 * @param ctx A pointer to a valid `op1_drum`.
 * @param enveloppe An array of parameters.
 *
 * @returns an error code in case of error, OP1_SUCCESS otherwise.
 */
int EMSCRIPTEN_KEEPALIVE op1_drum_set_enveloppe(op1_drum * ctx, int enveloppe[8]);

/**
 * Set the pitches for each samples.
 *
 * @param ctx A pointer to a valid `op1_drum`.
 * @param pitches An array of parameters. `OP1_PITCH_CENTER` makes it so that the
 * sample plays unmodified.
 *
 * @returns an error code in case of error, OP1_SUCCESS otherwise.
 */
int EMSCRIPTEN_KEEPALIVE op1_drum_set_pitches(op1_drum * ctx, int pitches[24]);

/**
 * Set the volume for each samples.
 *
 * @param ctx A pointer to a valid `op1_drum`.
 * @param volumes An array of parameters. `OP1_VOLUME_FLAT` makes it so that the
 * sample plays unmodified.
 *
 * @returns an error code in case of error, OP1_SUCCESS otherwise.
 */
int EMSCRIPTEN_KEEPALIVE op1_drum_set_volumes(op1_drum * ctx, int volumes[24]);

/**
 * Set the start time for each samples.
 *
 * @param ctx A pointer to a valid `op1_drum`.
 * @param start_times An array of start times, in frames.
 *
 * @returns an error code in case of error, OP1_SUCCESS otherwise.
 */
int EMSCRIPTEN_KEEPALIVE op1_drum_set_start_times(op1_drum * ctx, int start_times[24]);

/**
 * Set the end time for each samples.
 *
 * @param ctx A pointer to a valid `op1_drum`.
 * @param end_times An array of end times, in frames.
 *
 * @returns an error code in case of error, OP1_SUCCESS otherwise.
 */
int EMSCRIPTEN_KEEPALIVE op1_drum_set_end_times(op1_drum * ctx, int end_times[24]);

#ifdef __cplusplus
}
#endif

#endif // OP1_H_
