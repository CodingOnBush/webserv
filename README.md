# Webserv
<img width="446" alt="Screenshot 2024-04-13 222920" src="www/Screenshot 2024-09-09 105335.png">

## Overview

This project is part of 42 school common core cursus. Webserv is a custom HTTP server inspired by [NGINX](https://github.com/nginx/nginx) and designed to handle HTTP requests and responses, including support for CGI scripts, file uploads, and multiple HTTP methods (GET, POST, DELETE). The project aims to provide a fully functional web server with configurable settings and error handling.

## Features

- **HTTP Methods**: Supports GET, POST and DELETE methods. Sending other types of request will result in 405 Method Not Allowed Error.
- **File Uploads**: Handles file uploads to specified directories.
- **CGI Support**: Executes CGI scripts for dynamic content.
- **Configuration**: Customizable server settings via configuration files.
- **Error Handling**: Default error pages for various HTTP status codes. Custom error pages can be defined in Config file.
- **Multiple Ports**: Listens on multiple ports as specified in the configuration.

## Directory Structure

- **bin/**: Compiled object files
- **cgi-bin/**: CGI scripts
- **config/**: Configuration files
- **include/**: Header files
- **src/**: Source files
- **www/**: Web content

## Configuration

The server configuration is specified in `.conf` files located in the `config` directory. The default configuration file is `default.conf`.
### Directives
| Directive            | Syntax                 | Example(s)                              | Default      | Context          | Description                                                                                                      |
|---|---|---|---|---|---|
|Listen| `listen` | `listen localhost:8080;`, `listen 127.0.0.1:8081;`, `listen 8082;` | `localhost:8080` |  `server`| Sets host address and port for IP on which the server will accept requests. Both address and port, or only address or only port can be specified. An address may also be a hostname. | 
| Server name          | `server_name`          | `server_name example.com;`           | -            | `server`         | Sets names of a virtual server.  The first name becomes the primary server name.                                                                                |
|Root| `root` | `root ./www;` | `./www`|  `server`, `location` |Sets the root directory for requests.|
| Default error pages           | `error_page`           | `error_page 404 /404.html; error_page 403 /403.html;`          | -            | `server`, `location` | Defines the URI to redirect to in case of a specified error code.                                                              |
| Client body size limit    | `client_max_body_size` | `client_max_body_size 1k;`          | `client_max_body_size 1m;`          | `server`, `location` | Sets the maximum allowed size of the client request body.                                                        |
| Allowed HTTP methods | `allowed_methods` | `allowed_methods GET\|POST\|DELETE;`     | -            | `server`, `location`       | Defines a list of accepted HTTP methods for the route.|                                                   |
| Directory listing           | `autoindex`            | `autoindex on;`                      | `off`        | `server`, `location` | Turns on or off directory listing.                                                       |
| HTTP redirection          | `return`               | `return 301 http://example.com;`     | -            | `server`, `location` | Stops processing and redirects to a specified source, returning a specified code to the client.                  |
| Index file               | `index`                | `index new_index.html;`        | `index.html` | `server`, `location` | Sets a default file to answer if the request is a directory.                                     |
| Alias                | `alias`                | `alias /var/www/html;`               | -            | `location`       | Defines a directory or a file from where the file should be searched (a replacement for the specified location).                                            |
| CGI                  | `cgi`                  | `cgi .py .php;`           | -            | `server`, `location` | Specifies file extension(s) based on which CGI script can be executed for a given location.                                                    |
|Uploaded files location | `upload_location` | `upload_location ./www/upload;` | `./www/upload`| `location` | Makes the route able to accept uploaded files and configures where they should be saved. |

### Example Configuration

```conf
server {
    listen 8080;
    server_name localhost;
	client_max_body_size 1k;

    location / {
		allowed_methods GET|POST|DELETE;
        root ./www;
		autoindex on;
    }

    location /upload {
		allowed_methods POST|DELETE;
		upload_location ./www/upload;
	}
}
```

## Building the Project

To build the project, use the provided Makefile. Run the following command in the project root directory:

```sh
make
```

This will compile the source files and generate the `webserv` executable.

## Running the Server

To run the server with the default configuration:

```sh
./webserv
```

To specify a custom configuration file:

```sh
./webserv path/to/config.conf
```

## Handling Requests

### GET Request

Handles static files and directory listings (if autoindex is enabled).

### POST Request

Handles form submissions and file uploads.

### DELETE Request

Deletes specified files from the server.

## Error Handling

Custom error pages can be specified in the configuration file. Default error pages are provided for common HTTP status codes.

## CGI Support

CGI scripts can be executed for dynamic content. The server will handle the execution of the script and return the output as the response.

## Authors

- [Alisa Tonkopiy](https://github.com/a-dylean)
- [Mostafa Omrane](https://github.com/CodingOnBush)
- [Victoire Vaudaine](https://github.com/vicvdn)

## Additional Resources

For more information, refer to the [Notion page](https://www.notion.so/Webserv-e0b101573b614f959497d4856d97e59c).

---



