/************************************************************************
 * Copyright (C) 2017 Richard Palmer
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ************************************************************************/

#include <U3DExporter.h>
#include <IDTFExporter.h>
#include <cassert>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <boost/filesystem/operations.hpp>
#include <boost/process.hpp>    // Requires at least boost 1.64+
using RModelIO::IDTFExporter;
using RModelIO::U3DExporter;
using RFeatures::ObjModel;


std::string U3DExporter::IDTFConverter( "IDTFConverter"); // public static


// public static
bool U3DExporter::isAvailable()
{
    if ( boost::filesystem::exists( IDTFConverter))
        return true;
    const boost::filesystem::path exepath = boost::process::search_path( IDTFConverter);
    return !exepath.empty();
}   // end isAvailable


// public
U3DExporter::U3DExporter( bool delOnDestroy)
    : RModelIO::ObjModelExporter(), _delOnDestroy(delOnDestroy)
{
    if ( IDTFConverter.empty())
        IDTFConverter = "IDTFConverter";

    if ( isAvailable())
        addSupported( "u3d", "Universal 3D");
    else
        std::cerr << "[WARNING] RModelIO::U3DExporter: U3D export disabled; IDTFConverter not found on PATH!" << std::endl;
}   // end ctor


bool convertIDTF2U3D( const std::string& idtffile, const std::string& u3dfile)
{
    bool success = false;
    try
    {
        std::ostringstream cmd;
        cmd << "\"" << U3DExporter::IDTFConverter << "\""
            << " -pq 1000"  // Position quality MAX
            << " -tcq 1000" // Texture coordinates quality MAX
            << " -gq 1000"  // Geometry quality MAX
            << " -tq 100"   // Texture quality MAX
            << " -en 1"     // Enable normals exclusion 
            << " -eo 65535" // Export everything
            << " -input " << idtffile
            << " -output " << u3dfile;
        boost::process::child c( cmd.str());
        c.wait();
        success = c.exit_code() == 0;
    }   // end try
    catch ( const std::exception& e)
    {
        std::cerr << "Failed on read of byte from forked process " << std::endl;
        std::cerr << e.what() << std::endl;
        success = false;
    }   // end catch

    return success;
}   // end convertIDTF2U3D


// protected
bool U3DExporter::doSave( const ObjModel* model, const std::string& filename)
{
    bool savedOkay = true;

    // First save to intermediate IDTF format.
    IDTFExporter idtfExporter( _delOnDestroy);
    const std::string idtffile = boost::filesystem::path(filename).replace_extension("idtf").string();
    if ( !idtfExporter.save( model, idtffile))
    {   
        setErr( idtfExporter.err());
        savedOkay = false;
    }   // end if
    else if ( !convertIDTF2U3D( idtffile, filename))
    {
        setErr("Unable to convert IDTF to U3D!");
        savedOkay = false;
    }   // end if

    return savedOkay;
}   // end doSave
