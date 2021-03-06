#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import BaiduTranslate
import json
import sys
import os
import pickle
import time

if __name__ == "__main__":
    os.chdir(os.path.expanduser('~/.config/CuteTranslation/'))

    if os.path.exists("dict_object"):
        with open("dict_object", "rb") as f:
            d = pickle.load(f)
    else:
        d = BaiduTranslate.Dict()
        with open("dict_object", "wb") as f:
            pickle.dump(d, f)

    # open fanyi.baidu.com to get the code of language
    # https://fanyi.baidu.com/#zh/en/你好
    # json = d.dictionary('like a hot knife through butter', dst='zh', src='en')
    # print(json)
    # json = d.dictionary('青花瓷', dst='en', src='zh')
    #print(json['trans_result']['data'][0]['dst'])
    word = sys.argv[1]
    is_latin = True
    for c in word:
        if ord(c) > 127:
            is_latin = False
    if is_latin:
        obj = d.dictionary(word, dst='zh', src='en')
    else:
        obj = d.dictionary(word, dst='zh', src=d.langdetect(word))
    if "dict_result" in obj:
        print(json.dumps(obj["dict_result"]["simple_means"], ensure_ascii=False))
    elif "trans_result" in obj:
        data_list = obj["trans_result"]["data"]
        res = ""
        for item in data_list:
            res += item["dst"] + "<br/>"
        print(res)
    else:
        print("None\n")
        
