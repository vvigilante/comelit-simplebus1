import CircularBuffer from "./cq.js";

var logsampling = 0;
class WSReceiver extends AudioWorkletProcessor {
    constructor(options) {
        super();
        const BUF_SIZE = 16384;
        this.LATENCY_SAMPLES = 4096;
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
            console.log(this.samples.length);
        }
        logsampling++;
        var d = event['data'];
        for (var i = 0; i < d.length; i++){
            var v = d.charCodeAt(i)/127 - 1;
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

registerProcessor('ws-receiver', WSReceiver);
