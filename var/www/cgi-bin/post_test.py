#!/usr/bin/env python3

import os
import sys
from urllib.parse import parse_qs

def get_params():
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    print(f"Content-Length: {content_length}\r\n", end='')
    post_data = sys.stdin.read(content_length) if content_length > 0 else ""
    params = parse_qs(post_data)
    return params

def build_response_headers():
    print(f"Content-Type: text/html\r\n", end='')
    print(f"\r\n", end='')

def build_response_body(name, age):
    print(f"<!DOCTYPE html>")
    print(f"<html>")
    print(f"<body>")
    print(f"<h1>POST data recieved</h1>")
    print(f"<p><b>City:</b> {city}</p>")
    print(f"<p><b>Country:</b> {country}</p>")
    print(f"</body>")
    print(f"</html>")

build_response_headers()
params = get_params()
city = params.get('city', [''])[0]
country = params.get('country', [''])[0]
build_response_body(city, country)
