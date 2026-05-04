## general evaluation objectives
*a mix of requirements from subject.pdf, from evaluating other webserv projects, and from discussing with other 42 members*

*there are also readme + compilation requirements, but those should be a given.*

---
### subject.pdf: general requirements

- Your program must use a configuration file, provided as an argument on the command line, or available in a default path.
- You cannot `execve` another web server.
- Your server must remain non-blocking at all times and properly handle client disconnections when necessary.
- It must be non-blocking and use only 1 poll() (or equivalent) for all the I/O operations between the clients and the server (listen included).
- poll() (or equivalent) must monitor both reading and writing simultaneously.
- You must never do a read or a write operation without going through poll() (or equivalent).
- Checking the value of errno to adjust the server behaviour is strictly forbidden after performing a read or write operation.
- You are not required to use poll() (or an equivalent function) for regular disk files; read() and write() on them do not require readiness notifications.
	- **<!>** I/O that can wait for data (sockets, pipes/FIFOs, etc.) must be non-blocking and driven by a single poll() (or equivalent).
	- **<!>** Calling read/recv or write/send on these descriptors without prior readiness will result in a grade of 0. Regular disk files are exempt.
- When using poll() or any equivalent call, you can use every associated macro or helper function (e.g., FD_SET for select()).
- A request to your server should never hang indefinitely.
- Your server must be compatible with standard web browsers of your choice.
- NGINX may be used to compare headers and answer behaviours 
	- Pay attention to differences between HTTP versions.
- Your HTTP response status codes must be accurate.
- Your server must have default error pages if none are provided.
- You can’t use fork for anything other than CGI (like PHP, or Python, and so forth).
- You must be able to serve a fully static website.
- Clients must be able to upload files.
- You need at least the GET, POST, and DELETE methods.
- Stress test your server to ensure it remains available at all times.
- Your server must be able to listen to multiple ports to deliver different content.
- **<!>** Offering only a subset of the HTTP RFC is **deliberate**. In this context, the virtual host feature is considered out of scope, but can be implemented if desired.

---
### subject.pdf: config file requirements

The config file must have the following:
	
- Define all the interface:port pairs on which your server will listen to (defining multiple websites served by your program)
- Set up default error pages.
- Set the maximum allowed size for client request bodies.
- Specify rules or configurations on a URL/route (no regex required here), for a website, among the following:
	- List of accepted HTTP methods for the route.
	- HTTP redirection.
	- Directory where the requested file should be located (e.g., if URL /kapouet is rooted to /tmp/www, URL /kapouet/pouic/toto/pouet will search for /tmp/www/pouic/toto/pouet).
	- Enabling or disabling directory listing.
	- Default file to serve when the requested resource is a directory.
	- Uploading files from the clients to the server is authorized, and storage location is provided.
	- Execution of CGI, based on file extension (for example .php). Here are some specific remarks regarding CGIs:
		- Do you wonder what a CGI is? 
			- ref:https://en.wikipedia.org/wiki/Common_Gateway_Interface
		- Have a careful look at the environment variables involved in the web server-CGI communication. The full request and arguments provided by the client must be available to the CGI.
		- Just remember that, for chunked requests, your server needs to un-chunk them, the CGI will expect EOF as the end of the body.
		- The same applies to the output of the CGI. If no content_length is returned from the CGI, EOF will mark the end of the returned data.
		- The CGI should be run in the correct directory for relative path file access.
		- Your server should support at least one CGI (php-CGI, Python, and so forth).

You must provide configuration files and default files to test and demonstrate that every feature works during the evaluation.

You can have other rules or configuration information in your file (e.g., a server name for a website if you plan to implement virtual hosts).

If you have a question about a specific behaviour, you can compare your program’s behaviour with NGINX’s.
There is a tester but it's not mandatory to be used, though it can help find and fix bugs.

---
### subject.pdf: bonus requirements

- Support cookies and session management (provide simple examples).
- Handle multiple CGI types.

---
### yap: evaluation mandatory requirements
- Launch the installation of siege with homebrew.
- Ask explanations about the basics of an HTTP server.
- Ask what function the group used for I/O Multiplexing.
- Ask for an explanation of how does select() (or equivalent) work.
- Ask if they use only one select() (or equivalent) and how they've managed the server to accept and the client to read/write.
- The select() (or equivalent) should be in the main loop and should check file descriptors for read and write AT THE SAME TIME. If not, the grade is 0.
- There should be only one read or one write per client per select() (or equivalent). Ask the group to show you the code from the select() (or equivalent) to the read and write of a client.
- Search for all read/recv/write/send on a socket and check that, if an error is returned, the client is removed.
- Search for all read/recv/write/send and check if the returned value is correctly checked (checking only -1 or 0 values is not enough, both should be checked).
- If errno is checked after read/recv/write/send, the grade is 0.
- Writing or reading ANY file descriptor without going through the select() (or equivalent) is strictly FORBIDDEN.

Configuration

In the configuration file, check whether you can do the following and
test the result:

- Search for the HTTP response status codes list on the internet. Ensure no status codes are wrong.
- Setup multiple servers with different ports.
- Setup multiple servers with different hostnames (use something like: curl --resolve example.com:80:127.0.0.1 http://example.com/).
- Setup default error page (try to change the error 404).
- Limit the client body (use: curl -X POST -H "Content-Type: plain/text" --data "BODY IS HERE write something shorter or longer than body limit").
- Setup routes in a server to different directories.
- Setup a default file to search for if you ask for a directory.
- Setup a list of methods accepted for a certain route (e.g., try to delete something with and without permission).

Basic checks

Using telnet, curl, prepared files, demonstrate that the following
features work properly:

- GET, POST and DELETE requests should work.
- UNKNOWN requests should not result in a crash.
- For every test you should receive the appropriate status code.
- Upload some file to the server and get it back.

Check CGI

Pay attention to the following:

- The server is working fine using a CGI.
- The CGI should be run in the correct directory for relative path file access.
- With the help of the students you should check that everything is working properly. You have to test the CGI with the "GET" and "POST" methods.
- You need to test with files containing errors to see if the error handling works properly. You can use a script containing an infinite loop or an error; you are free to do whatever tests you want within the limits of acceptability that remain at your discretion. The group being evaluated should help you with this.

Check with a browser

- Use the reference browser of the team. Open the network part of it, and try to connect to the server using it.
- Look at the request header and response header.
- It should be compatible to serve a fully static website.
- Try a wrong URL on the server.
- Try to list a directory.
- Try a redirected URL.
- Try anything you want to.

Port issues

- In the configuration file setup multiple ports and use different websites. Use the browser to ensure that the configuration works as expected and shows the right website.
- In the configuration, try to setup the same port multiple times. It should not work.
- Launch multiple servers at the same time with different configurations but with common ports. Does it work? If it does, ask why the server should work if one of the configurations isn't functional. Keep going.

Siege & stress test

- Use Siege to run some stress tests.
- Availability should be above 99.5% for a simple GET on an empty page with a siege -b on that page.
- Verify there is no memory leak (Monitor the process memory usage. It should not go up indefinitely).
- Check if there is no hanging connection.
- You should be able to use siege indefinitely without having to restart the server (take a look at siege -b).

---
### yap: evaluation bonus requirements

Cookies and session

- There is a working session and cookies system on the webserver.

CGI

- There is more than one CGI system.