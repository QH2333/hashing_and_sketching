import os
import os.path
import sys
import time
import json
import threading

import flask
app = flask.Flask(__name__)

BASE_DIR = "/home/qh2333/hashing_and_sketching/web"

def get_if_list():
    os.chdir(BASE_DIR)
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
    os.chdir(BASE_DIR)
    with open("input", "w") as f:
        f.write("capture\n%s %s %s" % (if_name, pkt_cnt, k))
    if os.path.exists("log"):
        os.system("rm log")
    t1 = threading.Thread(target = lambda : os.system("../top-k/topk -f"))
    t1.start()

def is_finished():
    os.chdir(BASE_DIR)
    if not os.path.exists("log"):
        return False
    with open("log", "r") as f:
        lines = f.readlines()
        if lines[-1] == "Done":
            return True
        else:
            return False

def get_result():
    os.chdir(BASE_DIR)
    result_list = []
    with open("output", "r") as f:
        lines = f.readlines()
        for line in lines:
            result_list.append(line.strip().split("\t"))
    
    return result_list

@app.route('/')
def serve_main():
    return flask.render_template("main.html")

@app.route('/getiflist')
def serve_getiflist():
    return json.dumps(get_if_list())

@app.route('/run_capture', methods = ["POST", "GET"])
def serve_run_capture():
    if_name = flask.request.args.get("ifname")
    pkt_cnt = flask.request.args.get("pktCount")
    k = flask.request.args.get("k")
    print([if_name, pkt_cnt, k])
    start_monitor_pkt_on_if(if_name, pkt_cnt, k)
    return "success"

@app.route('/get_progress', methods = ["POST", "GET"])
def serve_get_progress():
    with open("log", "r") as f:
        return f.readlines()[-1].strip()

@app.route('/get_result', methods = ["POST", "GET"])
def serve_get_result():
    if is_finished():
        return json.dumps(get_result())
    else:
        return "fail"
    return "success"

if __name__ == "__main__":
    print(get_if_list())
    start_monitor_pkt_on_if("eth0", 100, 10)
    while not is_finished():
        time.sleep(0.1)
    print(get_result())