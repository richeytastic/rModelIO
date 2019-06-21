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

#include <LaTeXU3DInserter.h>
#include <U3DExporter.h>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/process.hpp>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
using RModelIO::LaTeXU3DInserter;
using RModelIO::U3DExporter;
using RFeatures::ObjModel;
typedef RFeatures::CameraParams Cam;


// public
LaTeXU3DInserter::Ptr LaTeXU3DInserter::create( const ObjModel& model,
                                                const std::string& sdirectory,
                                                float fw, float fh,
                                                const Cam& cam,
                                                const std::string& figCap, const std::string& figLab,
                                                bool actOnOpen, bool remgen)
{
    Ptr minserter(  new LaTeXU3DInserter( fw, fh, cam, figCap, figLab, actOnOpen, remgen), [](LaTeXU3DInserter* d){ delete d;});
    if ( !minserter->setModel( model, sdirectory))
        minserter = nullptr;
    return minserter;
}   // end create


// private
LaTeXU3DInserter::LaTeXU3DInserter( float fw, float fh,
                                    const Cam& cam,
                                    const std::string& figCap, const std::string& figLab,
                                    bool actOnOpen, bool remgen)
    : _fw(fw), _fh(fh), _cam(cam), _figCap(figCap), _figLab(figLab), _actOnOpen(actOnOpen), _remgen(remgen)
{
}   // end ctor


// private
LaTeXU3DInserter::~LaTeXU3DInserter()
{
    if ( !_remgen)
        return;

    for ( const std::string& tmpfile : _delfiles)
    {
        boost::filesystem::remove( tmpfile);
        std::cerr << "Removed " << tmpfile << std::endl;
    }   // end foreach
    _delfiles.clear();
}   // end dtor


// public
bool LaTeXU3DInserter::setModel( const std::string& u3dfile)
{
    bool setokay = false;
    typedef boost::filesystem::path Path;
    if ( boost::algorithm::to_lower_copy( Path(u3dfile).extension().string()) != ".u3d")
        std::cerr << "[ERROR] RModelIO::LaTeXU3DInserter::setModel: " << u3dfile << " must have .u3d extension!" << std::endl;
    else
    {
        _u3dfile = u3dfile;
        setokay = true;
    }   // end else
    return setokay;
}   // end setModel


// private
bool LaTeXU3DInserter::setModel( const ObjModel& model, const std::string& sdir)
{
    const std::string u3dtmp = sdir + "/" + boost::filesystem::unique_path().replace_extension(".u3d").string();

#ifndef NDEBUG
    U3DExporter u3dxptr(false);
#else
    U3DExporter u3dxptr;
#endif
    if ( !u3dxptr.save( model, u3dtmp))
    {
        std::cerr << u3dxptr.err() << std::endl;
        return false;
    }   // end if

    _u3dfile = u3dtmp;
    _delfiles.push_back(u3dtmp);
    return true;
}   // end setModel


// public
std::ostream& RModelIO::operator<<( std::ostream& os, const LaTeXU3DInserter& params)
{
    const cv::Vec3f& coo = params._cam.focus;
    cv::Vec3f cpos = params._cam.pos;  // Will be changing cpos because of horrible media9/Adobe Reader camera orientation issue.
    const double roo = cv::norm(cpos - coo);
    const double fov = params._cam.fov;

    // Set cpos to be roo distance along +Z from coo
    cpos = coo + cv::Vec3f(0,0,(float)roo);

    cv::Vec3f c2c = cpos - coo;
    cv::normalize( c2c, c2c); // Doesn't need normalizing, but such things are habitual...

    os << "\\begin{figure}[!ht]" << std::endl;
    os << "\\centering" << std::endl;
    os << "\\includemedia[" << std::endl;
    os << "\twidth=" << params._fw << "mm," << std::endl;
    os << "\theight=" << params._fh << "mm," << std::endl;
    os << "\tkeepaspectratio," << std::endl;
    os << "\tactivate=" << (params._actOnOpen ? "pageopen" : "click") << "," << std::endl;
    os << "\tplaybutton=plain,    % plain | fancy (default) | none" << std::endl;
    os << "\t3Dlights=Hard," << std::endl;
    os << "\t3Dbg=1 1 1,          % background colour of scene (r g b) \\in [0,1] (can't set transparency if set)" << std::endl;
    os << "\t3Dcoo=" << coo[0] << " " << coo[1] << " " << coo[2] << ",         % centre of orbit of the camera (x y z)" << std::endl;
    os << "\t3Dc2c=" << c2c[0] << " " << c2c[1] << " " << c2c[2] << ",         % direction to camera from coo" << std::endl;
    os << "\t3Droll=0,           % clockwise roll in degrees around optical axis" << std::endl;
    os << "\t3Droo=" << std::left << std::fixed << std::setprecision(3) << roo << ",        % radius of obrbit" << std::endl;
    os << "\t3Daac=" << std::left << std::fixed << std::setprecision(3) << fov << "         % perspective fov in degrees" << std::endl;
    os << "\t]{}{" << params._u3dfile << "}" << std::endl;

    if ( !params._figCap.empty())
        os << "\\caption{" << params._figCap << "}" << std::endl;
    if ( !params._figLab.empty())
        os << "\\label{" << params._figLab << "}" << std::endl;

    os << "\\end{figure}" << std::endl;
    return os;
}   // end operator<<
