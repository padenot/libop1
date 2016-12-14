function op1web_sample_load_buffer(typedArray) {
  var ptr_to_sample_ptr = Module._malloc(4);

  var buf = Module._malloc(typedArray.length*typedArray.BYTES_PER_ELEMENT);
  Module.HEAPU8.set(typedArray, buf);

  var rv = Module.ccall('op1_sample_load_buffer',
      'number',
      ['number', 'number', 'number'],
      [buf, typedArray.length, ptr_to_sample_ptr]);

  if (rv != 0) {
    console.log("Could not decode file.");
    return rv;
  }

  Module._free(buf);
  // TODO why can't we `free ptr_to_sample_ptr` ?

  return Module.getValue(ptr_to_sample_ptr, '*');
}

function op1web_sample_get_rate(sample_ptr) {
  var samplerate_ptr = Module._malloc(4);

  rv = Module.ccall('op1_sample_get_rate',
      'number',
      ['number', 'number'],
      [sample_ptr, samplerate_ptr]);

  if (rv != 0) {
    console.log("Could not get samplerate.");
    return rv;
  }
  return Module.getValue(samplerate_ptr, 'i32*');
}

function op1web_sample_get_data(sample_ptr) {
  var data_ptr_to_ptr = Module._malloc(4);
  var int16_ptr = Module._malloc(4);
  var length_ptr = Module._malloc(4);

  rv = Module.ccall('op1_sample_get_data',
                    'number',
                    ['number', 'number', 'number'],
                    [sample_ptr, data_ptr_to_ptr, length_ptr]);

  if (rv != 0) {
    console.log("Could not get sample data.");
    return rv;
  }

  length = Module.getValue(length_ptr, 'i32*');
  int16_ptr = Module.getValue(data_ptr_to_ptr, 'i16*');

  var integer_samples = Module.HEAP16.subarray(int16_ptr, int16_ptr + length);
  var float_samples = new Float32Array(integer_samples.length);

  for (var i = 0; i < float_samples.length; i++) {
    float_samples[i] = integer_samples[i] / (2 << 15);
  }

  Module._free(data_ptr_to_ptr);
  Module._free(int16_ptr);
  Module._free(length_ptr);

  return float_samples;
}

function op1web_drum_init() {
  var drum_init_ptr_ptr = Module._malloc(4);

  rv = Module.ccall('op1_drum_init',
      'number',
      ['number'],
      [drum_init_ptr_ptr]);

  if (rv != 0) {
    console.log("could not create drum ctx");
    return rv;
  }

  return Module.getValue(drum_init_ptr_ptr, '*');
}

function op1web_drum_destroy(drum_ctx) {
  rv = Module.ccall('op1_drum_destroy',
      'number',
      ['number'],
      [drum_ctx]);

  if (rv != 0) {
    console.log("could not destroy drum context");
  }

  return rv;
}

function op1web_drum_add_sample(drum_ctx, sample) {
  rv = Module.ccall('op1_drum_add_sample',
      'number',
      ['number', 'number'],
      [drum_ctx, sample]);

  if (rv != 0) {
    console.log("could not add sample to context");
  }

  return rv;
}

function op1web_drum_write_buffer(drum_ctx) {
  var uint8_ptr_ptr = Module._malloc(4);
  var length_ptr = Module._malloc(4);

  rv = Module.ccall('op1_drum_write_buffer',
                    'number',
                    ['number', 'number', 'number'],
                    [drum_ctx, uint8_ptr_ptr, length_ptr]);

  if (rv != 0) {
    console.log("Could not render buffer.");
  }

  length = Module.getValue(length_ptr, 'i32*');
  uint8_ptr = Module.getValue(uint8_ptr_ptr, 'u8*');

  console.log("length:" + length); 

  return Module.HEAP8.subarray(uint8_ptr, uint8_ptr + length);
}

function init() {
  drum_ctx = op1web_drum_init();

  if (drum_ctx < 0)  {
    console.log("Could not create drum ctx.");
    return 0;
  }
}

/**
 * Convert from float to i16, and downmix to mono if needed.
 */
function audiobuffer_to_i16(audiobuffer)
{
  var len = audiobuffer.length;
  var channels = audiobuffer.numberOfChannels;
  var rv = new Int16Array(len);
  for (var i = 0; i < len; i++) {
    rv[i] = 0;
    for (var c = 0; c < channels; c++) {
      rv[i] += audiobuffer.getChannelData(c)[i] * Math.pow(2, 15);
    }
  }
  return rv;
}

/**
 * samples is an array of files that libsndfile can decode
 */
function decodeAndRender(samples) {
  samples.forEach(function(sample) {
//    var sample_data_i16 = audiobuffer_to_i16(sample);
    var sample_ptr = op1web_sample_load_buffer(new Uint8Array(sample));
    console.log(op1web_sample_get_rate(sample_ptr));

    rv = op1web_drum_add_sample(drum_ctx, sample_ptr);
    if (rv != 0) {
      console.log("Could not add sample to context");
    }
  });

  var result = op1web_drum_write_buffer(drum_ctx);

  var blob = new Blob([result]);
  var url = window.URL.createObjectURL(blob);

  var a = document.createElement("a");
  a.href = url;
  a.download = "drum.aif";
  a.style.display = "none";
  document.body.appendChild(a);

  a.click();
}

function render() {
  var samples = [];

  var files = document.querySelectorAll("div.")
  init();

  fr = new FileReader();
  var count = e.target.files.length;
  for (var i = 0; i < count; i++) {
    fr.readAsArrayBuffer(e.target.files[i]);
    fr.onload = function(e) {
      console.log(e.target.result);
      samples.push(new Uint8Array(e.target.result));
      count--;

      if (count == 0) {
        decodeAndRender();
      }
    }
  }
}

