#!/usr/bin/env python3

import os
import cgi
import cgitb
import json

# Enable debugging
cgitb.enable()

# Content-Type header for the HTML response
print("Content-Type: text/html\n")

# Determine the request method (GET, POST, DELETE)
request_method = os.environ.get('REQUEST_METHOD')

# Initialize form data
form = cgi.FieldStorage()

print("METHOD")
print(request_method)
if request_method == '0':
    print("<html><body><h1>Received get</h1></body></html>")


elif request_method == '1':
    print("<html><body><h1>Comment Added</h1><h2>")
    print("</h2></body></html>")

elif request_method == '2':
    print("<html><body>")
    print("<h1>DELETE</h1>")
    print("</body></html>")

else:
    print("<html><body><h1>Method Not Supported</h1></body></html>")
