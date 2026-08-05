#include "w_utils.hpp"

#include "unistd.h"
#include <string>
#include <sstream>

// ----------------------------------------------------------------------------

void	wutils::safeClose(int &fd)
{
	if (fd == -1)
		return;
	close(fd);
	fd = -1;
}

std::string		wutils::ft_itoa(int num)
{
	std::stringstream	ss;
	ss << num;
	return (ss.str());
}
