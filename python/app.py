#!flask/bin/python
from flask import Flask, jsonify
import subprocess

app = Flask(__name__)

class Pool:

    def __init__(self, id):
        self.id = id
        self.temperature = None
        self.temperatureUnit = None
        self.vdcin = None
        self.error = None

        self.set_temperature()

    def set_temperature(self):
        cmd = "/usr/local/bin/getadc" 

        out, err = subprocess.Popen([cmd, str(self.id)], stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()

        #TODO: Add error handling 
        self.vdcin = int(out)

        ####
        # y = m * x + b       vdc = m * temp + b
        # x = (y - b) / m     temp = (vdc - b) / m
        #
        # Using Honeywell T755M Series 2000 Electronic Stand-Alone Controller
        # connected to MOD2, using SENSOR1.  MOD2 setup like so:
        #   - TYPE = 2 - 10 VDC 
        #   - SETPOINT = 100
        #   - THROTTLING RANGE = 40 (Range 80 F to 120 F) 
        #   - INTEGRAL = 0
        #
        # x1, x2, y1, y2 = 80, 120, 10, 2 (yes, y-axis is decreasing)
        #
        # Assuming 2000 mV is 80 and 10000 mV is 120:
        # temp = (vdc - 26) / -0.2
        ####

        m = -0.2
        b = 26
        converted_temperature = (self.vdcin - b) / m
        self.temperature = round(converted_temperature, 1)
        self.temperatureUnit = "F"

    def serialize(self):
        return {
            'id': self.id, 
            'temperature': self.temperature,
            'temperatureUnit': self.temperatureUnit,
            'vdcin': self.vdcin,
        } 

@app.route('/temperatures')
def get_temperatures():

    allPools = []
    for i in range(0,3):
        pool = Pool(i)
        allPools.append(pool.__dict__)

    return jsonify(allPools);

@app.route('/temperatures/<int:pool_id>', methods=['GET'])
def get_temperature(pool_id):

    pool = Pool(pool_id)
    return jsonify(pool.__dict__)


if __name__ == '__main__':
    app.run(debug=True, port=8080)
