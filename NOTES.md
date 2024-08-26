# Instructions from the subject
<ins>Tips : You can get some inspiration from the ’server’ part of NGINX configuration file.</ins>\
_I can search what are the rules in the nginx configuration file only for the server part._

**In the configuration file, you should be able to :**
- Choose the port and host of each ’server’.\
_A port is a number on which the server will listen for incoming requests. A host is the IP address on which the server will listen for incoming requests. And when I read "each server", I think it means that we can have multiple servers in the configuration file._
- Setup the server_names or not.
- The first server for a host:port will be the default for this host:port (that means it will answer to all the requests that don’t belong to an other server).
- Setup default error pages.
- Limit client body size.
- Setup routes with one or multiple of the following rules/configuration (routes wont be using regexp) :
	- Define a list of accepted HTTP methods for the route
	- Define a HTTP redirection
	- Define a directory or a file from where the file should be searched (for example, if url /kapouet is rooted to /tmp/www, url /kapouet/pouic/toto/pouet is /tmp/www/pouic/toto/pouet).
	- Turn on or off directory listing.
	- Set a default file to answer if the request is a directory.
	- Execute CGI based on certain file extension (for example .php).
	- Make it work with POST and GET methods.
	- Make the route able to accept uploaded files and configure where they should be saved.

## Rules are rules

We don't have to implement all the nginx rules, but we have to choose rules that match with each requirement of the subject.\
I found the readme.md from a friend who did the project, and listed all the rules that we have to check in the configuration file :
- conf file must include at least one server directive.\
All the possible directives in the server block : listen, server_name, root, error_page, client_max_body_size, location\
All the possible directives in the location block : root, alias, client_max_body_size, autoindex, index, return, path_info, cgi, upload_location, set_method

```bash
# Example of a server block
server {
	listen 80;
	server_name localhost;
	root /var/www/html;
	error_page 404 /404.html;
	client_max_body_size 10M;

	location / {
		root /var/www/html;
		autoindex on;
		index index.html;
	}

	location /upload {
		upload_location /var/www/html/upload;
	}
}
```