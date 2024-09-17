#!/usr/bin/env python3

import os
import cgi
import cgitb
import json

print("HELLOOOO")

# Enable debugging
cgitb.enable()

# Content-Type header for the HTML response
print("Content-Type: text/html\n")

# Determine the request method (GET, POST, DELETE)
request_method = os.environ.get('REQUEST_METHOD')

# File to store data (comments or similar content)
data_file = "/path/to/your/data_file.txt"

# Function to read the data file and return content
def read_data():
    try:
        with open(data_file, 'r') as file:
            return file.read()
    except FileNotFoundError:
        return "No data found."

# Function to write data to the file (POST request)
def write_data(content):
    with open(data_file, 'a') as file:
        file.write(content + "\n")

def printGET():
    print("Content-Type: text/html\n")

# Initialize form data
form = cgi.FieldStorage()

# List to store comments (this is in-memory and will reset with every script execution)
comments = []

# Check if the form was submitted and a comment was provided
if "comment" in form:
    comment = form.getvalue("comment")
    if comment:
        comments.append(comment)

# HTML form and display of comments
html = """
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Comment Upload</title>
</head>
<body>
    <h1>Upload a Comment</h1>
    <form action="/cgi-bin/upload_comment.py" method="post">
        <label for="comment">Enter your comment:</label><br>
        <textarea id="comment" name="comment" rows="4" cols="50"></textarea><br><br>
        <input type="submit" value="Submit Comment">
    </form>
    <h2>Comments</h2>
    <ul>
"""

# Display submitted comments
for comment in comments:
    html += f"<li>{comment}</li>"

html += """
    </ul>
</body>
</html>
"""

# Output the HTML
print(html)
# Function to delete data from the file (DELETE request)
def delete_data():
    try:
        os.remove(data_file)
        return "Data deleted successfully."
    except FileNotFoundError:
        return "No data to delete."

# Handling different HTTP methods
if request_method == 'GET':
    # Handle GET request: Retrieve and display the stored data
    printGET();

elif request_method == 'POST':
    # Handle POST request: Add new data (e.g., a comment)
    form = cgi.FieldStorage()
    new_comment = form.getvalue('comment', '')

    if new_comment:
        write_data(new_comment)
        print("<html><body><h1>Comment Added</h1></body></html>")
    else:
        print("<html><body><h1>No comment provided</h1></body></html>")

elif request_method == 'DELETE':
    # Handle DELETE request: Remove the stored data
    print("<html><body>")
    print("<h1>{}</h1>".format(delete_data()))
    print("</body></html>")

else:
    # If the request method is not supported
    print("<html><body><h1>Method Not Supportedddddddddddddddd</h1></body></html>")
