# Instructions from the subject
## Uderstand the subject
**_Choose the port and host of each ’server’._**
- server block to be able to have several servers in the configuration file.\
- listen directive to choose the port and host of each server (inside each server block).

**_Setup the server_names or not._**
- server_name directive could be used inside each server blocks.

**_The first server for a host:port will be the default for this host:port (that means it will answer to all the requests that don’t belong to an other server)._**
- The first server block for a specific port:ip will be the default server block for that port:ip.
- But I don't understand why ?!

**_Setup default error pages._**
- one error_page directive or more could be used inside each server block.

**_Limit client body size._**
- client_max_body_size directive could be used inside each server block to limit the client body size.

**Setup routes with one or multiple of the following rules/configuration (routes wont
be using regexp):**

**_Define a list of accepted HTTP methods for the route._**
- set_method directive will be used inside each server block to define a list of accepted HTTP methods for the route.

**_Define a HTTP redirection._**
- use the return directive to define a HTTP redirection.
```
location /old-route {
	return 301 /new-route;
}
```

**_Define a directory or a file from where the file should be searched_**
- for example, if url /kapouet is rooted to /tmp/www, url /kapouet/pouic/toto/pouet is /tmp/www/pouic/toto/pouet
- The root directive is used to specify the directory from which files will be served:
```
location / {
	root /var/www/html;
}
```

**_Turn on or off directory listing_**
- To control directory listing, use the autoindex directive
```
location /files {
	autoindex on;
}
```

**_Set a default file to answer if the request is a directory_**
- Use the index directive to specify the default file.
- This configuration will serve index.html when a directory is requested.
```
location / {
	index index.html;
}
```

**_Execute CGI based on certain file extension (for example .php)._**
- Use the cgi directive like this
```
location /python/ {
	cgi .py /usr/bin/python3;
}
```

**_Make it work with POST and GET methods_**
- we can use set_method directive inside a location block to define a list of accepted HTTP methods for the route.

**_Make the route able to accept uploaded files and configure where they should be saved._**
- Use the upload_store directive to configure where the uploaded files should be saved.

**_Because you won’t call the CGI directly, use the full path as PATH_INFO._**
