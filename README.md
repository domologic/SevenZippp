# SevenZippp
Advanced linux fork of SevenZip++ from cmcnab ([Origin](https://bitbucket.org/cmcnab/sevenzip/ "origin")).

This is a C++ wrapper for accessing the 7-zip COM-like API in 7z.so. This code is heavily based off of the Client7z sample found in the LZMA SDK ([LZMA](http://www.7-zip.org/sdk.html "LZMA")).

The project itself is a static library and a small sample. To compile the library you will need the LZMA SDK for some headers.

## Our changes and additions to the library:

* Support for linux (we lost the support for windows on our way)
* Encryption
* the possibility to list the supported extensions
* compress and decompress from abstract streams and to abstract streams
* the possibility to extract single files from an archive
* a console log callback
* a progress callback

## How to use the library:

### In your code:

First of all, you need to initialize the library with your "7z.so" file:

    #include <7zpp.h>
    SevenZippp::SevenZipLibrary lib;
    lib.Load("<path/to>/7z.so");
    
To extract an archive, here's an example:
    try {
        SevenZippp::SevenZipExtractor extractor(lib, "<path/to>/archive.7z");
        extractor.ExtractFile("<foldername>", "secretpassword", ".*");
    catch ( SevenZippp::SevenZipException e) {
        std::cout << e.GetMessage() << std::endl;
    }

It's pretty similar to creating an archive:
    try {
        SevenZippp::SevenZipCompressor compressor(*m_plib, "<path/to>/archive.7z", password);
        compressor.CompressDirectory(path);
        // or if it is a single file to compress:
        compressor.CompressFile(path);
    } catch(SevenZippp::SevenZipException e) {
        std::cout << e.GetMessage() << std::endl;
    }
### Build Process:

We're using CMake to build the static library.

To compile, navigate to another folder, e.g. ../build/

#### Building for linux x86:

`cmake <path/to/sourcedirectory>; make`

#### Building for linux ARM:

`cmake -DCMAKE_TOOLCHAIN_FILE=<path/to/sourcedirectory>/cmake/LinuxArm.cmake <path/to/sourcedirectory>; make`
