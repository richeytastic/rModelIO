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
#include <boost/process/start_dir.hpp>
#ifdef _WIN32
#include <boost/process/windows.hpp>
#endif
#include <cassert>
#include <fstream>
#include <iomanip>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
using RModelIO::PDFGenerator;
using RModelIO::U3DExporter;
using RFeatures::ObjModel;

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
    std::cerr << "[INFO] RModelIO::PDFGenerator: Attempting to generate PDF from " << texfile << std::endl;

    // Get the parent path of the texfile to run pdflatex in.
    const std::string ppath = tpath.parent_path().string();

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
        //std::string cmd = pdflatex + " -interaction batchmode -quiet -output-directory " + tpath.parent_path().string() + " " + texfile;
        std::string cmd = pdflatex + " --shell-escape -interaction batchmode -output-directory " + ppath + " " + texfile;
        std::cerr << cmd << std::endl;
#ifdef _WIN32
        bp::child c( cmd, bp::windows::hide, bp::start_dir=ppath);
#else
        bp::child c( cmd, bp::start_dir=ppath);
#endif
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

