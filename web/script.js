window.onload = function() {
$$ = document.querySelectorAll.bind(document);
$ = document.querySelector.bind(document);
var root = document.querySelector(".container");

var ac = new AudioContext();
// Initially suspend the AudioContext, no need to drain the user battery.
ac.suspend();
// Holds number of currently playing sound. When this is zero, the AudioContext
// gets suspended.
var playing = 0;

// A drum hit
function Sample(file)
{
  var self = this;
  var fr = new FileReader();
  // This constructs a Samples, keeps the raw data around, but also decodes it
  // to a Web Audio API AudioBuffer.
  // This resolves when the Sample is ready: parsed and decoded.
  this.parseAndDecodePromise = new Promise(function(resolve, reject) {
    fr.onload = function(e) {
    // The raw buffer, i.e. the file that the user gave
      self.raw_data = e.target.result;
      var off = new OfflineAudioContext(1, 48000, 48000);
      off.decodeAudioData(self.get_raw_data()).then((data) => {
        // The decoded buffer, in the form of a Web Audio API AudioBuffer
        self.audio_buffer = data;
        resolve();
      }, (err) => {
        alert("Could not decode file: " + err);
        reject();
      });
    }
    fr.readAsArrayBuffer(file);
  });
}

// This resolves when everything is ready: the file has been put in an array
// buffer, and it has been decoded into an AudioBuffer.
Sample.prototype.get_ready_promise = function() {
  return this.parseAndDecodePromise;
}

// Return a new copy of the raw data. Possibly wasteful but this is going to get
// detached often...
Sample.prototype.get_raw_data = function() {
  return this.raw_data.slice(0);
}

// This draws a waveform in the canvas passed as argument. It chunks the file
// and estimates the energy per chunk so that the waveform look pretty, like
// SoundCloud.
Sample.prototype.draw = function(cvs) {
  var factor = this.audio_buffer.length / cvs.width;
  var c = cvs.getContext("2d");
  var b = this.audio_buffer.getChannelData(0);
  c.fillStyle = "#000";
  function rms(buf, offset, len) {
    var rms = 0;
    if (buf.length < offset + len) {
      len = buf.length - offset;
    }
    if (len == 0) {
      return 0;
    }
    for (var i = 0; i < len; i++) {
      var v = buf[offset + i];
      rms += Math.sqrt(v * v);
    }
    rms /= len;
    return rms;
  }

  var j = 0;
  var max = 0;
  // rms by chunk to determine a normalization factor
  for (var i = 0; i < b.length; i=Math.floor(i+factor)) {
    var rmsvalue = rms(b, i, factor);
    max = Math.max(max, rmsvalue);
  }
  var boost = (0.5 / max) * cvs.height;
  for (var i = 0; i < b.length; i=Math.floor(i+factor)) {
    var rmsvalue = rms(b, i, factor) * boost;
    rmsvalue = Math.max(1.0, rmsvalue);
    c.fillStyle = "rgba(0, 0, 0, 1.0)";
    c.fillRect(j++, cvs.height / 1.3, 1.5, -rmsvalue * 1.5);
    c.fillStyle = "rgba(0, 0, 0, 0.4)";
    c.fillRect(j++, cvs.height / 1.3, 1.5, +rmsvalue * 0.5);
  }
}

// This plays the sample using the global AudioContext.
Sample.prototype.play = function() {
  if (!this.audio_buffer) {
    console.log("Could not play " + this + ": not decoded.");
    return;
  }
  // try to save CPU by not letting the AudioContext run when not needed.
  ac.resume().then(() => {
    source = ac.createBufferSource();
    source.buffer = this.audio_buffer;
    source.connect(ac.destination);
    source.start();
    playing++;
    source.onended = () => {
      playing--;
      if (!playing) {
        ac.suspend();
      }
    }
  });
}

function export_aiff()
{
  var sampleSlots = document.querySelectorAll(".sample");
  var samples = [];
  for (var i = 0; i < sampleSlots.length; i++) {
    if (sampleSlots[i].sample) {
      samples.push(sampleSlots[i].sample)
    }
  }
  init();
  decodeAndRender(samples);
}

function repaint(row) {
  var canvas_wrap_width = row.querySelector(".cvs-wrap");
  var cvs = row.querySelector("canvas");
  var ctx = cvs.getContext('2d');
  cvs.width = canvas_wrap_width.getBoundingClientRect().width;
  ctx.clearRect(0, 0, cvs.width, cvs.height);
  if (row.sample) {
    row.sample.draw(cvs);
  }
}

function addFiles(files, droppedOn) {
  var samplesSlot = document.querySelectorAll(".sample");
  if (!droppedOn) {
    // find first unused row
    for (var i = 0; i < samplesSlot.length; i++) {
      if (!samplesSlot[i].rawsample) {
        droppedOn = samplesSlot[i];
        break;
      }
    }
  }
  var droppedCount = files.length;
  var thisrow = Array.prototype.indexOf.call(samplesSlot, droppedOn);
  var fileDroppedIndex = 0;
  var promises = [];
  for (var i = thisrow; i < samplesSlot.length && droppedCount > 0; i++) {
    var button = samplesSlot[i].querySelector("button");
    button.disabled = false;
    samplesSlot[i].sample = new Sample(files[fileDroppedIndex++]);
    promises.push(samplesSlot[i].sample.get_ready_promise());
    droppedCount--;
  }
  Promise.all(promises).then(() => {
    for(var i = 0; i < samplesSlot.length; i++) {
      repaint(samplesSlot[i]);
    }
  });
}

function el(tag, classes, id, inner) {
  var elem = document.createElement(tag);
  if (classes) {
    classes.forEach(e => elem.classList.add(e));
  }
  if (id) {
    elem.id = id;
  }
  if (inner) {
    console.log(inner);
    elem.innerHTML = inner;
  }
  return elem;
}

function oneSample() {
  var sample = document.createElement("div");
  sample.classList.add("sample");

  var cvswrap = el("div", ["cvs-wrap"]);;
  var cvs = document.createElement("canvas");
  cvs.height = "40";
  var playmode = document.createElement("select");
  playmode.classList.add("flat");
  ["normal", "oneshot", "reverse"].forEach(function(e) {
    var el = document.createElement("option");
    el.innerHTML = e;
    playmode.appendChild(el);
  });
  var direction = document.createElement("select");
  direction.classList.add("flat");
  ["forward", "reverse"].forEach(function(e) {
    var el = document.createElement("option");
    el.innerHTML = e;
    direction.appendChild(el);
  });

  var playSample = document.createElement("button");
  playSample.classList.add("playbutton");
  playSample.classList.add("flat");
  playSample.innerHTML = "&#9654;";
  playSample.disabled = true;

  playSample.onclick = function() {
    if (!sample.sample) {
      throw "no sample?!";
    }
    sample.sample.play();
  }

  var filename = el("div", ["sample-name"]);

  cvswrap.append(cvs);
  cvswrap.append(filename);
  sample.appendChild(cvswrap);
  sample.appendChild(playmode);
  sample.appendChild(direction);
  sample.appendChild(playSample);

  sample.addEventListener("dragover", function(e) {
    e.preventDefault();
    e.stopPropagation();
    var dropped = e.dataTransfer.files;
    var droppedCount = dropped.length;
    var droppedOn = e.target;
    while (!droppedOn.classList.contains("sample")) {
      droppedOn = droppedOn.parentNode;
    }
    e.target.classList.add("hover");
  });
  sample.addEventListener("dragleave", function(e) {
    e.preventDefault();
    e.stopPropagation();
    var dropped = e.dataTransfer.files;
    var droppedCount = dropped.length;
    var droppedOn = e.target;
    while (!droppedOn.classList.contains("sample")) {
      droppedOn = droppedOn.parentNode;
    }
    e.target.classList.remove("hover");
  });
  sample.addEventListener("drop", function(e) {
    e.preventDefault();
    e.stopPropagation();
    var droppedOn = e.target;
    while (!droppedOn.classList.contains("sample")) {
      droppedOn = droppedOn.parentNode;
    }
    e.target.classList.remove("hover");
    // fill the slots from the one this was dropped on
    addFiles(e.dataTransfer.files, droppedOn);
  });

  return sample;
}

var samples = $$('#samples');
for (var i = 0; i < 24; i++) {
  root.appendChild(oneSample());
}

$("#sample-input").onchange = function(e) {
  var files = e.target.files;
  addFiles(files);
}

$("#export").addEventListener("click", export_aiff);

}
