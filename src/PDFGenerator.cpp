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
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/foreach.hpp>
#include <fstream>
#include <iomanip>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
using RModelIO::PDFGenerator;
using RModelIO::U3DExporter;
using RFeatures::ObjModel;
typedef RFeatures::CameraParams Cam;


std::string PDFGenerator::pdflatex;


// public
PDFGenerator::PDFGenerator( bool remGen) : _remGen(remGen)
{
    if ( pdflatex.empty())
        std::cerr << "[WARNING] RModelIO::PDFGenerator: PDF export disabled until pdflatex set" << std::endl;
    else
        std::cerr << "[STATUS] RModelIO::PDFGenerator using pdflatex at " << pdflatex << std::endl;
}   // end ctor


// public
PDFGenerator::~PDFGenerator()
{
    BOOST_FOREACH ( const LaTeXU3DInserter* inserter, _inserters)
        delete inserter;
}   // end dtor


// public
bool PDFGenerator::operator()( const std::string& texfile, bool remtexfile)
{
    if ( pdflatex.empty())
        return false;

    const std::string stem = boost::filesystem::path(texfile).stem().string();

    // Enclosing pdflatex command in quotes since Windows paths often have spaces
    bool success = false;
    const std::string cmd = "\"" + pdflatex + "\" -interaction batchmode \"" + texfile + "\"";
    int errsv = 0;
    std::string errMsg;
    std::cerr << "Generating PDF from " << texfile;

    try
    {
#ifdef _WIN32
        const int retVal = system( cmd.c_str());
        success = retVal == 0;
        if ( !success)
            errMsg = "[ERROR] RModelIO::PDFGenerator::operator():";
#else
        FILE* pipe = popen( cmd.c_str(), "r");
        if (!pipe)
            errMsg = "[ERROR] RModelIO::PDFGenerator::operator(): unable to open pipe;";
        else if ( pclose(pipe) < 0)
        {
            errsv = errno;
            errMsg = "[ERROR] RModelIO::PDFGenerator::operator(): unable to close pipe;";
        }   // end if
        else
        {
            std::cerr << " >> " << stem << ".pdf" << std::endl;
            success = true;
        }   // end else
#endif
    }   // end try
    catch ( const std::exception& e)
    {
        errMsg = "[EXCEPTION!] RModelIO::PDFGenerator::operator():" + std::string(e.what());
        success = false;
    }   // end ctch

    if ( !errMsg.empty())
    {
        std::cerr << " !FAILED!" << std::endl;
        std::cerr << errMsg << "; failed to execute " << cmd;
        if ( errsv > 0)
            std::cerr << " (errno: " << errsv;
        std::cerr << std::endl;
    }   // end if

    bool remGen = _remGen;
    // Don't remove pdflatex operator()d files on failure if this is a debug build
#ifndef NDEBUG
    remtexfile = false; // Ensure the .tex file is never removed in debug mode
    if ( !success && _remGen)
        remGen = false;
#endif

    if ( remGen)
    {
        boost::filesystem::remove( stem + ".aux");
        boost::filesystem::remove( stem + ".log");
        boost::filesystem::remove( stem + ".out");
    }   // end if

    // Remove the input .tex file?
    if ( success && remtexfile)
        boost::filesystem::remove(texfile);

    return success;
}   // end operator()


// public
PDFGenerator::LaTeXU3DInserter& PDFGenerator::getModelInserter( const ObjModel::Ptr model,
                                                                float fw, float fh,
                                                                const Cam& cam,
                                                                const std::string& figCap, const std::string& figLab,
                                                                bool actOnOpen)
{
    LaTeXU3DInserter* modelInserter = new LaTeXU3DInserter( fw, fh, cam, figCap, figLab, actOnOpen);
    modelInserter->setModel(model);
    _inserters.push_back(modelInserter);
    return *modelInserter;
}   // end getModelInserter


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
    BOOST_FOREACH ( const std::string& tmpfile, _delfiles)
        boost::filesystem::remove( tmpfile);
    _delfiles.clear();
}   // end dtor


// public
void PDFGenerator::LaTeXU3DInserter::setDimensions( float fw, float fh)
{
    _fw = fw;
    _fh = fh;
}   // end setDimensions

// public
void PDFGenerator::LaTeXU3DInserter::setCamera( const Cam& cam) { _cam = cam;}
void PDFGenerator::LaTeXU3DInserter::setCaption( const std::string& cap) { _figCap = cap;}
void PDFGenerator::LaTeXU3DInserter::setLabel( const std::string& lab) { _figLab = lab;}
void PDFGenerator::LaTeXU3DInserter::setActivateOnOpen( bool aoo) { _actOnOpen = aoo;}


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
bool PDFGenerator::LaTeXU3DInserter::setModel( const ObjModel::Ptr model)
{
    bool setokay = false;
    const std::string u3dtmp = boost::filesystem::unique_path().string() + ".u3d";
    U3DExporter u3dxptr(model);
    if ( !u3dxptr.save( u3dtmp))
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
