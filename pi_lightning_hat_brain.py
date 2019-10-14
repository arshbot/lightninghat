import pysher
import time
import logging
import sys
import json
import qrcode
import serial
import subprocess
import requests

root = logging.getLogger()
root.setLevel(logging.INFO)
ch = logging.StreamHandler(sys.stdout)
root.addHandler(ch)

key = "8f167ac7d1842453e486"
cluster = "us2"
secret = "5fbd1ad8fb00909eaaf4"
app_id = "874808"

spin_time_per_payment = "3000" # milliseconds
current_index = 0
save_dir = '/tmp/'
qrcode_name = 'lnhat'
is_last_fan_A = True


pusher = pysher.Pusher(
    key=key,
    cluster=cluster,
    secret=secret,
    user_data={'user_id': app_id}
)


def get_current_qr_code_url():
    global current_index

    name = save_dir + qrcode_name + str(current_index) + ".png"
    return name

def gen_qr_code_url():
    global current_index

    name = save_dir + qrcode_name + str(current_index) + ".png"
    current_index += 1
    return name

def gen_qr_code(payment_string):
    img = qrcode.make(payment_string)
    img.save(gen_qr_code_url())
    return

def spin_fans(spin_time_per_payment):
    global ser, is_last_fan_A

    fan = 'B' if is_last_fan_A else 'A'
    is_last_fan_A = not is_last_fan_A
    msg = "{}:{}\n".format(fan, spin_time_per_payment)
    ser.write(msg.encode())
    received_msg = ser.readline()

    if 'Received Message: {}'.format(msg).encode().strip() != received_msg.strip():
        print("Message dropped")
    else:
        print("Spinning Motor: {}".format(fan))

    return

def display_qr_code():
    subprocess.call(["fbi", "-a", get_current_qr_code_url()])
    return

def handle_successful_payment(*args, **kwargs):
    global is_last_fan_A

    args = json.loads(args[0])
    gen_qr_code(args['payment_string'])
    display_qr_code()
    spin_fans(5000)
    return

def get_first_payment_string():
    res = requests.get("https://lnhat.com/payment_string")
    payment_string = res.json()['payment_string']
    return payment_string

def connect_handler(data):
    global pusher

    channel = pusher.subscribe('main')
    channel.bind('payment', handle_successful_payment)


while True:
    try:
        ser = serial.Serial("/dev/ttyACM0",9600)
        ser.flushInput()
        print("Successfully connected to serial port")
        break
    except serial.serialutil.SerialException as e:
        print("Connecting to port failed: {}".format(e))
        continue

    time.sleep(1)

pusher.connection.bind('pusher:connection_established', connect_handler)
pusher.connect()
print("Successfully connected to pusher")

print("DEBUG: Getting first payment string")
payment_string = get_first_payment_string()
print("Generating qr code")
gen_qr_code(payment_string)
print("Displaying qr code")
display_qr_code()

while True:
    time.sleep(1)
