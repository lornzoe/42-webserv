#include "BlockDirective.hpp"

BlockDirective::BlockDirective(TokenisedBlock::const_iterator &cit) : Directive(*cit)
{
	std::size_t braces = 1;
	cit++;

	// start cit from the first line in the contents
	while (braces != 0)
	{
		if (cit->front() == "}")
		{
			braces--;
			continue;
		}
		if (AllowedDirectives::isSimpleType(cit->front()))
		{
			// handle simple type
		}
		else if (AllowedDirectives::isBlockType(cit->front()))
		{
			// handle block type
		}
		else
			throw std::exception(); // not a valid directive
		cit++;
	}
}

BlockDirective::~BlockDirective()
{
}