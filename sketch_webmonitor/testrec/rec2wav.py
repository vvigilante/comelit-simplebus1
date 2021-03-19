import sys,os
import json
import wave
import struct
from tqdm import tqdm
import numpy as np
import matplotlib.pyplot as plt


with open('rec.json') as f:
    d = json.load(f)
audio = [i['y'] for i in d]
audio = np.array(audio, dtype=np.int16)

bias = 1000

audio-=bias
audio*= 2**(15-12)

plt.plot(range(len(audio)), audio)
plt.grid(True)
plt.show()
#sys.exit(0)

wav_file=wave.open("rec.wav","w")
nchannels = 1
sampwidth = 2
sample_rate = 8000
nframes = len(audio)
comptype = "NONE"
compname = "not compressed"
wav_file.setparams((nchannels, sampwidth, sample_rate, nframes, comptype, compname))
#for sample in tqdm(audio):
#    wav_file.writeframes( struct.pack('>f', sample) )
wav_file.writeframes( audio )

wav_file.close()

