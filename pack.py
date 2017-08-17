#!python

import shutil
import os
from subprocess import call

version = "1.1.1"

sevenZCmd = r"C:\Program Files\7-Zip\7z.exe"
archs = ["Win32", "x64"]
srcDir = "SevenZip++"
dstDir = "Build"
includeDir = os.path.join("Include", "7zpp")
headers = [
	"7zpp.h",
	"CompressionFormat.h",
	"CompressionLevel.h",
	"Enum.h",
	"FileInfo.h",
	"SevenZipCompressor.h",
	"SevenZipException.h",
	"SevenZipExtractor.h",
	"SevenZipLibrary.h",
	"String.h"
]

def copyHeaders(arch):
	targetDir = os.path.join(dstDir, arch, includeDir)
	for h in headers:
		sourceFile = os.path.join(srcDir, h)
		#print "copy {0} to {1}".format(sourceFile, targetDir)
		shutil.copy(sourceFile, targetDir)

def getArchiveName(arch):
	if arch == "Win32":
		return "7zpp-{0}.7z".format(version)
	return "7zpp-{0}-{1}.7z".format(version, arch)

def package(arch):
	archiveName = os.path.abspath(os.path.join(dstDir, getArchiveName(arch)))
	targetDir = os.path.abspath(os.path.join(dstDir, arch))
	cmd = "\"{0}\" a \"{1}\" *".format(sevenZCmd, archiveName)
	#print targetDir
	#print cmd
	call(cmd, cwd=targetDir)

for a in archs:
	copyHeaders(a)
	# TODO: remove or exlude the PDB files?
	package(a)