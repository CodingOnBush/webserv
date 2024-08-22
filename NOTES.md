# Configuration file from the subject
In the configuration file, you should be able to :
- Choose the port and host of each ’server’.
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