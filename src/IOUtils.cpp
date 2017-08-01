#include <IOUtils.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>


// public
void RModelIO::removeParentheticalContent( std::string& s)
{
    bool removed = true;
    while ( removed)
    {
        const std::string::size_type p0 = s.find_first_of('(');
        const std::string::size_type p1 = s.find_first_of(')');
        if ( p0 == std::string::npos || p1 == std::string::npos || p1 < p0)
            removed = false;
        else
            s.replace( p0, p1-p0+1, "");
    }   // end while
    boost::algorithm::trim(s);
}   // end removeParentheticalContent


// public
std::string RModelIO::getExtension( const std::string& fname)
{
    std::string fname2 = fname;
    boost::algorithm::trim(fname2);
    boost::filesystem::path p( fname2);
    if ( !p.has_extension())    // No extension
        return "";

    std::string ext = p.extension().string();
    assert( ext[0] == '.');
    if ( ext == ".")    // Empty extension (ends with dot)
        return "";

    ext = ext.substr(1);
    boost::algorithm::to_lower(ext);  // Don't want preceeding period & set to lower case
    return ext;
}   // end getExtension
