#include "MultipartBody.hpp"
#include "Utils.hpp"

#include <stdexcept>

// ----------------------------------------------------------------------------

MultipartBody::MultipartBody(std::string const &body, std::string const &bound)
: _http_body(body), _bound(bound)
{
	t_bound	ret;
	size_t	cur = 0;
	while ((ret = consmBound(cur)) == B_CONT)
	{
		part p;
		if (!consmHeads(cur, p) || !consmBody(cur, p))
		{ ret = B_ERR; break; }
		_parts.push_back(p);
		if (p.is_file)
			_file_idx.push_back(_parts.size() - 1);
	}
	if (ret == B_ERR)
		throw	std::runtime_error("Bad multipart body\n");
}

// ----------------------------------------------------------------------------

MultipartBody::t_bound		MultipartBody::consmBound(size_t &cur)
{
	std::string		full_bound = "--" + _bound;

	if (_http_body.compare(cur, full_bound.size(), full_bound) == 0)
	{
		cur += full_bound.size();
		if (_http_body.compare(cur, 2, "\r\n") == 0)
		{
			cur += 2;
			return B_CONT;
		}
		else if (_http_body.compare(cur, 4, "--\r\n") == 0)
		{
			cur += 4;
			return B_END;
		}
	}
	return B_ERR;
}

bool		MultipartBody::consmHeads(size_t &cur, part &p)
{
	size_t			clrf_idx;
	size_t			hdrSize;
	std::string		hdrStr;
	kvpair			hdrPair;
	size_t			hdrCount = 0;
	bool			has_x2clrf = 0;
	bool			has_cDisp = 0;

	while ((clrf_idx = _http_body.find("\r\n", cur)) != std::string::npos)
	{
		if (clrf_idx == cur)
		{ has_x2clrf = true; cur += 2; break; }

		hdrSize = clrf_idx - cur;
		hdrStr = _http_body.substr(cur, hdrSize);
		cur = clrf_idx + 2;

		if (!Utils::split_pair(hdrStr, ':', hdrPair))
			return false;
		if (Utils::toLowercase(hdrPair.first) == "content-disposition")
		{
			has_cDisp = true;
			if (!fill_contDisp(hdrPair.second, p))
				return false;
		}
		else if (Utils::toLowercase(hdrPair.first) == "content-type")
			p.cont_type = Utils::Trim(hdrPair.second);

		hdrCount++;
	}

	if (!(hdrCount > 0 && has_x2clrf) || !has_cDisp)
		return false;
	return true;
}

bool	MultipartBody::consmBody(size_t &cur, part &p)
{
	size_t			end_idx;
	std::string		endMarker = "\r\n--" + _bound;
	size_t			bodSize;
	std::string		bodStr;

	if ((end_idx = _http_body.find(endMarker, cur)) == std::string::npos)
		return false;
	bodSize = end_idx - cur;
	bodStr = _http_body.substr(cur, bodSize);
	cur = end_idx + 2;
	p.body = bodStr;
	return true;
}

namespace {
	//Lower case comparison
	bool			accept(std::string const &str, std::string const &content, size_t &cur)
	{
		size_t	strSize = str.size();
		if (Utils::toLowercase(content.substr(cur, strSize)) == str)
		{
			cur += strSize;
			return true;
		}
		return false;
	}

	void			consmOWS(std::string const &content, size_t &cur)
	{
		while (cur < content.size() && Utils::canFind(content[cur], OWS))
			cur++;
	}

	std::string		consmStr(std::string const &content, size_t &cur, size_t &offset)
	{
		std::string		subStr;
		size_t 			size = offset - cur;
		subStr = content.substr(cur, size);
		cur = offset;
		return subStr;
	}

	bool			consmKey(std::string const &content, size_t &cur, std::string &key)
	{
		size_t	idx = cur;

		if ((idx = content.find("=", cur)) == std::string::npos)
			return false;
		key = Utils::toLowercase(consmStr(content, cur, idx));
		if (key.empty() || Utils::canFind(' ', key) || Utils::canFind('\t', key) || Utils::canFind(';', key))
			return false;
		return true;
	}

	bool			consmVal(std::string const &content, size_t &cur, std::string &val)
	{
		size_t	idx = cur;

		if (idx >= content.size() || content[idx] != '"')
			return false;

		cur++;
		idx++;
		while (idx < content.size() && content[idx] != '"')
			idx++;
		if (idx == content.size())
			return false;

		val = consmStr(content, cur, idx);
		cur++;
		return true;
	}
}

bool	MultipartBody::fill_contDisp(std::string const &content, part &p)
{
	size_t	cur = 0;
	bool	has_name = 0, is_file = 0;

	consmOWS(content, cur);
	if (!accept("form-data", content, cur))
		return false;
	consmOWS(content, cur);

	while (cur < content.size())
	{
		if (!accept(";", content, cur))
			return false;

		kvpair	kv;
		consmOWS(content, cur);
		if (!consmKey(content, cur, kv.first) || !accept("=", content, cur))
			return false;
		if (!consmVal(content, cur, kv.second))
			return false;
		consmOWS(content, cur);

		if (kv.first == "name")
			has_name = true;
		if (kv.first == "filename")
			is_file = true;
		p.fdata.insert(kv);
	}
	if (!has_name)
		return false;
	p.is_file = is_file;
	return true;
}
