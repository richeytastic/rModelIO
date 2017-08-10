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
#include <cstdio>   // popen
#include <boost/filesystem/operations.hpp>
using RModelIO::IDTFExporter;
using RModelIO::U3DExporter;
using RFeatures::ObjModel;


// public
U3DExporter::U3DExporter( const ObjModel::Ptr mod) : RModelIO::ObjModelExporter(mod)
{
#ifdef IDTF_CONVERTER
    addSupported( "u3d", "Universal 3D");
    std::cout << "IDTFConverter defined as " << IDTF_CONVERTER << std::endl;
#else
    std::cerr << "[ERROR] RModelIO::U3DExporter: U3D export unavailable; IDTF to U3D converter not defined!" << std::endl;
#endif
}   // end ctor


int convertIDTF2U3D( const std::string& idtffile, const std::string& u3dfile)
{
    std::string convexe;
#ifdef IDTF_CONVERTER
    convexe = IDTF_CONVERTER;
#endif
    std::ostringstream oss;
    oss << convexe << " -en 1 -input " << idtffile << " -output " << u3dfile;
    const std::string cmd = oss.str();

    FILE* pipe = popen( cmd.c_str(), "r");
    if ( !pipe)
        return -1;

    int retVal = 0;
    try
    {
        char buffer[128];
        std::string result = "";
        while ( !feof(pipe))
        {
            if (fgets(buffer, 128, pipe) != NULL)
                result += buffer;
        }   // end while
        std::cerr << result << std::endl;
    }   // end try
    catch ( const std::exception& e)
    {
        std::cerr << "Failed on read of byte from forked process " << cmd << std::endl;
        std::cerr << e.what() << std::endl;
        retVal = -2;
    }   // end catch

    pclose(pipe);
    return retVal;
}   // end convertIDTF2U3D


// protected
bool U3DExporter::doSave( const std::string& filename)
{
#ifndef IDTF_CONVERTER
    std::cerr << "[ERROR] RModelIO::U3DExporter::doSave: U3D export unavailable; IDTF to U3D converter not defined!" << std::endl;
    return false;
#endif

    bool savedOkay = true;

    // First save to intermediate IDTF format.
    bool delOnDestroy = true;
#ifndef NDEBUG
    delOnDestroy = false;
#endif

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
