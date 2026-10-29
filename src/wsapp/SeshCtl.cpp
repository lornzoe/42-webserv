#include "SeshCtl.hpp"
#include <cstdlib>

// OCF ------------------------------------------------------------------------

SeshCtl::SeshCtl(const Server &serv)
: _server(serv)
{
	std::srand(time(NULL));
}

SeshCtl::~SeshCtl() {}

// ----------------------------------------------------------------------------

sesh_data &	SeshCtl::create()
{
	std::string	sid;
	char	c;
	do {
		sid = "";
		for (int i = 0; i < 12; i++)
		{
			if (i < 6)
				c = 'a' + std::rand() % 26;
			else
				c = '0' + std::rand() % 10;
			sid += c;
		}
	}	while (_sessions.find(sid) != _sessions.end());

	sesh_data	data;

	data.sid = sid;
	data.last_time_active = time(NULL);
	return (_sessions[sid] = data);
}

bool		SeshCtl::is_expired(std::time_t time_now, std::time_t last_time_active)
{
	return (std::difftime(time_now, last_time_active) >= _timeout);
}

sesh_data *	SeshCtl::retrieve(std::string sid)
{
	std::map<std::string, sesh_data>::iterator	it;
	it = _sessions.find(sid);
	if (it == _sessions.end())
		return NULL;

	std::time_t	time_now = std::time(NULL);
	if (is_expired(time_now, it->second.last_time_active))
	{
		_sessions.erase(it);
		return NULL;
	}

	it->second.last_time_active = time_now;
	return &it->second;
}

void	SeshCtl::clear_expired()
{
	std::map<std::string, sesh_data>::iterator	it = _sessions.begin();
	std::map<std::string, sesh_data>::iterator del_it;
	std::time_t		time_now = std::time(NULL);

	while (it != _sessions.end())
	{
		if (is_expired(time_now, it->second.last_time_active))
		{
			del_it = it;
			it++;
			_sessions.erase(del_it);
		}
		else
			it++;
	}
}
