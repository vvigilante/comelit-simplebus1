const queryString = window.location.search;
const urlParams = new URLSearchParams(queryString);
var addr = window.location.hostname;
if (urlParams.has('addr')) {
    addr = urlParams.get('addr');
}


var snd = new Audio("data:audio/mp3;base64,/+NIxAAAAAAAAAAAAFhpbmcAAAAPAAAABAAAA2AAgICAgICAgICAgICAgICAgICAgICAgICAoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoODg4ODg4ODg4ODg4ODg4ODg4ODg4ODg4OD/////////////////////////////////AAAACkxBTUUzLjEwMAQoAAAAAAAAAAAVCCQC8SEAAZoAAANg6nK+XgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA/+NIxAAGiALTAUAAAYmHQG0AAPDw8PSAAAAf4eH/EZzD/j///0PHqrykRzMMuMsLsIk/zKn1hb8M7ePf7xxx1lrdWt/7xrMqMsMTA2FBYfqImHzZrErzpMoDVAXKcpygjpa9Ih6njjkav81346/sM00ajVWlhqU2p69b7h/////0ugoFJBRekGUW29W7BlTdu6CrjcNxd+O9xo5U/z/Vf/CkkTvTkEY1qsM8l5gyEUfYEEzgVQx7jEoeroXH//CAEJBuqZPSHKnS2qZRRmTXpSm/HnjaSGFAyCv7pvCQNRUFakxBTUUzLjEwMKqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqTEFNRTMu/+MYxPgYgULPAZrAATEwMKqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq/+MoxNEV0QrjAZp4AKqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq/+MYxMQAAAP8AcAAAKqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq");

function ringOnce() {
    snd.play();
}

function ring() {
    for (i = 0; i < 5000; i += 1000)
        setTimeout(ringOnce, i);
}

function drawAudio(dataArray, canvas) {
    bufferLength = dataArray.length;
    WIDTH = canvas.width;
    HEIGHT = canvas.height;
    canvasCtx = canvas.getContext('2d');
    canvasCtx.clearRect(0, 0, WIDTH, HEIGHT);
    canvasCtx.fillStyle = 'rgb(50, 50, 50)';
    canvasCtx.fillRect(0, 0, WIDTH, HEIGHT);
    canvasCtx.lineWidth = 2;
    canvasCtx.strokeStyle = 'rgb(0, 255, 0)';
    canvasCtx.beginPath();
    const STEP = 2;
    var sliceWidth = WIDTH * 1.0 * STEP / bufferLength;
    var x = 0;
    for (var i = 0; i < bufferLength; i += STEP) {
        var v = dataArray[i] / 255.0;
        var y = v * HEIGHT;
        if (i === 0) {
            canvasCtx.moveTo(x, y);
        } else {
            canvasCtx.lineTo(x, y);
        }
        x += sliceWidth;
    }
    canvasCtx.stroke();
}

const statestrings = {
    'state1': 'CLEAR',
    'state2': 'CALL_REQ',
    'state3': 'RINGING',
    'state6': 'PICKUP2',
    'state7': 'CONNECTED',
    'state8': 'HANGED'
};
const zeroPad = (num, places) => String(num).padStart(places, '0');
var webSocket1 = null;

class WSSourceNode extends AudioWorkletNode {
    constructor() {
        super(audioCtx, 'ws-receiver');
        this.p = this.port;
    }
}
class WSSinkNode extends AudioWorkletNode {
    constructor() {
        super(audioCtx, 'ws-sender');
        this.port.addEventListener('message', (event) => {
            this.ondata(event);
        });
        this.port.start();
    }
    ondata(event) {
        var d = event['data'];
        if (webSocket1)
            webSocket1.send(d);
        drawAudio(d, document.getElementById('micCanvas'));
    }
}
const SAMPLE_RATE = 8000;
var audioCtx = null;
var wssource = null;
var micsource = null;
var wssink = null;

async function initCtx() {
    if (audioCtx == null) {
        audioCtx = new AudioContext({
            'sampleRate': SAMPLE_RATE
        });
        let blob = new Blob([`
            class CircularBuffer {
                constructor(size) {
                    this.element = [];
                    this.size = size
                    this.length = 0
                    this.front = 0
                    this.back = -1
                }
                isEmpty() {
                    return (this.length == 0)
                }
                enqueue(element) {
                    if (this.length >= this.size) {
                        //throw (new Error("Maximum length exceeded"))
                    } else {
                        this.back = (this.back + 1) % this.size
                        this.element[this.back] = element
                        this.length++
                    }
                }
                dequeue() {
                    if (this.isEmpty()) throw (new Error("No elements in the queue"))
                    const value = this.getFront()
                    this.element[this.front] = null
                    this.front = (this.front + 1) % this.size
                    this.length--
                    return value
                }
                getFront() {
                    if (this.isEmpty()) throw (new Error("No elements in the queue"))
                    return this.element[this.front]
                }
                clear() {
                    this.element = new Array()
                    this.length = 0
                    this.back = 0
                    this.front = -1
                }
            }

            var logsampling = 0;
            class WSReceiver extends AudioWorkletProcessor {
                constructor(options) {
                    super(options);
                    const BUF_SIZE = 32768;
                    this.LATENCY_SAMPLES = 8192;
                    this.underrun = true;
                    this.samples = new CircularBuffer(BUF_SIZE);
                    this.j = 0;
                    this.port.addEventListener('message', (event) => {
                        this.ondata(event);
                    });
                    this.port.start()
                }
                ondata(event) {
                    if (logsampling == 10) {
                        logsampling = 0;
                    }
                    logsampling++;
                    var d = event['data'];
                    const bias = 174;
                    const scale = 128;
                    for (var i = 0; i < d.length; i++){
                        var v = (d[i]-bias)/scale;
                        this.samples.enqueue(v);
                    }
                }
                process(inputs, outputs) {
                    const output = outputs[0]; // First output
                    var oc = output[0] // First channel
                    
                    // If we had an underrun, wait until the buffer is full again
                    if (this.underrun) {
                        if (this.samples.length >= this.LATENCY_SAMPLES)
                            this.underrun = false;
                    }else{
                        
                        // Underrun detection:
                        if (this.samples.length < oc.length) {
                            console.log("Buffer underrun!", oc.length);
                            this.underrun = true;
                        }else{
                            // If we are in sync, consume the samples
                            for (var i = 0; i < oc.length; i++) {
                                oc[i] = this.samples.dequeue();
                            }
                        }
                    }

                    return true;
                }
            }

            class WSSender extends AudioWorkletProcessor {
                constructor(options) {
                    super(options);
                    this.BUF_SIZE = 512;
                    this.index = 0;
                    this.samples = new Uint8ClampedArray(this.BUF_SIZE);
                }
                process(inputs, outputs) {
                    const inp = inputs[0]; // First output
                    var ic = inp[0] // First channel
                    if(ic){
                        for(var i=0; i<ic.length; i++){
                            var v = ic[i]*127 + 127;
                            this.samples[this.index]=v;
                            this.index++;
                            if(this.index>=this.BUF_SIZE){
                                this.index = 0;
                                let d = new Uint8ClampedArray(this.samples);
                                this.port.postMessage(d);
                            }
                        }
                    }

                    return true;
                }
            }
            registerProcessor('ws-receiver', WSReceiver);
            registerProcessor('ws-sender', WSSender);

        `], { type: 'application/javascript' });
        await audioCtx.audioWorklet.addModule(URL.createObjectURL(blob));
    }
}
async function listen() {
    if (webSocket1 == null)
        return;
    await initCtx();
    if (wssource == null) {
        wssource = new WSSourceNode();
    }
    wssource.connect(audioCtx.destination);
    $(".play-btn").addClass("active-btn");
};
async function talk() {
    if (webSocket1 == null)
        return;
    await initCtx();
    stream = await navigator.mediaDevices.getUserMedia({
        audio: true,
        video: false
    });
    micsource = audioCtx.createMediaStreamSource(stream);
    if (wssink == null) {
        wssink = new WSSinkNode();
    }
    micsource.connect(wssink);
    $.post('https://' + addr + '/setMic', data = {
        'state': "1"
    }).always(function(d) {
        console.log(d);
        $(".rec-btn").addClass("active-btn");
    });
};

async function stop_listen() {
    if (wssource) {
        try {
            wssource.disconnect(audioCtx.destination);
            $(".play-btn").removeClass("active-btn");
        } catch (DOMException) {
            console.log("Already disconnected SPEAKER?");
        }
    }
};
async function stop_talk() {
    if (micsource) {
        try {
            micsource.disconnect();
        } catch (DOMException) {
            console.log("Already disconnected MIC?");
        }
    }
    $.post('https://' + addr + '/setMic', data = {
        'state': "0"
    }).always(function(d) {
        console.log(d);
        $(".rec-btn").removeClass("active-btn");
    });
};

function reconnect() {
    if (webSocket1 != null)
        return;
    webSocket1 = new WebSocket('wss://' + addr + '/ws');
    webSocket1.addEventListener('open', function(event) {
        console.log("Connected.");
        $("#connection-status").html('OK');
        $("#connection-status").removeClass();
        $("#connection-status").addClass('conn-ok');
    });
    webSocket1.addEventListener('close', function(event) {
        console.log("Disconnected.");
        $("#connection-status").html('LOST <a href="javascript:reconnect();">reconnect</a>');
        $("#connection-status").removeClass();
        $("#connection-status").addClass('conn-ko');
        webSocket1 = null;
        stop_talk();
        stop_listen();
    });
    var last_chunk_time_avg = 0;
    var last_chunk_time_num = 0;
    var last_chunk_time = null;
    webSocket1.onmessage = function(a) {
        var t = a.data;
        if (t instanceof Blob) {
            /* Process data */
            t.arrayBuffer().then(function(d) {
                d = new Uint8Array(d);
                if ($("#update_chart_check").prop("checked")) {
                    updateChart(d);
                }
                drawAudio(d, document.getElementById('audioCanvas'));
                if (wssource) {
                    wssource.p.postMessage(d);
                }
            });
            /* Update diagnostics */
            var new_chunk_time = Date.now();
            if (last_chunk_time) {
                var elapsed = new_chunk_time - last_chunk_time;
                last_chunk_time_avg = (last_chunk_time_avg * last_chunk_time_num + elapsed) / (last_chunk_time_num + 1);
                if (last_chunk_time_num < 100)
                    last_chunk_time_num++;
                if (elapsed > last_chunk_time_avg)
                    $("#ic_time").html(elapsed.toFixed(2));
                $("#ic_time_avg").html(last_chunk_time_avg.toFixed(2));
            }
            last_chunk_time = new_chunk_time;
        } else if (t[0] == 'L') {
            console.log(t.substring(1));
        } else {
            t = t.split(',');
            state = t[0];
            userid = t[1];
            message = t[2];
            message = message.split(' ');
            d = new Date(parseInt(message[0]));
            d = zeroPad(d.getHours(), 2) + ":" + zeroPad(d.getMinutes(), 2) + ":" + zeroPad(d.getSeconds(), 2) + "." + zeroPad(d.getMilliseconds(), 3);
            msg = message[1] + " " + message[2] + " " + message[3];
            cmd = message[5];
            id = message[6];
            chk = message[7];
            ack = message[8];
            $("#log").append("<tr>" + "<td>" + d + "</td>" + "<td>" + msg + "</td>" + "<td>" + cmd + "</td>" + "<td>" + id + "</td>" + "<td>" + chk + "</td>" + "<td>" + ack + "</td>" + "</tr>");
            $("#log-container").animate({ 'scrollTop': $("#log-container > table").innerHeight() - $("#log-container").innerHeight() }, 500);
            $("#userid").html(userid);
            $("#state").removeClass();
            $("#state").addClass('state' + state);
            $("#state").html(statestrings['state' + state]);
            if (state == '2') {
                ring();
            }
        }
    };
}

$(function() {
    reconnect();

    $('#makecall').click(function() {
        $.post('https://' + addr + '/makeCall', data = {
            'userid': $('#input_userid').val()
        }).always(function(d) {
            console.log(d);
        });
    });
});