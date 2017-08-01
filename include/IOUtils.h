#ifndef RMODELIO_IOUTILS_H
#define RMODELIO_IOUTILS_H

#include "rModelIO_Export.h"
#include <string>

namespace RModelIO
{

// Remove all occurances of "(*)" in given string.
rModelIO_EXPORT void removeParentheticalContent( std::string&);

// Returns the extension from filename without dot and in lower case.
// Returns an empty string if filename has no extension or has an empty extension (ends with a dot).
rModelIO_EXPORT std::string getExtension( const std::string& filename);
}   // end namespace

#endif
