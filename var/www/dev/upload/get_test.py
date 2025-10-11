#!/usr/bin/env python3

import os
from urllib.parse import parse_qs

def get_params():
    query = os.environ.get('QUERY_STRING', '')
    params = parse_qs(query)
    return params

def build_response_headers():
    print(f"Content-Type: text/html\r\n", end='')
    print(f"\r\n", end='')

def build_response_body(name, age):
    print(f"<!DOCTYPE html>")
    print(f"<html>")
    print(f"<body>")
    print(f"<h1>Hello, {name}!</h1>")
    print(f"<p>You are {age} years old.</p>")
    print(f"</body>")
    print(f"</html>")

build_response_headers()
params = get_params()
name = params.get('name_py', ['Guest'])[0]
age = params.get('age_py', ['unknown'])[0]
build_response_body(name, age)

