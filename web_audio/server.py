from flask import Flask, render_template, send_from_directory
from flask_sockets import Sockets # https://github.com/heroku-python/flask-sockets
from gevent import monkey
monkey.patch_all()

import numpy as np
from base64 import b64encode
from threading import Thread
from time import sleep


test_data = np.tile(np.array([127, 201, 248, 247, 200, 124,  50,   5,   7,  55, 131, 205, 249,
       246, 196, 121,  47,   4,   8,  58, 135, 208, 250, 244, 193, 116,
        44,   3,  10,  62, 139, 211, 251, 243, 190, 113,  41,   2,  11,
        65, 143, 214, 251, 241, 186, 108,  38,   1,  13,  69, 146, 217,
       252, 239, 183, 105,  35,   1,  15,  72, 150, 219, 253, 237, 179,
       101,  32,   0,  17,  76, 154, 222, 253, 235, 175,  97,  30,   0,
        19,  80, 158, 225, 253, 233, 172,  93,  27,   0,  21,  83, 162,
       227, 254, 231, 168,  89,  25,   0,  23,  87, 166, 230, 254, 228,
       164,  85,  22,   0,  26,  91, 170, 232, 253, 226, 160,  81,  20,
         0,  28,  95, 173, 234, 253, 223, 156,  78,  18,   0,  31,  99,
       177, 236, 253, 221, 152,  74,  16,   0,  34, 103, 181, 238, 252,
       218, 148,  70,  14,   1,  36, 107, 184, 240, 252, 215, 145,  67,
        12,   2,  39, 110, 188, 242, 251, 212, 140,  63,  10,   2,  42,
       114, 191, 243, 250, 209, 137,  60,   9,   3,  45, 118, 195, 245,
       249, 206, 132,  57,   7,   4,  48, 122, 198, 246, 248, 203, 129,
        53,   6,   5,  52], dtype=np.uint8),10)
    
app = Flask(__name__)
sockets = Sockets(app)

def handlews_receive(ws):
    print('WS Open')
    message = ws.receive()
    print(message)

@sockets.route('/ws')
def handlews(ws):
    Thread(target=handlews_receive, daemon=True, args=[ws]).start()
    while not ws.closed:
        sendaudio(ws)
        
    print("WS Closed")

def sendaudio(ws):
    RATE = 4000
    d = test_data
    SENT = len(d)
    TIME_SENT = SENT/RATE
    ws.send( d )
    sleep(TIME_SENT)



@app.route('/')
def index():
    return render_template('index.html')

        

if __name__ == '__main__':
    #Thread(target=sendaudio, daemon=True).start()
    from gevent import pywsgi
    from geventwebsocket.handler import WebSocketHandler
    server = pywsgi.WSGIServer(('', 5000), app, handler_class=WebSocketHandler)
    server.serve_forever()
