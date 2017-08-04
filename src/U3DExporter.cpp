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
#include <ObjModel2VCG.h>
using RModelIO::VCGObjModel;
using RModelIO::U3DExporter;
using RFeatures::ObjModel;
#include <wrap/io_trimesh/export_u3d.h> // VCG
#include <cassert>
#include <iostream>
#include <sstream>
#include <boost/filesystem/operations.hpp>


// public
U3DExporter::U3DExporter( const ObjModel::Ptr mod) : RModelIO::ObjModelExporter(mod)
{
    addSupported( "u3d", "Universal 3D");
}   // end ctor


// protected
bool U3DExporter::doSave( const ObjModel::Ptr model, const std::string& filename)
{
    RModelIO::ObjModel2VCG om2vcg;
    VCGObjModel::Ptr vmodel = om2vcg.create( model);
    if ( vmodel == NULL)
    {
        setErr( "[ERROR] RModelIO::U3DExporter::save: NULL VCGObjModel!");
        return false;
    }   // end if

    // Need to set all the texture map filenames (if present) and save out the textures.
    // Image files are saved in the same location alongside the model.
    const std::vector<cv::Mat>& tmaps = om2vcg.getTextures();
    typedef boost::filesystem::path Path;
    const Path mpath( filename);
    const Path dir = mpath.parent_path();  // Directory model is being saved in
    const Path stem = mpath.stem();

    const int nt = tmaps.size();
    for ( int i = 0; i < nt; ++i)
    {
        std::ostringstream oss;
        oss << "_M" << i << ".bmp";
        Path mfname = dir / stem / oss.str();
        if ( !cv::imwrite( mfname.string(), tmaps[i]))
            std::cerr << "[WARNING] RModelIO::U3DExporter::save: Unable to save texture map " << mfname << std::endl;
        vmodel->textures[i] = mfname.filename().string();
    }   // end for

    const char* convLoc = dir.string().c_str();  // Do conversion in same directory as where model being saved
    vcg::tri::io::u3dparametersclasses::Movie15Parameters<VCGObjModel> movParams;
    const int mask = vcg::tri::io::ExporterU3D<VCGObjModel>::GetExportMaskCapability();

    const int errCode = vcg::tri::io::ExporterU3D<VCGObjModel>::Save( *vmodel, filename.c_str(), convLoc, movParams, mask);
    if ( errCode > 0)
        setErr( vcg::tri::io::ExporterU3D<VCGObjModel>::ErrorMsg( errCode));
    return errCode == 0;
}   // end doSave


