#include "WUtils.hpp"

#include "unistd.h"

// ----------------------------------------------------------------------------

void	WUtils::safeClose(int &fd)
{
	if (fd == -1)
		return;
	close(fd);
	fd = -1;
}
