#!/usr/bin/env python3

import os
import sys
from urllib.parse import parse_qs

def get_params():
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    post_data = sys.stdin.read(content_length) if content_length > 0 else ""
    params = parse_qs(post_data)
    return params

def get_theme_from_cookie():
    cookie = os.environ.get('HTTP_COOKIE', '')
    cookies = {}
    for item in cookie.split(';'):
        if '=' in item:
            key, value = item.strip().split('=', 1)
            cookies[key] = value
    return cookies.get('theme', 'light')  # Default theme is 'light'

def build_response_headers():
    print(f"Content-Type: text/html\r\n", end='')
    print(f"\r\n", end='')

def build_response_body(city, country):
    if theme == 'dark':
        bg_color = "#121212"
        text_color = "#FFFFFF"
    else:
        bg_color = "#FFFFFF"
        text_color = "#000000"

    print(f"<!DOCTYPE html>")
    print(f"<html>")
    print(f"<head>")
    print(f"<title>Hello Page</title>")
    print(f"<style>")
    print(f"body {{ background-color: {bg_color}; color: {text_color}; font-family: Arial, sans-serif; }}")
    print(f"</style>")
    print(f"</head>")
    print(f"<body>")
    print(f"<h1>POST data recieved</h1>")
    print(f"<p><b>City:</b> {city}</p>")
    print(f"<p><b>Country:</b> {country}</p>")
    print(f"</body>")
    print(f"</html>")

build_response_headers()
params = get_params()
# while (True):
#     print("Infinity")
city = params.get('city', [''])[0]
country = params.get('country', [''])[0]
theme = get_theme_from_cookie()
build_response_body(city, country)
