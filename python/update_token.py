#!/usr/bin/env python3
import requests
import os
from configparser import ConfigParser

cfg = ConfigParser()
cfg.read(os.path.expanduser('~/.config/CuteTranslation/config.ini'))

token_url = cfg.get('General', "TokenURL")

os.chdir(os.path.expanduser('~/.config/CuteTranslation/'))

res = requests.get(token_url, timeout=5)
print(res.text)
with open('token', 'w') as f:
    f.write(res.text)
