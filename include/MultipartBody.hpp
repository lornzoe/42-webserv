#ifndef MULTIPARTBODY_HPP
# define MULTIPARTBODY_HPP

# include <string>
# include <vector>
# include <map>

class MultipartBody
{
public:
	MultipartBody(std::string const &body, std::string const &bound);

	typedef	std::map<std::string, std::string>		map_strstr;
	struct part {
		bool			is_file;
		map_strstr		fdata;		//Content-Disposition's form-data fields
		std::string		cont_type;
		std::string		body;

		part() : is_file() {}
	};

	// Returns size of file parts in the multipart body
	size_t			fileCount() const { return _file_idx.size(); }

	/** Returns struct of one file body part
	 * Use with fileCount to iterate over all file parts
	 * access file name e.g. p.fdata.find("filename")->second;
	 * access body e.g. p.body
	 */
	part const &	filePart(size_t indx) const { return _parts[_file_idx[indx]]; }

private:
	typedef	std::pair<std::string, std::string>		kvpair;

	enum t_bound {
		B_END,
		B_CONT,
		B_ERR
	};

	std::string const &	_http_body;
	std::string			_bound;
	std::vector<part>	_parts;
	std::vector<size_t>	_file_idx;

	t_bound		consmBound(size_t &cur);
	bool		consmHeads(size_t &cur, part &p);
	bool		fill_contDisp(std::string const &content, part &p);
	bool		consmBody(size_t &cur, part &p);
};

#endif
