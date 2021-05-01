import os
import os.path
import sys
import time
import json
import threading

import grpc
import tele_service_pb2
import tele_service_pb2_grpc
import flask
app = flask.Flask(__name__)

BASE_DIR = "/home/qh2333/hashing_and_sketching/web"
RPC_PROTOCOL_VER = "0.1"
AGENT_ADDR = 'localhost:50051'

def get_if_list():
    ret_val = []
    with grpc.insecure_channel(AGENT_ADDR) as channel:
        stub = tele_service_pb2_grpc.tele_serviceStub(channel)
        request = tele_service_pb2.empty_request(protocol_version=RPC_PROTOCOL_VER)
        response = stub.get_if_list(request)
        for entry in response.interface_list:
            ret_val.append([entry.if_name, entry.if_description])
    return ret_val

def run_capture(if_name, pkt_cnt, k):
    with grpc.insecure_channel(AGENT_ADDR) as channel:
        stub = tele_service_pb2_grpc.tele_serviceStub(channel)
        request = tele_service_pb2.run_cap_request(protocol_version=RPC_PROTOCOL_VER, if_name=if_name, pkt_count=pkt_cnt, k=k)
        response = stub.run_capture(request)
        return response.is_started

def get_cap_status():
    with grpc.insecure_channel(AGENT_ADDR) as channel:
        stub = tele_service_pb2_grpc.tele_serviceStub(channel)
        request = tele_service_pb2.empty_request(protocol_version=RPC_PROTOCOL_VER)
        response = stub.get_cap_status(request)
        return {"is_finished": response.is_finished, "captured_pkt_count": response.captured_pkt_count}

def get_topk_result():
    ret_val = []
    with grpc.insecure_channel(AGENT_ADDR) as channel:
        stub = tele_service_pb2_grpc.tele_serviceStub(channel)
        request = tele_service_pb2.empty_request(protocol_version=RPC_PROTOCOL_VER)
        response = stub.get_topk_result(request)
        for entry in response.topk_results:
            ret_val.append([entry.id, entry.flow_description, entry.flow_count])
    return ret_val

@app.route('/')
def serve_main():
    return flask.render_template("main.html")

@app.route('/getiflist')
def serve_getiflist():
    global AGENT_ADDR
    agent_addr = flask.request.args.get("agentAddress")
    AGENT_ADDR = agent_addr
    return json.dumps(get_if_list())

@app.route('/run_capture', methods = ["POST", "GET"])
def serve_run_capture():
    if_name = flask.request.args.get("ifname")
    pkt_cnt = int(flask.request.args.get("pktCount"))
    k = int(flask.request.args.get("k"))
    is_started = run_capture(if_name, pkt_cnt, k)
    if is_started:
        return "success"
    else:
        return "fail"

@app.route('/get_progress', methods = ["POST", "GET"])
def serve_get_progress():
    status = get_cap_status()
    if status["is_finished"]:
        return "Done"
    else:
        return str(status["captured_pkt_count"])

@app.route('/get_result', methods = ["POST", "GET"])
def serve_get_result():
    return json.dumps(get_topk_result())

if __name__ == "__main__":
    print(get_if_list())
    print(run_capture("eth0", 100, 10))
    while not get_cap_status():
        time.sleep(0.5)
    print(get_topk_result())
