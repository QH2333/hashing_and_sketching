import os
import os.path
import time
import json

import flask
import flask_login
import flask_wtf
from wtforms import StringField, PasswordField, BooleanField
from wtforms.validators import Length,DataRequired,Optional
import mysql.connector

import grpc
import tele_service_pb2
import tele_service_pb2_grpc

BASE_DIR = "/home/qh2333/hashing_and_sketching/web"
RPC_PROTOCOL_VER = "0.1"
AGENT_ADDR = 'localhost:50051'
db_host = "192.168.1.101"
db_user = "topkmanagement"
db_passwd = "admin"
db_name = "topk"
'''
DB structure: 
    create table user ( username varchar(16), password varchar(64) );
To add new user: 
    insert into user values ('admin', SHA2('password',256));
'''

app = flask.Flask(__name__)
app.secret_key = b'topkmanagement'
login_manager = flask_login.LoginManager()
login_manager.init_app(app)

class MyUser(flask_login.UserMixin):
    def __init__(self, id=None):
        super().__init__()
        self.id = id

    def get_id(self):
        return self.id

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

def stop_capture():
    with grpc.insecure_channel(AGENT_ADDR) as channel:
        stub = tele_service_pb2_grpc.tele_serviceStub(channel)
        request = tele_service_pb2.empty_request(protocol_version=RPC_PROTOCOL_VER)
        response = stub.stop_capture(request)
        return response.is_stopped

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
def serve_login():
    return flask.render_template("login.html")

@app.route('/login', methods=['GET', 'POST'])
def login():
    username = flask.request.form['username']
    password = flask.request.form['password']
    identity = validate_user(username, password)
    if (identity != None):
        user = MyUser(username)
        flask_login.login_user(user)
        flask.flash('Logged in successfully.')
        return flask.redirect("/app")
    return flask.render_template('login.html')

@app.route('/app')
@flask_login.login_required
def serve_main():
    return flask.render_template("main.html")

@app.route('/getiflist')
@flask_login.login_required
def serve_getiflist():
    global AGENT_ADDR
    agent_addr = flask.request.args.get("agentAddress")
    AGENT_ADDR = agent_addr
    return json.dumps(get_if_list())

@app.route('/run_capture', methods = ["POST", "GET"])
@flask_login.login_required
def serve_run_capture():
    if_name = flask.request.args.get("ifname")
    pkt_cnt = int(flask.request.args.get("pktCount"))
    k = int(flask.request.args.get("k"))
    is_started = run_capture(if_name, pkt_cnt, k)
    if is_started:
        return "success"
    else:
        return "fail"

@app.route('/stop_capture', methods = ["POST", "GET"])
@flask_login.login_required
def serve_stop_capture():
    is_stopped = stop_capture()
    if is_stopped:
        return "stopped"
    else:
        return "delayed"

@app.route('/get_progress', methods = ["POST", "GET"])
@flask_login.login_required
def serve_get_progress():
    status = get_cap_status()
    if status["is_finished"]:
        return "Done"
    else:
        return str(status["captured_pkt_count"])

@app.route('/get_result', methods = ["POST", "GET"])
@flask_login.login_required
def serve_get_result():
    return json.dumps(get_topk_result())

@login_manager.user_loader
def load_user(user_id):
    conn = mysql.connector.connect(
        user=db_user, host=db_host, password=db_passwd, database=db_name)
    cursor = conn.cursor()
    cursor.execute("select * from user where username='%s'" % user_id)
    values = cursor.fetchall()
    cursor.close()
    conn.close()
    if len(values) != 0:
        return MyUser(user_id)
    else:
        return None

def validate_user(username=None, password=None):
    conn = mysql.connector.connect(user=db_user, host=db_host, password=db_passwd, database=db_name)
    cursor = conn.cursor()
    cursor.execute("select * from user where username='%s' and password='%s'" % (username, password))
    values = cursor.fetchall()
    cursor.close()
    conn.close()
    if len(values) != 0:
        return MyUser(username)
    else:
        return None

if __name__ == "__main__":
    print(get_if_list())
    print(run_capture("eth0", 100, 10))
    while not get_cap_status():
        time.sleep(0.5)
    print(get_topk_result())
