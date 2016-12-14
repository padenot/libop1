window.onload = function() {
$$ = document.querySelectorAll.bind(document);
$ = document.querySelector.bind(document);
var root = document.querySelector(".container");

var ac = new AudioContext();
ac.suspend();

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

function rms(array) {
  return Math.sqrt(array.reduce((acc, x) => acc + (x * x)) / array.length);
}

function drawWaveform(cvs, sample) {
    var factor = sample.length / cvs.width;
    var c = cvs.getContext("2d");
    var b = sample.getChannelData(0);
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

function repaint(row) {
  var canvas_wrap_width = row.querySelector(".cvs-wrap");
  var cvs = row.querySelector("canvas");
  var ctx = cvs.getContext('2d');
  cvs.width = canvas_wrap_width.getBoundingClientRect().width;
  ctx.clearRect(0, 0, cvs.width, cvs.height);
  row.querySelector(".sample-name").innerHTML = row.rawsample.name;
  if (row.rawsample) {
    var fr = new FileReader();
    fr.onload = function(e) {
      row.sample = e.target.result;
      // copy the data since decodeAudioData will detach
      var datacopy = e.target.result.slice(0);
      var off = new OfflineAudioContext(1, 48000, 48000);
      off.decodeAudioData(datacopy).then((data) => {
        row.audiobuffer = data;
        drawWaveform(cvs, data);
      }).catch((e) => console.log(e));
    }
    fr.readAsArrayBuffer(row.rawsample);
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
  for (var i = thisrow; i < samplesSlot.length && droppedCount > 0; i++) {
    var button = samplesSlot[i].querySelector("button");
    button.disabled = false;
    samplesSlot[i].rawsample = files[fileDroppedIndex++];
    repaint(samplesSlot[i]);
    droppedCount--;
  }
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
    if (!sample.audiobuffer) {
      throw "no sample?!";
    }
    ac.resume();
    var source = ac.createBufferSource();
    source.buffer = sample.audiobuffer;
    source.connect(ac.destination);
    source.start();
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
