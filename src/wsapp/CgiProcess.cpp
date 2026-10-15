#include "CgiProcess.hpp"

CgiProcess::CgiProcess()
{
}

void CgiProcess::start()
{
	if (!_parent_to_child.create() || !_child_to_parent.create())
	{
		throw Pipe::PipeException();
	}

	pid_t pid = fork();
	if (pid < 0)
	{
		throw ForkException();
	}

	// Child process
	if (pid == 0)
	{
		if (true)
			_exit(1);

		if (dup2(_parent_to_child.getReadFd(), STDIN_FILENO) == -1 ||
			dup2(_child_to_parent.getWriteFd(), STDOUT_FILENO) == -1)
		{
			_exit(1);
		}

		_parent_to_child.closeAll();
		_child_to_parent.closeAll();

		// TODO: resolve the CGI script path using server's configuration
		// and the incoming request
		char *argv[3];
		argv[0] = const_cast<char *>("/usr/bin/python3");
		argv[1] = const_cast<char *>("scripts/cgi.py");
		argv[2] = NULL;

		// TODO:
		char *envp[3];
		envp[0] = const_cast<char *>("MY_VAR1=env1");
		envp[1] = const_cast<char *>("MY_VAR2=env2");
		envp[2] = NULL;

		execve(argv[0], argv, envp);

		_exit(1);
	}
	// Parent process.
	else
	{
		_parent_to_child.closeRead();
		_child_to_parent.closeWrite();
	}

	// // Parent process
	// else
	// {
	// 	parent_to_child.closeRead();
	// 	child_to_parent.closeWrite();

	// 	// TODO: Write to parent_to_child_pipes[1]
	// 	parent_to_child.closeWrite();

	// 	// Read the CGI script's output.
	// 	std::string cgi_output;
	// 	char buffer[1024];
	// 	ssize_t bytes_read;

	// 	while ((bytes_read = read(
	// 				child_to_parent.getReadFd(),
	// 				buffer,
	// 				sizeof(buffer))) > 0)
	// 	{
	// 		cgi_output.append(buffer, bytes_read);
	// 	}

	// 	if (bytes_read < 0)
	// 	{
	// 		std::cerr << "Error reading CGI output" << std::endl;
	// 	}

	// 	child_to_parent.closeRead();

	// 	LOG_DEBUG(cgi_output);

	// 	std::string::size_type header_end = cgi_output.find("\r\n\r\n");

	// 	if (header_end == std::string::npos)
	// 	{
	// 		// Invalid or incomplete CGI response.
	// 	}
	// 	std::string cgi_headers = cgi_output.substr(0, header_end);
	// 	std::string cgi_body = cgi_output.substr(header_end + 4);

	// 	LOG_DEBUG(cgi_headers);
	// 	LOG_DEBUG(cgi_body);

	// 	// TODO: this needs to be changed as it is blocking
	// 	int status;
	// 	waitpid(pid, &status, 0);

	// 	if (WIFEXITED(status))
	// 	{
	// 		std::cout << "CGI exit code: "
	// 				  << WEXITSTATUS(status) << std::endl;
	// 	}
	// }
}

const char *CgiProcess::ForkException::what() const throw()
{
	return "CgiProcess::ForkException.";
}
