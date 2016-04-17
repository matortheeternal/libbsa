/*  libbsa

A library for reading and writing BSA files.

Copyright (C) 2012-2013    WrinklyNinja

This file is part of libbsa.

libbsa is free software: you can redistribute
it and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation, either version 3 of
the License, or (at your option) any later version.

libbsa is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libbsa.  If not, see
<http://www.gnu.org/licenses/>.
*/

#include "libwrapper.h"
#include "..\..\src\helpers.h"
#include "..\..\src\genericbsa.h"
#include "..\..\src\tes3bsa.h"
#include "..\..\src\tes4bsa.h"
#include "..\..\src\error.h"
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>
#include <boost/filesystem.hpp>
#include <locale>
#include <boost/regex.hpp>
#include <boost/unordered_set.hpp>
#include <boost/crc.hpp>

using namespace std;
using namespace BSAWrapper;
using namespace System::Runtime::InteropServices;

/*------------------------------
Global variables
------------------------------*/

const unsigned int LIBBSA_VERSION_MAJOR = 2;
const unsigned int LIBBSA_VERSION_MINOR = 0;
const unsigned int LIBBSA_VERSION_PATCH = 0;

const char * extErrorString = nullptr;

/*------------------------------
Constants
------------------------------*/

// Return codes
const unsigned int LIBBSA_OK = 0;
const unsigned int LIBBSA_ERROR_INVALID_ARGS = 1;
const unsigned int LIBBSA_ERROR_NO_MEM = 2;
const unsigned int LIBBSA_ERROR_FILESYSTEM_ERROR = 3;
const unsigned int LIBBSA_ERROR_BAD_STRING = 4;
const unsigned int LIBBSA_ERROR_ZLIB_ERROR = 5;
const unsigned int LIBBSA_ERROR_PARSE_FAIL = 6;
const unsigned int LIBBSA_RETURN_MAX = BSAWrapper::LIBBSA_ERROR_PARSE_FAIL;

/* BSA save flags */
/* Use only one version flag. */
const unsigned int LIBBSA_VERSION_TES3 = 0x00000001;
const unsigned int LIBBSA_VERSION_TES4 = 0x00000002;
const unsigned int LIBBSA_VERSION_TES5 = 0x00000004;
/* Use only one compression flag. */
const unsigned int LIBBSA_COMPRESS_LEVEL_0 = 0x00000010;
const unsigned int LIBBSA_COMPRESS_LEVEL_1 = 0x00000020;
const unsigned int LIBBSA_COMPRESS_LEVEL_2 = 0x00000040;
const unsigned int LIBBSA_COMPRESS_LEVEL_3 = 0x00000080;
const unsigned int LIBBSA_COMPRESS_LEVEL_4 = 0x00000100;
const unsigned int LIBBSA_COMPRESS_LEVEL_5 = 0x00000200;
const unsigned int LIBBSA_COMPRESS_LEVEL_6 = 0x00000400;
const unsigned int LIBBSA_COMPRESS_LEVEL_7 = 0x00000800;
const unsigned int LIBBSA_COMPRESS_LEVEL_8 = 0x00001000;
const unsigned int LIBBSA_COMPRESS_LEVEL_9 = 0x00002000;
const unsigned int LIBBSA_COMPRESS_LEVEL_NOCHANGE = 0x00004000;

unsigned int c_error(const unsigned int code, const char * what) {
	extErrorString = what;
	return code;
}


/*------------------------------
Version Functions
------------------------------*/

/* Returns whether this version of libbsa is compatible with the given
version of libbsa. */
bool BSANET::bsa_is_compatible(const unsigned int versionMajor, const unsigned int versionMinor, const unsigned int versionPatch) {
	if (versionMajor == 2 && versionMinor == 0 && versionPatch == 0)
		return true;
	else
		return false;
}

void BSANET::bsa_get_version(unsigned int ^ versionMajor, unsigned int ^ versionMinor, unsigned int ^ versionPatch) {
	versionMajor = LIBBSA_VERSION_MAJOR;
	versionMinor = LIBBSA_VERSION_MINOR;
	versionPatch = LIBBSA_VERSION_PATCH;
}


/*------------------------------
Error Handling Functions
------------------------------*/

/* Outputs a string giving the a message containing the details of the
last error or warning encountered by a function called for the given
game handle. */
unsigned int BSANET::bsa_get_error_message(String^ details) {
	if (details == nullptr)
		return c_error(LIBBSA_ERROR_INVALID_ARGS, "Null pointer passed.");

	details = gcnew String(extErrorString);

	return LIBBSA_OK;
}

void BSANET::bsa_cleanup() {
	delete[] extErrorString;
}


/*----------------------------------
Lifecycle Management Functions
----------------------------------*/

/* Opens a BSA file at path, returning a handle.  */
unsigned int BSANET::bsa_open(String^ path) {
	if (path == nullptr)  //Check for valid args.
		return c_error(LIBBSA_ERROR_INVALID_ARGS, "Null pointer passed.");

	//Set the locale to get encoding conversions working correctly.
	setlocale(LC_CTYPE, "");
	locale global_loc = locale();
	locale loc(global_loc, new boost::filesystem::detail::utf8_codecvt_facet());
	boost::filesystem::path::imbue(loc);

	// convert path to char*
	char* pathc = (char*)(void*) Marshal::StringToHGlobalAnsi(path);

	//Create handle for the appropriate BSA type.
	try {
		if (libbsa::tes3::IsBSA(pathc))
			bh = new libbsa::tes3::BSA(pathc);
		else if (libbsa::tes4::IsBSA(pathc))
			bh = new libbsa::tes4::BSA(pathc);
		else
			bh = new libbsa::tes4::BSA(pathc);  //Arbitrary choice of BSA type.
	}
	catch (libbsa::error& e) {
		return c_error(e.code(), e.what());
	}
	catch (ios_base::failure& e) {
		return c_error(LIBBSA_ERROR_FILESYSTEM_ERROR, e.what());
	}

	return LIBBSA_OK;
}

/* Create a BSA at the specified path. The 'flags' argument consists of a set
of bitwise OR'd constants defining the version of the BSA and the
compression level used (and whether the compression is forced). */
unsigned int BSANET::bsa_save(String^ path, const unsigned int flags) {
	if (bh == nullptr || path == nullptr)  //Check for valid args.
		return c_error(LIBBSA_ERROR_INVALID_ARGS, "Null pointer passed.");

	//Check that flags are valid.
	unsigned int version = 0, compression = 0;

	//First we need to see what flags are set.
	if (flags & LIBBSA_VERSION_TES3 && !(flags & LIBBSA_COMPRESS_LEVEL_0))
		return c_error(LIBBSA_ERROR_INVALID_ARGS, "Morrowind BSAs cannot be compressed.");


	//Check for version flag duplication.
	if (flags & LIBBSA_VERSION_TES3)
		version = LIBBSA_VERSION_TES3;
	if (flags & LIBBSA_VERSION_TES4) {
		if (version > 0)
			return c_error(LIBBSA_ERROR_INVALID_ARGS, "Cannot specify more than one version.");
		version = LIBBSA_VERSION_TES4;
	}
	if (flags & LIBBSA_VERSION_TES5) {
		if (version > 0)
			return c_error(LIBBSA_ERROR_INVALID_ARGS, "Cannot specify more than one version.");
		version = LIBBSA_VERSION_TES5;
	}

	//Now remove version flag from flags and check for compression flag duplication.
	compression = flags ^ version;
	if (!(compression & (compression - 1)))
		return c_error(LIBBSA_ERROR_INVALID_ARGS, "Invalid compression level specified.");

	// convert path to char*
	char* pathc = (char*)(void*)Marshal::StringToHGlobalAnsi(path);

	try {
		bh->Save(pathc, version, compression);
	}
	catch (libbsa::error& e) {
		return c_error(e.code(), e.what());
	}
	catch (ios_base::failure& e) {
		return c_error(LIBBSA_ERROR_FILESYSTEM_ERROR, e.what());
	}

	return LIBBSA_OK;
}

/* Closes the BSA associated with the given handle, freeing any memory
allocated during its use. */
void BSANET::bsa_close() {
	delete bh;
}


/*------------------------------
Content Reading Functions
------------------------------*/

/* Gets an array of all the assets in the given BSA that match the contentPath
given. contentPath is a POSIX Extended regular expression that all asset
paths within the BSA will be compared to. */
unsigned int BSANET::bsa_get_assets(String^ contentPath, cli::array<String^>^ assetPaths) {
	if (bh == nullptr || contentPath == nullptr || assetPaths == nullptr) //Check for valid args.
		return c_error(LIBBSA_ERROR_INVALID_ARGS, "Null pointer passed.");

	//Free memory if in use.
	if (bh->extAssets != nullptr) {
		for (size_t i = 0; i < bh->extAssetsNum; i++)
			delete[] bh->extAssets[i];
		delete[] bh->extAssets;
		bh->extAssets = nullptr;
		bh->extAssetsNum = 0;
	}

	//Init values.
	assetPaths = nullptr;

	//Build regex expression. Also check that it is valid.
	boost::regex regex;
	char* ccontentPath = (char*)(void*)Marshal::StringToHGlobalAnsi(contentPath);

	try {
		regex = boost::regex(ccontentPath, boost::regex::extended | boost::regex::icase);
	}
	catch (boost::regex_error& e) {
		return c_error(LIBBSA_ERROR_INVALID_ARGS, e.what());
	}

	//We don't know how many matches there will be, so put all matches into a temporary buffer first.
	list<libbsa::BsaAsset> temp;
	bh->GetMatchingAssets(regex, temp);

	if (temp.empty())
		return LIBBSA_OK;

	//Fill external array.
	try {
		bh->extAssetsNum = temp.size();
		bh->extAssets = new char*[bh->extAssetsNum];
		size_t i = 0;
		for (list<libbsa::BsaAsset>::iterator it = temp.begin(), endIt = temp.end(); it != endIt; ++it) {
			bh->extAssets[i] = libbsa::ToNewCString(it->path);
			i++;
		}
	}
	catch (bad_alloc& e) {
		return c_error(LIBBSA_ERROR_NO_MEM, e.what());
	}
	catch (libbsa::error& e) {
		return c_error(e.code(), e.what());
	}

	// convert char** to array<String^>
	char** cassetPaths = bh->extAssets;
	std::vector<std::string> vAssets(cassetPaths, cassetPaths + bh->extAssetsNum);
	assetPaths = gcnew cli::array<String^>(vAssets.size());
	for (int i = 0; i < vAssets.size(); i++)
	{
		assetPaths[i] = gcnew String(vAssets[i].c_str());
	}

	return LIBBSA_OK;
}

/* Checks if a specific asset, found within the BSA at assetPath, is in the given BSA. */
unsigned int BSANET::bsa_contains_asset(String^ assetPath, bool^ result) {
	if (bh == nullptr || assetPath == nullptr || result == nullptr) //Check for valid args.
		return c_error(LIBBSA_ERROR_INVALID_ARGS, "Null pointer passed.");

	// convert assetPath to char*
	char* cassetPath = (char*)(void*)Marshal::StringToHGlobalAnsi(assetPath);

	string assetStr = libbsa::FixPath(cassetPath);

	result = bh->HasAsset(assetStr);

	return LIBBSA_OK;
}


/*------------------------------
Content Writing Functions
------------------------------*/

/* Replaces all the assets in the given BSA with the given assets. */
unsigned int BSANET::bsa_set_assets(cli::array<bsa_asset^>^ assets) {
	if (bh == nullptr || assets == nullptr) //Check for valid args.
		return c_error(LIBBSA_ERROR_INVALID_ARGS, "Null pointer passed.");

	return LIBBSA_OK;
}

/* Adds a specific asset to a BSA. */
unsigned int BSANET::bsa_add_asset(bsa_asset^ asset) {
	if (bh == nullptr) //Check for valid args.
		return c_error(LIBBSA_ERROR_INVALID_ARGS, "Null pointer passed.");

	return LIBBSA_OK;
}

/* Removes a specific asset, found at assetPath, from a BSA. */
unsigned int BSANET::bsa_remove_asset(String^ assetPath) {
	if (bh == nullptr || assetPath == nullptr) //Check for valid args.
		return c_error(LIBBSA_ERROR_INVALID_ARGS, "Null pointer passed.");

	return LIBBSA_OK;
}


/*--------------------------------
Content Extraction Functions
--------------------------------*/

/* Extracts all the files and folders that match the contentPath given to the
given destPath. contentPath is a path ending in a filename given as a POSIX
Extended regular expression that all asset paths within the BSA will be
compared to. Directory structure is preserved. */
unsigned int BSANET::bsa_extract_assets(String^ contentPath, String^ destPath, cli::array<String^>^ assetPaths, bool overwrite) {
	if (bh == nullptr || contentPath == nullptr || destPath == nullptr || assetPaths == nullptr) //Check for valid args.
		return c_error(LIBBSA_ERROR_INVALID_ARGS, "Null pointer passed.");

	//Free memory if in use.
	if (bh->extAssets != nullptr) {
		for (size_t i = 0; i < bh->extAssetsNum; i++)
			delete[] bh->extAssets[i];
		delete[] bh->extAssets;
		bh->extAssets = nullptr;
		bh->extAssetsNum = 0;
	}

	//Init values.
	assetPaths = nullptr;

	// convert contentPath to char*
	char* ccontentPath = (char*)(void*)Marshal::StringToHGlobalAnsi(contentPath);

	//Build regex expression. Also check that it is valid.
	boost::regex regex;
	try {
		regex = boost::regex(string(reinterpret_cast<const char*>(ccontentPath)), boost::regex::extended | boost::regex::icase);
	}
	catch (boost::regex_error& e) {
		return c_error(LIBBSA_ERROR_INVALID_ARGS, e.what());
	}

	//We don't know how many matches there will be, so put all matches into a temporary buffer first.
	list<libbsa::BsaAsset> temp;
	bh->GetMatchingAssets(regex, temp);

	if (temp.empty())
		return LIBBSA_OK;

	// convert destPath to char*
	char* cdestPath = (char*)(void*)Marshal::StringToHGlobalAnsi(destPath);

	//Extract files.
	try {
		bh->Extract(temp, string(reinterpret_cast<const char*>(cdestPath)), overwrite);
	}
	catch (libbsa::error& e) {
		return c_error(e.code(), e.what());
	}

	//Now iterate through temp hashmap, outputting filenames.
	try {
		bh->extAssetsNum = temp.size();
		bh->extAssets = new char*[bh->extAssetsNum];
		size_t i = 0;
		for (list<libbsa::BsaAsset>::iterator it = temp.begin(), endIt = temp.end(); it != endIt; ++it) {
			bh->extAssets[i] = libbsa::ToNewCString(it->path);
			i++;
		}
	}
	catch (bad_alloc& e) {
		return c_error(LIBBSA_ERROR_NO_MEM, e.what());
	}
	catch (libbsa::error& e) {
		return c_error(e.code(), e.what());
	}

	// convert char** to array<String^>
	char** cassetPaths = bh->extAssets;
	std::vector<std::string> vAssets(cassetPaths, cassetPaths + bh->extAssetsNum);
	assetPaths = gcnew cli::array<String^>(vAssets.size());
	for (int i = 0; i < vAssets.size(); i++)
	{
		assetPaths[i] = gcnew String(vAssets[i].c_str());
	}

	return LIBBSA_OK;
}

/* Extracts a specific asset, found at assetPath, from a given BSA, to destPath. */
unsigned int BSANET::bsa_extract_asset(String^ assetPath, String^ destPath, bool overwrite) {
	if (bh == nullptr || assetPath == nullptr || destPath == nullptr) //Check for valid args.
		return c_error(LIBBSA_ERROR_INVALID_ARGS, "Null pointer passed.");

	// convert assetPath to char*
	char* cassetPath = (char*)(void*)Marshal::StringToHGlobalAnsi(assetPath);
	// convert destPath to char*
	char* cdestPath = (char*)(void*)Marshal::StringToHGlobalAnsi(destPath);

	string assetStr = libbsa::FixPath(cassetPath);

	try {
		bh->Extract(assetStr, string(reinterpret_cast<const char*>(cdestPath)), overwrite);
	}
	catch (libbsa::error& e) {
		return c_error(e.code(), e.what());
	}

	return LIBBSA_OK;
}
