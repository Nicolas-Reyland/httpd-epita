import subprocess as sp
import requests
import time
import socket
from http.client import HTTPResponse

def launch_server(args, configs):
    http_proc = sp.Popen(["./httpd"] + args + configs)
    return http_proc

def test_dry_run():
    http_proc = sp.Popen(["./httpd", "--dry-run", "./reload.conf"])
    time.sleep(0.2)
    res = http_proc.wait()
    assert 0 == res

def test_request_default_file():
    http_proc = launch_server([],["reload.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    response = requests.get(f"http://{ip}:{port}/")
    assert 200 == response.status_code

def test_request_no_file():
    http_proc = launch_server([],["reload.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    response = requests.get(f"http://{ip}:{port}/src")
    assert 404 == response.status_code

def test_traversal_attack():
    http_proc = launch_server([],["reload.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    response = requests.get(f"http://{ip}:{port}/../../Documents/oop.c")
    assert 404 == response.status_code

def send_get(ip, port, target):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send("GET {target} HTTP/1.1\r\nHost: {ip}:{port}\r\nConnection: keep-alive\r\n\r\n".encode())
    return s

def test_socket_default_file():
    http_proc = launch_server([],["reload.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = send_get(ip, port, "./")
    response = HTTPResponse(s)
    response.begin()
    assert response.status == 404
    #should return 200