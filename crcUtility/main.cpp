#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "crc.h"
#include <iostream>
#include <sstream>

// Aiases
namespace fs = boost::filesystem;

// Prototypes
bool createCRCFile(fs::path file);
void log(const std::string &message);

int main( int argc, char *argv[] ) {
    if(argc < 2){
        log("Please include the path to the top level directory you would like crc files created for.");
        return 1;
    }

    std::string sPath = std::string(argv[1]);
    fs::path fsPath(sPath);
    // Make sure it's a directory
    try{
        if(!fs::is_directory(fsPath)){
            log(sPath + " does not appear to be a directory.");
            return 1;
        }
    }catch(fs::filesystem_error fse){
        log("Error:");
        log(std::string("\t") + fse.what());
        return 1;
    }

    try{
        for(fs::recursive_directory_iterator end, fileIt(fsPath); fileIt != end; ++fileIt){
            fs::path filePath = fileIt->path();
            if(fs::exists(filePath) && fs::is_regular_file(filePath) && fs::extension(filePath) != ".crc" && filePath.filename() != "." && filePath.filename() != ".."){
                bool success = createCRCFile(filePath);
                if(!success){
                    log("Could not create crc file for:\t" + filePath.string());
                }
            }
        }
    }catch(fs::filesystem_error fse){
        log("Error:");
        log(std::string("\t") + fse.what());
        return 1;
    }

    return 0;
}

// Functions
bool createCRCFile(fs::path file){
    fs::path crcFile(file.string() + ".crc");
    
    std::string contents = "";
    try{
        fs::ifstream ifsFile(file, std::ios::binary | std::ios::in);
        if(ifsFile.is_open()){
            contents = std::string(static_cast<std::stringstream const&>(std::stringstream() << ifsFile.rdbuf()).str());
        }
        ifsFile.close();
    }catch(fs::ofstream::failure e){
        log(e.what());
        return false;
    }

    boost::crc_32_type result;
    result.process_bytes(contents.data(), contents.length());

    // Overwrite if it exists, so trunc
    try{
        fs::ofstream ofsFile(crcFile, std::ios::out | std::ios::trunc);
        if(ofsFile.is_open()){
            ofsFile << std::hex << std::uppercase << result.checksum() << std::flush;
        }
        ofsFile.close();
    }catch(fs::ofstream::failure e){
        log(e.what());
        return false;
    }
    return true;
}

void log(const std::string &message){
    std::cout << message << std::endl;
}