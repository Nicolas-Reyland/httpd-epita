#!/usr/bin/env python3
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
def start_server(config):
    assert os.system(f"./httpd -a start {config}") == 0

@right_path
def stop_server(config):
    assert os.system(f"./httpd -a stop {config}") == 0
    # Wait up to 0.5s for process to end
    for _ in range(100):
        if os.system("pgrep httpd") != 0:
            break
        time.sleep(0.05)

@right_path
def test_dry_run():
    assert os.system("./httpd --dry-run tests/meta/reload.conf") == 0

@right_path
def test_request_default_file():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    response = requests.get(f"http://{ip}:{port}/")
    assert 200 == response.status_code
    stop_server("tests/meta/server.conf")

@right_path
def test_acu():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /index.html HTTP/1.1\r\nHost: test.com:{port}\r\nContent-length: 24\r\nConnection: close \r\n\r\nThis is a simple request".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 200

@right_path
def test_request_no_file():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    response = requests.get(f"http://{ip}:{port}/src")
    assert 404 == response.status_code
    stop_server("tests/meta/server.conf")

@right_path
def test_socket_default_file():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = send_get(ip, port, "/meta/server.conf")
    response = HTTPResponse(s)
    response.begin()
    assert response.status == 200
    stop_server("tests/meta/server.conf")

@right_path
def test_socket_error_ip_host_header():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /meta/server.conf HTTP/1.1\r\nhOsT: 169.2.2.2:{port}\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 400

@right_path
def test_socket_path_attack():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = send_get(ip, port, "/../../Documents/oop.c")
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status in [403, 404]

@right_path
def test_socket_path_attack_2():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = send_get(ip, port, "/../Documents/oop.c")
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 404

@right_path
def test_socket_no_path_attack():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = send_get(ip, port, "/meta/../request_test.py")
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 200

@right_path
def test_socket_no_host():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /meta/server.conf HTTP/1.1\r\n\r\nhOsT: {ip}:{port}\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 400

@right_path
def test_socket_double_column():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /meta/server.conf HTTP/1.1\r\nhoST:: {ip}:{port}\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 400

@right_path
def test_socket_protocol_error():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /meta/server.conf HTTP/1.2\r\nHOST: {ip}:{port}\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 505

@right_path
def test_socket_method_error():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"PUT /meta/server.conf HTTP/1.1\r\nhOsT: {ip}:{port}\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 405

@right_path
def test_socket_invalid():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 400

@right_path
def test_not_enough_header():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"/meta/server.conf HTTP/1.1\r\nhOsT: {ip}:{port}\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 400

@right_path
def test_option_invalid():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /meta/server.conf HTTP/1.1\r\nhOsT     : {ip}:{port}\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 400

'''
@right_path
def test_no_CRLFCRLF_err():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /meta/server.conf HTTP/1.1\r\nhOsT: {ip}:{port}\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 400
'''

@right_path
def test_body():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /meta/server.conf HTTP/1.1\r\nhOsT: {ip}:{port}\r\n\r\n bisoir".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 200

@right_path
def test_request_without_port():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /meta/server.conf HTTP/1.1\r\nhOsT: {ip}\r\n\r\n bisoir".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 200

@right_path
def test_request_content_len():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GE\0T /src/\0main.c HTTP/1.1\r\nhOsT: {ip}\r\nContent-Length: 7\r\n\r\n bis\0oir".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 400

@right_path
def test_request_content_len_2():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /meta/server.conf HTTP/1.1\r\nhOsT: {ip}\r\nContent-Length: 1     3\r\n\r\n bisoirbisoir".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 400

@right_path
def test_server_name():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /meta/server.conf HTTP/1.1\r\nHoST: {server_name}:{port}\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 200

@right_path
def test_server_name_without_ip():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /meta/server.conf HTTP/1.1\r\nHoST: {server_name}\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 200

@right_path
def test_two_times_same_header():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /meta/server.conf HTTP/1.1\r\nHoST: {server_name}\r\nHoST: plouf\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 400

@right_path
def test_invalid_protocol():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /meta/server.conf HTTP/1x1\r\nHoST: {server_name}\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 400


@right_path
def test_wrong_protocol_bad_request_1():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /meta/server.conf FTTP/1.1\r\nHoST: {server_name}\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 400

@right_path
def test_wrong_protocol_bad_request_2():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /meta/server.conf HTTP/0w0\r\nHoST: {server_name}\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 400

@right_path
def test_wrong_protocol_bad_request_3():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /meta/server.conf HTTP/1x1\r\nHoST: {server_name}\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 400

@right_path
def test_wrong_protocol_bad_request_4():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /meta/server.conf HTTP=1.1\r\nHoST: {server_name}\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 400

@right_path
def test_wrong_protocol_bad_request_5():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /meta/server.conf HTTP/1\r\nHoST: {server_name}\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 505

@right_path
def test_abc_content_lenght():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /meta/server.conf HTTP/1.1\r\nHoST: {server_name}\r\nContent-Length: abc\r\n\r\nbonsoir".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 400

@right_path
def test_content_lenght_negative():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /meta/server.conf HTTP/1.1\r\nHoST: {server_name}\r\nContent-Length: -7\r\n\r\nbonsoir".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 400

@right_path
def test_empty_host():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /meta/server.conf HTTP/1.1\r\nHoST: \r\nContent-Length: 7\r\n\r\nbonsoir".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 400

@right_path
def test_body_but_no_body():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /meta/server.conf HTTP/1.1\r\nHoST: {server_name}\r\nContent-Length: 7\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 400

@right_path
def test_bad_white_space():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /meta/server.conf HTTP/1.1\r\nHoST: {server_name}\r\nContent-Length : 0\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 400

@right_path
def test_header_is_simple_word():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /meta/server.conf HTTP/1.1\r\nHoST: {server_name}\r\nContent-Length\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 400

@right_path
def test_no_host():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /meta/server.conf HTTP/1.1\r\nHoST:\r\nContent-Length: 0\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 400

@right_path
def test_no_host_2():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /meta/server.conf HTTP/1.1\r\nHoST: \r\nContent-Length: 0\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 400

@right_path
def test_missing_leading_slash():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET src/main.c HTTP/1.1\r\nHoST: {server_name}\r\nContent-Length: 0\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 400 #sould be 400 but is 200

@right_path
def test_wrong_ip():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /meta/server.conf HTTP/1.1\r\nHoST: 127.0.0.2\r\nContent-Length: 0\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 400

@right_path
def test_wrong_port():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /meta/server.conf HTTP/1.1\r\nHoST: 127.5.5.5:1312\r\nContent-Length: 0\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 400

@right_path
def test_gui():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /index.html HTTP/1.1\r\nHost: 127.5.5.5:42069\r\nContent-length: 24\r\nConnection: close \r\n\r\nThis is a simple request".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 200#sould be an error?

@right_path
def test_header_multiple_times():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /index.html HTTP/1.1\r\nHost: 127.5.5.5:42069\r\nContent-length: 24\r\ncontent-length: 24\r\nConnection: close \r\n\r\nThis is a simple request".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 400

@right_path
def test_no_header_protocol():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /index.html \r\nHost: 127.5.5.5:42069\r\nContent-length: 24\r\nConnection: close \r\n\r\nThis is a simple request".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 400

@right_path
def test_space_beginning_header():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /index.html HTTP/1.1\r\n   Host: 127.5.5.5:42069\r\nContent-length: 24\r\nConnection: close \r\n\r\nThis is a simple request".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 400

@right_path
def test_space_between_colomn_and_key():
    start_server("tests/meta/server.conf")
    time.sleep(0.2)
    ip = "127.5.5.5"
    server_name = "test.com"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /index.html HTTP/1.1\r\nHost : 127.5.5.5:42069\r\nContent-length: 24\r\nConnection: close \r\n\r\nThis is a simple request".encode())
    response = HTTPResponse(s)
    response.begin()
    stop_server("tests/meta/server.conf")
    assert response.status == 400

