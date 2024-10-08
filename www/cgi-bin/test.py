#!/usr/bin/env python3
import os
import cgi
import cgitb
import json

# while True:
#     pass

print("Hi from test.py")
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
            width: 90%;
            padding: 15px;
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
        .comment-block {
            border: 1px solid #ccc;
            border-radius: 4px;
            padding: 15px;
            margin-top: 20px;
            background-color: #f9f9f9;
        }
    </style>
</head>
<body>

<h1>Leave a Comment</h1>

<div class="comment-block">
    <form action="/submit_comment" method="post">
        <label for="name">Name:</label>
        <input type="text" id="name" name="name" required>
        
        <label for="comment">Comment:</label>
        <textarea id="comment" name="comment" required></textarea>
        
        <input type="submit" value="Submit Comment">
    </form>
</div>
"""

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
    print("<h2>Comment Submitted:</h2>")
    print("""
    <div style="border: 1px solid #ccc; border-radius: 4px; padding: 15px; margin-top: 20px; background-color: #f9f9f9;">
        <strong>Name:</strong> {}
        <p><strong>Comment:</strong></p>
        <p>{}</p>
        <form action="/delete_comment" method="post" style="display:inline;">
            <input type="hidden" name="name" value="{name}">
            <input type="hidden" name="comment" value="{comment}">
        </form>
    </div>
    """.format(name, comment, name=name, comment=comment))
    print("</body></html>")
    
if request_method == '0':
    printGet()
elif request_method == '1':
    printPost()
else:
    print("<html><body><h1>Method Not Supported</h1></body></html>")
print("Content-Type: text/html")
# to test for infinite loop
# while True:
#     print("hello")