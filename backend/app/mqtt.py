########################################################################################################
#                                                                                                      #
#   MQTT Paho Documentation - https://eclipse.dev/paho/index.php?page=clients/python/docs/index.php    #
#                                                                                                      #
########################################################################################################
import paho.mqtt.client as mqtt
from random import randint
from json import dumps, loads
from time import sleep

class MQTT:

    # ID = f"IOT_B_1000"
    ID = f"IOT_B_{randint(1,1000000)}"

    #  1. DEFINE ALL TOPICS TO SUBSCRIBE TO. BELOW ARE SOME EXAMPLES. YOUR ARE REQUIRED TO CHANGE THESE TO TOPICS THAT FITS YOUR USE CASE
    #  Lab2 (your topics)
    STUDENT_ID = "620171852"
    TOPIC_UPDATE   = STUDENT_ID               # hardware publishes sensor JSON here
    TOPIC_CONTROLS = f"{STUDENT_ID}_sub"      # backend/frontend publish LED control JSON here
    TOPIC_APP      = "/elet2415"              # sometimes used by frontend (keep subscribed)

    sub_topics = [(TOPIC_UPDATE, 0), (TOPIC_CONTROLS, 0), (TOPIC_APP, 0)]  # A list of tuples of (topic, qos).


    def __init__(self, mongo, server="www.yanacreations.com", port=1883):
        self.randint                = randint
        self.loads                  = loads
        self.dumps                  = dumps
        self.sleep                  = sleep
        self.mongo                  = mongo

        self.client                 = mqtt.Client(client_id=self.ID, clean_session=True, reconnect_on_failure=True)
        self.client.on_connect      = self.on_connect
        self.client.on_message      = self.on_message
        self.client.on_disconnect   = self.on_disconnect
        self.client.on_subscribe    = self.on_subscribe

        # 3. REGISTER CALLBACK FUNCTION(S) FOR EACH TOPIC USING THE self.client.message_callback_add("topic",self.function) FUNCTION
        # WHICH TAKES A TOPIC AND THE NAME OF THE CALLBACK FUNCTION YOU HAVE CREATED FOR THIS SPECIFIC TOPIC
        self.client.message_callback_add(self.TOPIC_UPDATE, self.update)
        self.client.message_callback_add(self.TOPIC_CONTROLS, self.controls)
        self.client.message_callback_add(self.TOPIC_APP, self.app)

        # 4. UPDATE MQTT SERVER AND PORT INFORMATION BELOW
        self.client.connect_async(server, port, 60)


    def connack_string(self, rc):
        connection = {
            0: "Connection successful",
            1: "Connection refused - incorrect protocol version",
            2: "Connection refused - invalid client identifier",
            3: "Connection refused - server unavailable",
            4: "Connection refused - bad username or password",
            5: "Connection refused - not authorised"
        }
        return connection.get(rc, "Unknown return code")


    def on_connect(self, client, userdata, flags, rc):
        # Called when the broker responds to our connection request.
        print("\n\nMQTT: " + self.connack_string(rc), " ID: ", client._client_id.decode('utf-8'))
        # Subscribing in on_connect() means that if we lose the connection and reconnect then subscriptions will be renewed.
        client.subscribe(self.sub_topics)


    def on_subscribe(self, client, userdata, mid, granted_qos):
        # Called when the broker responds to a subscribe request.
        print("MQTT: Subscribed to", [topic[0] for topic in self.sub_topics])


    def publish(self, topic, payload):
        try:
            info = self.client.publish(topic, payload)
            info.wait_for_publish()
            return info.is_published()
        except Exception as e:
            print(f"MQTT: Publish failed {str(e)}")
            return False


    def on_message(self, client, userdata, msg):
        # Default callback (runs if a message arrives on a topic without a specific callback)
        try:
            print(msg.topic + " " + str(msg.payload.decode("utf-8")))
        except Exception as e:
            print(f"MQTT: onMessage Error: {str(e)}")


    def on_disconnect(self, client, userdata, rc):
        if rc != 0:
            print("MQTT: Unexpected Disconnection.")


    # 2. DEFINE CALLBACK FUNCTIONS(S) BELOW FOR EACH TOPIC(S) THE BACKEND SUBSCRIBES TO

    def update(self, client, userdata, msg):
        """
        CALLBACK for hardware updates topic (student_id).
        Expects:
        {"id":"620171852","timestamp":1702212234,"temperature":30,"humidity":90,"heatindex":30}
        Must INSERT into MongoDB: ELET2415.climo using mongo.addUpdate(data)
        """
        try:
            payload = msg.payload.decode("utf-8")
            data = loads(payload)

            # Optional sanity checks (prevents crashing if message is bad)
            if "timestamp" not in data:
                print("MQTT:update -> missing timestamp, ignoring:", payload)
                return

            # Insert into MongoDB
            ok = self.mongo.addUpdate(data)
            if ok:
                print("MQTT:update -> saved to MongoDB (climo)")
            else:
                print("MQTT:update -> NOT saved (maybe duplicate timestamp)")
        except Exception as e:
            print(f"MQTT:update Error: {str(e)}")


    def controls(self, client, userdata, msg):
        """
        CALLBACK for controls topic (student_id_sub).
        Expects something like:
        {"type":"controls","leds":2,"brightness":120,"color":{"r":255,"g":0,"b":0}}

        Backend usually forwards this to the frontend (or logs it). If your frontend listens on /elet2415,
        we can republish there. If not needed, you can comment out the publish.
        """
        try:
            payload = msg.payload.decode("utf-8")
            print("MQTT:controls ->", payload)

            # Forward to /elet2415 (ONLY if your frontend expects it)
            # self.publish(self.TOPIC_APP, payload)

        except Exception as e:
            print(f"MQTT:controls Error: {str(e)}")


    def app(self, client, userdata, msg):
        """
        CALLBACK for /elet2415 (if used by your frontend or lab).
        """
        try:
            payload = msg.payload.decode("utf-8")
            print("MQTT:/elet2415 ->", payload)
        except Exception as e:
            print(f"MQTT:app Error: {str(e)}")
