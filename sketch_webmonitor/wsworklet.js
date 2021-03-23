import CircularBuffer from "./cq.js";

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
    }
    process(inputs, outputs) {
        const inp = inputs[0]; // First output
        var ic = inp[0] // First channel
        if(ic){
            var ic_int = new Uint8Array(ic.buffer);
            for(var i=0; i<ic.buffer.length; i++){
                v = ic[i]*127 + 127;
                v = max(min(v,255),0);
                ic_int[i] = v
            }
            this.port.postMessage(ic_int);
        }

        return true;
    }
}
registerProcessor('ws-receiver', WSReceiver);
registerProcessor('ws-sender', WSSender);
