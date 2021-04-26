import argparse
import os
import os.path
import subprocess
import sys
import time

def get_if_list():
    if_list = []
    with open("input", "w") as f:
        f.write("getiflist")

    os.system("../top-k/topk -f")

    with open("output", "r") as f:
        lines = f.readlines()
        for line in lines:
            if_list.append(line.split("\t"))
    
    return if_list

def start_monitor_pkt_on_if(if_name, pkt_cnt, k):
    with open("input", "w") as f:
        f.write("capture\n%s %s %s" % (if_name, pkt_cnt, k))
    if os.path.exists("log"):
        os.system("rm log")
    os.system("../top-k/topk -f")

def is_finished():
    if not os.path.exists("log"):
        return False
    with open("log", "r") as f:
        lines = f.readlines()
        if lines[-1] == "Done":
            return True
        else:
            return False

def get_result():
    result_list = []
    with open("output", "r") as f:
        lines = f.readlines()
        for line in lines:
            result_list.append(line.split("\t"))
    
    return result_list

print(get_if_list())

start_monitor_pkt_on_if("eth0", 100, 10)

while not is_finished():
    time.sleep(0.1)

print(get_result())