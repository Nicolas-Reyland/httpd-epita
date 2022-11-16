import subprocess as sp
import requests
import time
import socket
from http.client import HTTPResponse

def launch_server(args, configs):
    http_proc = sp.Popen(["./httpd"] + args + configs)
    return http_proc

def test_dry_run():
    http_proc = sp.Popen(["./httpd", "--dry-run", "tests/meta/reload.conf"])
    time.sleep(0.2)
    res = http_proc.wait()
    assert 0 == res

def test_request_default_file():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    response = requests.get(f"http://{ip}:{port}/")
    assert 200 == response.status_code

def test_request_no_file():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    response = requests.get(f"http://{ip}:{port}/src")
    assert 404 == response.status_code

def send_get(ip, port, target):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET {target} HTTP/1.1\r\nhOsT: {ip}:{port}\r\n\r\n".encode())
    return s

def test_socket_default_file():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = send_get(ip, port, "/src/main.c")
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 200

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

def test_socket_path_attack():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = send_get(ip, port, "../../Documents/oop.c")
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 403

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

def test_socket_double_column():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /src/main.c HTTP/1.1\r\nhOsT:: {ip}:{port}\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 400

def test_socket_protocol_error():
    http_proc = launch_server(["-a", "start"],["tests/meta/server.conf"])
    time.sleep(0.2)
    ip = "127.5.5.5"
    port = "42069"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.send(f"GET /src/main.c HTTP/1.2\r\nhOsT: {ip}:{port}\r\n\r\n".encode())
    response = HTTPResponse(s)
    response.begin()
    http_proc = launch_server(["-a", "stop"],["tests/meta/server.conf"])
    assert response.status == 505

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