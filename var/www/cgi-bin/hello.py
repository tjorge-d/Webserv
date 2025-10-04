#!/usr/bin/env python3

import os
import sys
from urllib.parse import parse_qs

def get_params():
    method = os.environ.get('REQUEST_METHOD', 'GET')
    params = {}
    if method == 'GET':
        query = os.environ.get('QUERY_STRING', '')
        print(f"GET parameters: {params}", file=sys.stderr)
        params = parse_qs(query)
        print(f"GET parameters: {params}", file=sys.stderr)
    elif method == 'POST':
        content_length = int(os.environ.get('CONTENT_LENGTH', 0))
        post_data = sys.stdin.read(content_length)
        params = parse_qs(post_data)
    return params

def print_environ_to_stderr():
    for key, value in os.environ.items():
        print(f"{key}={value}", file=sys.stderr)

def build_response_headers():
    print(f"Content-Type: text/html, charset=UTF-8\r\n", end='')
    print(f"\r\n", end='')

def build_response_body(name, age):
    print(f"<html><body>")
    print(f"<h1>Hello, {name}!</h1>")
    print(f"<p>You are {age} years old.</p>")
    print(f"</body></html>")

# Example usage:
print_environ_to_stderr()
build_response_headers()
params = get_params()
name = params.get('name', ['Guest'])[0]
age = params.get('age', ['unknown'])[0]
build_response_body(name, age)

