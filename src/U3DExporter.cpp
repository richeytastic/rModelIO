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
#include <cstdlib>  // system (WIN32)
#include <cstdio>   // popen (UNIX)
#include <boost/filesystem/operations.hpp>
using RModelIO::IDTFExporter;
using RModelIO::U3DExporter;
using RFeatures::ObjModel;


std::string U3DExporter::IDTFConverter; // public static


// public
U3DExporter::U3DExporter( const ObjModel::Ptr mod)
    : RModelIO::ObjModelExporter(mod)
{
    if ( IDTFConverter.empty())
        std::cerr << "[WARNING] RModelIO::U3DExporter: U3D export disabled; IDTFConverter not set!" << std::endl;
    else
    {
        addSupported( "u3d", "Universal 3D");
        std::cout << "[STATUS] RModelIO::U3DExporter using IDTFConverter at " << IDTFConverter << std::endl;
    }   // end else
}   // end ctor


int convertIDTF2U3D( const std::string& idtffile, const std::string& u3dfile)
{
    // Enclose command in quotes due to possibility of spaces in Windows path
    const std::string cmd = "\"" + U3DExporter::IDTFConverter + "\" -en 1 -input " + idtffile + " -output " + u3dfile;
    int retVal = 0;

    try
    {
#ifdef _WIN32
        retVal = system(cmd.c_str());
        std::cerr << "Returned from system with value " << retVal << std::endl;
#else
        FILE* pipe = popen( cmd.c_str(), "r");
        if ( !pipe)
            return -1;

        char buffer[128];
        std::string result = "";
        while ( !feof(pipe))
        {
            if (fgets(buffer, 128, pipe) != NULL)
                result += buffer;
        }   // end while
        std::cerr << result << std::endl;
        pclose(pipe);
#endif
    }   // end try
    catch ( const std::exception& e)
    {
        std::cerr << "Failed on read of byte from forked process " << cmd << std::endl;
        std::cerr << e.what() << std::endl;
        retVal = -2;
    }   // end catch

    return retVal;
}   // end convertIDTF2U3D


// protected
bool U3DExporter::doSave( const std::string& filename)
{
    bool savedOkay = true;

    // First save to intermediate IDTF format.
    bool delOnDestroy = true;
    const ObjModel::Ptr model = _model;
    IDTFExporter idtfExporter( model, delOnDestroy);
    const std::string idtffile = boost::filesystem::path(filename).stem().string() + ".idtf";
    if ( !idtfExporter.save( idtffile))
    {   
        std::cerr << "[ERROR] RModelIO::U3DExporter::doSave: Export to intermediate IDTF format failed!" << std::endl;
        setErr( idtfExporter.err());
        savedOkay = false;
    }   // end if
    else
    {
        const int res = convertIDTF2U3D( idtffile, filename);
        const std::string ierr = "Unable to convert IDTF to U3D : ";
        std::string xerr;
        switch ( res)
        {
            case -1:
                xerr = "Could not fork conversion process!";
                break;
            case -2:
                xerr = "Failed to read byte data from forked process!";
                break;
        }   // end if
        if ( res < 0)
        {
            setErr( ierr + xerr);
            savedOkay = false;
        }   // end if
    }   // end else

    return savedOkay;
}   // end doSave
