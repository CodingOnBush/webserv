#!/usr/bin/env python3

# Output HTTP headers
print("Content-Type: text/html")
print()  # End headers

# Output HTML content with embedded JavaScript
print("""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>My Static Website</title>
    <style>
        body { font-family: Arial, sans-serif; }
        h1 { color: #333; }
        p { font-size: 18px; }
    </style>
</head>
<body>
    <h1>Welcome to My Static Website!</h1>
    <p>This is a static page served by a Python CGI script.</p>
    <button id="changeTextButton">Change Text</button>
    <p id="dynamicText">This text will change when you click the button.</p>
    <script>
        document.getElementById('changeTextButton').addEventListener('click', function() {
            document.getElementById('dynamicText').textContent = 'The text has been changed!';
        });
    </script>
</body>
</html>
""")

