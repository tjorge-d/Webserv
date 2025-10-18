#!/usr/bin/env python3

import os
from urllib.parse import parse_qs

def get_params():
    query = os.environ.get('QUERY_STRING', '')
    params = parse_qs(query)
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

def build_response_body(name, age, theme):
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
    print(f"<h1>Hello, {name}!</h1>")
    print(f"<p>You are {age} years old.</p>")
    print(f"<p>Current theme: {theme}</p>")
    print(f"</body>")
    print(f"</html>")

# --- Main CGI logic ---
build_response_headers()
params = get_params()
name = params.get('name_py', ['Guest'])[0]
age = params.get('age_py', ['unknown'])[0]
theme = get_theme_from_cookie()
build_response_body(name, age, theme)
