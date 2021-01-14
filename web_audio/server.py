from flask import Flask, render_template, send_from_directory
from flask_socketio import SocketIO, emit
import numpy as np
from base64 import b64encode
from threading import Thread
from time import sleep

app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app)

@app.route('/')
def index():
    return render_template('index.html')

@socketio.on('client_event')
def test_message(message):
    global flag
    emit('server_response', {'data': "Server heard "+message['data']})
    flag=True

@socketio.on('connect')
def test_connect():
    emit('server_response', {'data': 'Connected'})

@socketio.on('disconnect')
def test_disconnect():
    print('Client disconnected')


def sendaudio():
    RATE = 4000
    while True:
        d = [0.000,0.590,0.953,0.948,0.578,-0.016,-0.603,-0.958,-0.943,-0.565,0.032,0.616,0.962,0.938,0.551,-0.047,-0.628,-0.966,-0.932,-0.538,0.063,0.640,0.970,0.926,0.525,-0.079,-0.652,-0.974,-0.920,-0.511,0.095,0.664,0.977,0.914,0.498,-0.110,-0.676,-0.981,-0.907,-0.484,0.126,0.687,0.984,0.900,0.470,-0.142,-0.699,-0.986,-0.894,-0.456,0.157,0.710,0.989,0.886,0.442,-0.173,-0.721,-0.991,-0.879,-0.428,0.188,0.732,0.993,0.871,0.413,-0.204,-0.742,-0.995,-0.863,-0.399,0.219,0.753,0.996,0.855,0.385,-0.235,-0.763,-0.997,-0.847,-0.370,0.250,0.773,0.998,0.839,0.355,-0.265,-0.783,-0.999,-0.830,-0.340,0.280,0.793,1.000,0.821,0.325,-0.295,-0.802,-1.000,-0.812,-0.311,0.311,0.812,1.000,0.802,0.295,-0.325,-0.821,-1.000,-0.793,-0.280,0.340,0.830,0.999,0.783,0.265,-0.355,-0.839,-0.998,-0.773,-0.250,0.370,0.847,0.997,0.763,0.235,-0.385,-0.855,-0.996,-0.753,-0.219,0.399,0.863,0.995,0.742,0.204,-0.413,-0.871,-0.993,-0.732,-0.188,0.428,0.879,0.991,0.721,0.173,-0.442,-0.886,-0.989,-0.710,-0.157,0.456,0.894,0.986,0.699,0.142,-0.470,-0.900,-0.984,-0.687,-0.126,0.484,0.907,0.981,0.676,0.110,-0.498,-0.914,-0.977,-0.664,-0.095,0.511,0.920,0.974,0.652,0.079,-0.525,-0.926,-0.970,-0.640,-0.063,0.538,0.932,0.966,0.628,0.047,-0.551,-0.938,-0.962,-0.616,-0.032,0.565,0.943,0.958,0.603,0.016,-0.578,-0.948,-0.953,-0.590]
        d*=10
        d = (np.array(d)*127+127).astype(np.uint8)
        SENT = len(d)
        TIME_SENT = SENT/RATE
        socketio.emit('audio_data', {'data': b64encode(d.tobytes()).decode('ascii')})
        sleep(TIME_SENT)
        

if __name__ == '__main__':
    Thread(target=sendaudio, daemon=True).start()
    socketio.run(app, debug=True)