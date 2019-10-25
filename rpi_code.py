from allthingstalk import Client, Device, IntegerAsset, BooleanAsset, Asset

from xOC03 import xOC03
from xCore import xCore

import time

OC03 = xOC03()

# start OC03
OC03.init()

# Parameters used to authorize and identify your device
# Get them on maker.allthingstalk.com
DEVICE_TOKEN = 'Your DEVICE_TOKEN' #Enter here your Device Token
DEVICE_ID = 'Your DEVICE_ID' #Enter here your Device ID
    
client = Client(DEVICE_TOKEN)

while True:
    state = str(client.get_asset_state(DEVICE_ID,"1"))
    print(state)
    if state == "True":
        OC03.writePin(True)
    elif state == "False":
        OC03.writePin(False)
