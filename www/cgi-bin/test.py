#!/usr/bin/env python3
import os
import cgi
import cgitb
import json

request_method = os.environ.get('REQUEST_METHOD')
request_query = os.environ.get('QUERY_STRING')
html_content = """<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Comment Form</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 20px;
        }
        form {
            margin-top: 20px;
        }
        label {
            display: block;
            margin: 8px 0 4px;
        }
        input[type="text"], textarea {
            width: 100%;
            padding: 10px;
            margin-bottom: 10px;
            border: 1px solid #ccc;
            border-radius: 4px;
        }
        input[type="submit"] {
            background-color: #4CAF50;
            color: white;
            padding: 10px 15px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
        }
        input[type="submit"]:hover {
            background-color: #45a049;
        }
    </style>
</head>
<body>

<h1>Leave a Comment</h1>

<form action="/submit_comment" method="post">
    <label for="name">Name:</label>
    <input type="text" id="name" name="name" required>
    
    <label for="comment">Comment:</label>
    <textarea id="comment" name="comment" required></textarea>
    
    <input type="submit" value="Submit Comment">
</form>"""

def parse_query_string(query_string):
    pairs = query_string.split('&')
    name = None
    comment = None
    for pair in pairs:
        key, value = pair.split('=')
        if key == 'name':
            name = value
        elif key == 'comment':
            comment = value
            
    return name, comment

def printGet():

    print()
    print(html_content)
    print("</body></html>")

def printPost():
    name, comment = parse_query_string(request_query)
    print()
    print(html_content)
    print("<h2>Comment Submitted</h2>")
    print("<p>Comment:")
    print("<p>")
    print(f"Name: {name}")
    print(f"Comment: {comment}")
    print("</p>")
    print("""<form action="/delete_comment" method="delete" style="display:inline;">
    <input type="hidden" name="index" value="{idx}">
    <input type="submit" value="Delete Comment" 
    style="background-color: red; color: white; border: none; border-radius: 4px; cursor: pointer;">
    </form>""")
    print("</body></html>")
    

if request_method == '0':
    printGet()
elif request_method == '1':
    printPost()
else:
    print("<html><body><h1>Method Not Supported</h1></body></html>")