#include "SeshProfile.hpp"
#include "SeshCtl.hpp"
#include "Utils.hpp"

#include <vector>
#include <map>

namespace {

	int	hex_value(char c)
	{
		if (c >= '0' && c <= '9')
			return c - '0';
		if (c >= 'a' && c <= 'f')
			return c - 'a' + 10;
		if (c >= 'A' && c <= 'F')
			return c - 'A' + 10;
		return -1;
	}

	std::string	url_decode(std::string const &str)
	{
		std::string	out;
		for (size_t i = 0; i < str.size(); ++i)
		{
			if (str[i] == '+')
				out += ' ';
			else if (str[i] == '%' && i + 2 < str.size())
			{
				int hi = hex_value(str[i + 1]);
				int lo = hex_value(str[i + 2]);

				if (hi != -1 && lo != -1)
				{
					out += static_cast<char>(hi * 16 + lo);
					i += 2;
				}
				else
					out += str[i];
			}
			else
				out += str[i];
		}
		return out;
	}

	std::string	html_escape(std::string const &str)
	{
		std::string	out;
		for (size_t i = 0; i < str.size(); ++i)
		{
			switch (str[i])
			{
				case '&':	out += "&amp;";	break;
				case '<':	out += "&lt;";	break;
				case '>':	out += "&gt;";	break;
				case '"':	out += "&quot;";	break;
				case '\'':	out += "&#39;";	break;
				default:	out += str[i];
			}
		}
		return out;
	}

	void	replace_all(std::string &body, std::string const &token, std::string const &value)
	{
		size_t	pos = 0;

		while ((pos = body.find(token, pos)) != std::string::npos)
		{
			body.replace(pos, token.size(), value);
			pos += value.size();
		}
	}

	std::map<std::string, std::string>	make_foods()
	{
		std::map<std::string, std::string> foods;
		foods["Pizza"] = "pizza.jpeg";
		foods["Salad"] = "salad.jpeg";
		foods["Burger"] = "burger.jpeg";
		foods["Sushi"] = "sushi.jpeg";
		foods["Pasta"] = "pasta.webp";
		foods["Chicken Rice"] = "chicken_rice.jpeg";
		foods["Steak"] = "steak.jpeg";
		foods["Tacos"] = "tacos.jpeg";
		foods["Ice Cream"] = "ice_cream.jpeg";
		foods["Durian"] = "durian.jpeg";
		return foods;
	}

	std::map<std::string, std::string> const	&food_map()
	{
		static const std::map<std::string, std::string> foods = make_foods();
		return foods;
	}

}

namespace SeshProfile {

	void	post_update(const std::string &post_body, sesh_data &sesh)
	{
		std::vector<std::string> fields = Utils::ft_split(post_body, "&", true);

		for (std::vector<std::string>::iterator it = fields.begin(); it != fields.end(); ++it)
		{
			Utils::kvpair kv;
			if (!Utils::split_pair(*it, '=', kv))
				continue;

			std::string key = url_decode(kv.first);
			std::string val = url_decode(kv.second);

			// Empty means "leave unchanged".
			if (key == "name")
			{
				if (!val.empty())
					sesh.name = val;
			}
			else if (key == "fav_food")
			{
				if (!val.empty() && food_map().find(val) != food_map().end())
					sesh.fav_food = val;
			}
		}
		sesh.profile_updated = true;
	}

	void	dynamic_fill(std::string &resp_body, sesh_data &sesh)
	{
		sesh.profile_visits++;
		std::map<std::string, std::string> const &foods =
			food_map();

		// Template replacement: Food options -------------------------
		std::stringstream	options;
		options
			<< "<option value=\"\" selected>"
			<< "Choose one..."
			<< "</option>\n";

		std::map<std::string, std::string>::const_iterator it;

		for (it = foods.begin(); it != foods.end(); ++it)
		{
			options
				<< "<option value=\""
				<< html_escape(it->first)
				<< "\">"
				<< html_escape(it->first)
				<< "</option>\n";
		}

		// Template replacement: Fav food -----------------------------
		std::string	image;

		it = foods.find(sesh.fav_food);
		if (it != foods.end())
		{
			image =
				"<img class=\"food-image\" src=\"/profile_pics/"
				+ it->second
				+ "\" alt=\""
				+ html_escape(it->first)
				+ "\">";
		}

		// Template replacement: Profile visits -----------------------
		std::stringstream	visits;
		visits << sesh.profile_visits;

		// Template replacement: Update message -----------------------
		std::string	updated_message;

		if (sesh.profile_updated)
		{
			updated_message = "<p class=\"updated\">Profile updated!</p>";
			sesh.profile_updated = false;
		}

		// Fill templates
		replace_all(resp_body, "{{SID}}", html_escape(sesh.sid));
		replace_all(resp_body, "{{NAME}}", html_escape(sesh.name));
		replace_all(resp_body, "{{FAV_FOOD}}", sesh.fav_food.empty() ? "Not selected" : html_escape(sesh.fav_food));
		replace_all(resp_body, "{{PROFILE_VISITS}}", visits.str());
		replace_all(resp_body, "{{UPDATED_MESSAGE}}", updated_message);
		replace_all(resp_body, "{{FOOD_IMAGE}}", image);
		replace_all(resp_body, "{{FOOD_OPTIONS}}", options.str());
	}
}
