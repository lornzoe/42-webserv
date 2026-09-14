
#ifndef UPLOADSTOREDIRECTIVE_HPP
#define UPLOADSTOREDIRECTIVE_HPP

#include "SimpleDirective.hpp"

class UploadStoreDirective : public SimpleDirective
{
    private:
        std::string upload_location;
    
    public:
        UploadStoreDirective(TokenisedBlock::const_iterator& cit);
        void print(int depth) const;
        const std::string &getPath() const;
};

#endif