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

#include <PDFGenerator.h>
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
using RModelIO::PDFGenerator;
using RModelIO::U3DExporter;
using RFeatures::ObjModel;
typedef RFeatures::CameraParams Cam;

std::string PDFGenerator::pdflatex( "pdflatex"); // public static


// public
bool PDFGenerator::isAvailable()
{
    if ( boost::filesystem::exists( pdflatex))
        return true;
    const boost::filesystem::path genpath = boost::process::search_path(pdflatex);
    return !genpath.empty();
}   // end isAvailable


// public
PDFGenerator::PDFGenerator( bool remGen) : _remGen(remGen)
{
    if ( pdflatex.empty())
        pdflatex = "pdflatex";
}   // end ctor


// public
PDFGenerator::~PDFGenerator()
{
    for ( const LaTeXU3DInserter* inserter : _inserters)
        delete inserter;
}   // end dtor


// public
bool PDFGenerator::operator()( const std::string& texfile, bool remtexfile)
{
    namespace bp = boost::process;
    if ( !isAvailable())
    {
        std::cerr << "[WARNING] RModelIO::PDFGenerator: pdflatex not available! PDF generation disabled." << std::endl;
        return false;
    }   // end if

    boost::filesystem::path tpath = texfile;
    bool success = false;
    std::ostringstream errMsg;
    std::cerr << "Attempting to generate PDF from " << texfile << std::endl;

    try
    {
        // Annoyingly, Windows MiKTeX installs itself with .../MiKTeX 2.9/... in the filepath!!!
        // boost::process doesn't seem to handle this too well - it starts the process but then
        // pdflatex fails due to (apparently) parsing the program name as two separate tokens.
        // This doesn't appear to be an issue with pdflatex itself since the problem doesn't crop
        // up when running pdflatex on cmd line using its fully qualified path (with enclosing
        // quotes). Tried enclosing with escaped quotes in the pathname, but that didn't work
        // either. So doing it this way!
        //boost::filesystem::path genpath = boost::process::search_path(pdflatex);
        //if ( genpath.empty())
        //    genpath = boost::filesystem::path( pdflatex);
        //bp::child c(genpath, "-interaction", "batchmode", texfile, bp::std_out > stdout, bp::std_err > stderr);
        bp::child c( pdflatex + " -interaction batchmode -output-directory " + tpath.parent_path().string() + " " + texfile);
        c.wait();
        success = c.exit_code() == 0;
        if ( success)
            std::cerr << "Generated " << tpath.replace_extension("pdf").string() << std::endl;
        else
            errMsg << "[ERROR] RModelIO::PDFGenerator::operator(): Child process exited with " << c.exit_code();
    }   // end try
    catch ( const std::exception& e)
    {
        errMsg << "[EXCEPTION!] RModelIO::PDFGenerator::operator():" << e.what();
        success = false;
    }   // end ctch

    if ( !errMsg.str().empty())
        std::cerr << errMsg.str() << std::endl;

    bool remGen = _remGen;
    // Don't remove pdflatex generatedd files on failure if this is a debug build
#ifndef NDEBUG
    remtexfile = false; // Ensure the .tex file is never removed in debug mode
    if ( !success && _remGen)
        remGen = false;
#endif

    if ( remGen)
    {
        boost::filesystem::remove( tpath.replace_extension("aux"));
        boost::filesystem::remove( tpath.replace_extension("log"));
        boost::filesystem::remove( tpath.replace_extension("out"));
    }   // end if

    // Remove the input .tex file?
    if ( success && remtexfile)
    {
        boost::filesystem::remove(texfile);
        std::cerr << "Removed " << texfile << std::endl;
    }   // end if

    return success;
}   // end operator()


// public
PDFGenerator::LaTeXU3DInserter* PDFGenerator::getFigureInserter( const ObjModel* model,
                                                                 float fw, float fh,
                                                                 const Cam& cam,
                                                                 const std::string& figCap, const std::string& figLab,
                                                                 bool actOnOpen)
{
    LaTeXU3DInserter* modelInserter = new LaTeXU3DInserter( fw, fh, cam, figCap, figLab, actOnOpen);
    if ( !modelInserter->setModel(model))
    {
        delete modelInserter;
        return NULL;
    }   // end if
    _inserters.push_back(modelInserter);
    return modelInserter;
}   // end getFigureInserter


// private
PDFGenerator::LaTeXU3DInserter::LaTeXU3DInserter( float fw, float fh,
                                                  const Cam& cam,
                                                  const std::string& figCap, const std::string& figLab,
                                                  bool actOnOpen)
    : _fw(fw), _fh(fh), _cam(cam), _figCap(figCap), _figLab(figLab), _actOnOpen(actOnOpen)
{
}   // end ctor


// private
PDFGenerator::LaTeXU3DInserter::~LaTeXU3DInserter()
{
    for ( const std::string& tmpfile : _delfiles)
    {
        boost::filesystem::remove( tmpfile);
        std::cerr << "Removed " << tmpfile << std::endl;
    }   // end foreach
    _delfiles.clear();
}   // end dtor


// public
bool PDFGenerator::LaTeXU3DInserter::setModel( const std::string& u3dfile)
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


// public
bool PDFGenerator::LaTeXU3DInserter::setModel( const ObjModel* model)
{
    bool setokay = false;
    const std::string u3dtmp = boost::filesystem::unique_path().string() + ".u3d";
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
std::ostream& RModelIO::operator<<( std::ostream& os, const PDFGenerator::LaTeXU3DInserter& params)
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
