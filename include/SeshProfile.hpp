#ifndef SESHPROFILE_HPP
# define SESHPROFILE_HPP

# include "HttpRequest.hpp"
# include <string>

namespace SeshProfile
{
	void	post_update(const std::string &post_body, sesh_data &sesh);
	void	dynamic_fill(std::string &resp_body, sesh_data &sesh);
};


#endif
