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

#include <WRLExporter.h>
#include <ObjModel2VCG.h>
#include <FileIO.h> // rlib
#include <wrap/io_trimesh/export_vrml.h>
using RModelIO::WRLExporter;
using RFeatures::ObjModel;


// protected
bool WRLExporter::doSave( const ObjModel::Ptr model, const std::string& filename)
{
    if ( rlib::getExtension( filename) != "wrl")
    {
        std::cerr << "[ERROR] RModelIO::WRLExporter::doSave: " << filename << " does not end in '.wrl'" << std::endl;
        return false;
    }   // end if

    RModelIO::ObjModel2VCG obj2vcg;
    RModelIO::VCGObjModel::Ptr vobj = obj2vcg.create(model);
    int mask = vcg::tri::io::Mask::IOM_WEDGTEXCOORD;
    const int saveResult = vcg::tri::io::ExporterWRL<RModelIO::VCGObjModel>::Save( *vobj, filename.c_str(), mask, NULL);
    if ( saveResult != 0)
        std::cerr << "[ERROR] RModelIO::WRLExporter::doSave: Error saving model to '" << filename << "'" << std::endl;
    return saveResult == 0;
}   // end doSave
