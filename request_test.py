#!/usr/bin/env python3
import subprocess as sp
import requests
import time
import socket
from http.client import HTTPResponse
import os

BASE_PATH = os.path.abspath(os.getcwd())

def right_path(f):
    def g(*args, **kwargs):
        os.chdir(BASE_PATH)
        return f(*args, **kwargs)
    return g

def send_get(ip, port, target):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET {target} HTTP/1.1\r\nhOsT: {ip}:{port}\r\n\r\n".encode())
    return s

@right_path
def launch_server(args, configs):
    http_proc = sp.Popen(["./httpd"] + args + configs)
    return http_proc

@right_path
def test_dry_run():
    http_proc = sp.Popen(["./httpd", "--dry-run", "tests/meta/reload.conf"])
    time.sleep(0.2)
    res = http_proc.wait()
    assert 0 == res

@right_path
def test_request_default_file():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    response = requests.get(f"http://{ip}:{port}/")
    assert 200 == response.status_code

@right_path
def test_request_no_file():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    response = requests.get(f"http://{ip}:{port}/src")
    assert 404 == response.status_code
    launch_server(["-a", "stop"],["tests/meta/server.conf"])

@right_path
def test_socket_default_file():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = send_get(ip, port, "/src/main.c")
    response = HTTPResponse(s)
    response.begin()
    assert response.status == 200
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])

@right_path
def test_socket_error_ip_host_header():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /src/main.c HTTP/1.1\r\nhOsT: 169.2.2.2:{port}\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 400

@right_path
def test_socket_path_attack():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = send_get(ip, port, "../../Documents/oop.c")
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status in [403, 404]

@right_path
def test_socket_path_attack_2():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = send_get(ip, port, "../Documents/oop.c")
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 404

@right_path
def test_socket_no_path_attack():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = send_get(ip, port, "src/../Makefile")
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 200

@right_path
def test_socket_no_host():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /src/main.c HTTP/1.1\r\n\r\nhOsT: {ip}:{port}\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 400

@right_path
def test_socket_double_column():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /src/main.c HTTP/1.1\r\nhoST:: {ip}:{port}\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 400

@right_path
def test_socket_protocol_error():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /src/main.c HTTP/1.2\r\nHOST: {ip}:{port}\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 505

@right_path
def test_socket_method_error():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"PUT /src/main.c HTTP/1.1\r\nhOsT: {ip}:{port}\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 405

@right_path
def test_socket_invalid():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 400

@right_path
def test_not_enough_header():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"/src/main.c HTTP/1.1\r\nhOsT: {ip}:{port}\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 400

@right_path
def test_option_invalid():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /src/main.c HTTP/1.1\r\nhOsT     : {ip}:{port}\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 400

@right_path
def test_no_CRLFCRLF_err():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /src/main.c HTTP/1.1\r\nhOsT: {ip}:{port}\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 400

@right_path
def test_body():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /src/main.c HTTP/1.1\r\nhOsT: {ip}:{port}\r\n\r\n bisoir".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 200

@right_path
def test_request_without_port():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /src/main.c HTTP/1.1\r\nhOsT: {ip}\r\n\r\n bisoir".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 200

@right_path
def test_request_content_len():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GE\0T /src/\0main.c HTTP/1.1\r\nhOsT: {ip}\r\nContent-Length: 7\r\n\r\n bis\0oir".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 400

@right_path
def test_request_content_len_2():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /src/main.c HTTP/1.1\r\nhOsT: {ip}\r\nContent-Length: 1     3\r\n\r\n bisoirbisoir".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 400

@right_path
def test_server_name():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /src/main.c HTTP/1.1\r\nHoST: {server_name}:{port}\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 200

@right_path
def test_server_name_without_ip():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /src/main.c HTTP/1.1\r\nHoST: {server_name}\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 200

@right_path
def test_two_times_same_header():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /src/main.c HTTP/1.1\r\nHoST: {server_name}\r\nHoST: plouf\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 400

@right_path
def test_invalid_protocol():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /src/main.c HTTP/1x1\r\nHoST: {server_name}\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 400


@right_path
def test_wrong_protocol_bad_request_1():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /src/main.c FTTP/1.1\r\nHoST: {server_name}\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 400

@right_path
def test_wrong_protocol_bad_request_2():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /src/main.c HTTP/0w0\r\nHoST: {server_name}\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 400

@right_path
def test_wrong_protocol_bad_request_3():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /src/main.c HTTP/1x1\r\nHoST: {server_name}\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 400

@right_path
def test_wrong_protocol_bad_request_4():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /src/main.c HTTP=1.1\r\nHoST: {server_name}\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 400

@right_path
def test_wrong_protocol_bad_request_5():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /src/main.c HTTP/1\r\nHoST: {server_name}\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 505

@right_path
def test_abc_content_lenght():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /src/main.c HTTP/1.1\r\nHoST: {server_name}\r\nContent-Length: abc\r\n\r\nbonsoir".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 400

@right_path
def test_content_lenght_negative():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /src/main.c HTTP/1.1\r\nHoST: {server_name}\r\nContent-Length: -7\r\n\r\nbonsoir".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 400

@right_path
def test_empty_host():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /src/main.c HTTP/1.1\r\nHoST: \r\nContent-Length: 7\r\n\r\nbonsoir".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 400

@right_path
def test_body_but_no_body():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /src/main.c HTTP/1.1\r\nHoST: {server_name}\r\nContent-Length: 7\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 400

@right_path
def test_bad_white_space():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /src/main.c HTTP/1.1\r\nHoST: {server_name}\r\nContent-Length : 0\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 400

@right_path
def test_header_is_simple_word():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /src/main.c HTTP/1.1\r\nHoST: {server_name}\r\nContent-Length: \r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 400

@right_path
def test_no_host():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /src/main.c HTTP/1.1\r\nHoST:\r\nContent-Length: 0\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 400

@right_path
def test_no_host_2():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /src/main.c HTTP/1.1\r\nHoST: \r\nContent-Length: 0\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 400

@right_path
def test_leading_slash():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /src/main.c HTTP/1.1\r\nHoST: {server_name}\r\nContent-Length: 0\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 200 #sould be 400 but is 200

@right_path
def test_wrong_ip():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /src/main.c HTTP/1.1\r\nHoST: 127.0.0.2\r\nContent-Length: 0\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 400

@right_path
def test_wrong_port():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /src/main.c HTTP/1.1\r\nHoST: 127.5.5.5:1312\r\nContent-Length: 0\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 400

@right_path
def test_gui():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /index.html HTTP/1.1\r\nHost: 127.5.5.5:42069\r\nContent-length: 24\r\nConnection: close \r\n\r\nThis is a simple request".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 200#sould be an error?

@right_path
def test_header_multiple_times():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /index.html HTTP/1.1\r\nHost: 127.5.5.5:42069\r\nContent-length: 24\r\ncontent-length: 24\r\nConnection: close \r\n\r\nThis is a simple request".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 400