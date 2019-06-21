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

#include <ObjModelExporter.h>
using RModelIO::ObjModelExporter;
using RFeatures::ObjModel;


// public
ObjModelExporter::ObjModelExporter() : rlib::IOFormats()
{
}   // end ctor


// public
bool ObjModelExporter::save( const ObjModel& model, const std::string& fname)
{
    if ( fname.empty())
    {
        setErr( "Empty filename passed to RModelIO::ObjModelExporter::save!");
        return false;
    }   // end if

    setErr(""); // Clear error
    if ( !isSupported( fname))
    {
        setErr( fname + " has an unsupported file extension for exporting!");
        return false;
    }   // end if

    return doSave( model, fname);    // virtual
}   // end save
