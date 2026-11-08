#ifndef SESHCTL_HPP
# define SESHCTL_HPP

# include <string>
# include <ctime>
# include <map>

class Server;

struct sesh_data
{
	std::string		sid;
	std::string		name;
	std::string		fav_food;
	int				profile_visits;
	std::time_t		last_time_active;
	int				activeCGI;				//increment CGI start, decrement when end
	bool			profile_updated;

	sesh_data() : name("Mr. Guest"), profile_visits(), 
	last_time_active(-1), activeCGI(0),  profile_updated(false){}
};

class SeshCtl
{
private:
	static const int	_timeout = 3600;
	const Server &		_server;
	std::map<std::string, sesh_data>	_sessions;

	static bool			is_expired(std::time_t time_now, std::time_t last_time_active);

	SeshCtl();
	SeshCtl(const SeshCtl & other);
	SeshCtl &	operator=(const SeshCtl & other);

public:
	SeshCtl(const Server &serv);
	~SeshCtl();

	sesh_data &	create();
	sesh_data *	retrieve(std::string sid);
	void		clear_expired();

	//Cookie helpers
	static std::string		set_cookie_header(std::string sid);
	static std::string		get_header_sid(std::string cookie_val);
};

#endif
