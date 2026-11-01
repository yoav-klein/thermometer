import paho.mqtt.client as mqtt

import logging

# Configure to catch lower severity events and change format
logging.basicConfig(
    level=logging.DEBUG,
    format='%(asctime)s - %(levelname)s - %(message)s',
    datefmt='%Y-%m-%d %H:%M:%S'
)

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, reason_code, properties):
    print(f"Connected with result code {reason_code}")
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("measure/backbalcony")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))

mqttc = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id="thermostat", clean_session=True)
mqttc.enable_logger()
mqttc.on_connect = on_connect
mqttc.on_message = on_message

mqttc.tls_set(certfile="terraform/certificate.pem", keyfile="terraform/privatekey.pem")
print("CONNECT")
mqttc.connect("a1h1llc4j4ix9q-ats.iot.us-east-1.amazonaws.com", 8883, 60)
print("AFTER CONNECT")
print(mqttc.is_connected())

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
mqttc.loop_forever()