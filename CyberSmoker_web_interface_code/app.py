from flask import Flask, render_template, request, jsonify
import paho.mqtt.client as mqtt

import subprocess
import os
import time

### LAUNCH INTERNET FLASK SERVER ###


app = Flask(__name__)
mqtt_broker = '192.168.1.83'  # IP of your MQTT broker (probably your PC)
smoker_data = {"meat_temp":'--',"ambient_temp":'--',"set_temp":'--',"pellet_level":'--'}

# MQTT Setup
def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT Broker")
    client.subscribe("esp32/data/smoker_data")

def on_message(client, userdata, msg):
    global smoker_data
    print("DEBUG one message being called")
    smoker_data= msg.payload.decode()

mqtt_client = mqtt.Client()
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message
mqtt_client.connect(mqtt_broker, 1883, 60)
mqtt_client.loop_start()

@app.route('/')
def index():
    return render_template("index.html")

@app.route('/smoker_data')
def get_smoker_data():
    print(f"DEBUG get_smoker_data is being called,  returning jsonified: {smoker_data}")
    return smoker_data

@app.route('/command', methods=["POST"])
def send_command():
    cmd = request.json.get("command")
    mqtt_client.publish("esp32/command", cmd)
    return jsonify({"status": "sent", "command": cmd})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)  # accessible from other LAN devices
