#pragma once

#include <stddef.h>
#include <stdint.h>

using namespace System;

namespace BSAWrapper {

	/*********************//**
	@name Return Codes
	@brief Error codes signify an issue that caused a function to exit prematurely. If a function exits prematurely, a reversal of any changes made during its execution is attempted before it exits.
	*************************/
	///@{

	const unsigned int LIBBSA_OK;  ///< The function completed successfully.
	const unsigned int LIBBSA_ERROR_INVALID_ARGS;  ///< Invalid arguments were given for the function.
	const unsigned int LIBBSA_ERROR_NO_MEM;  ///< The library was unable to allocate the required memory.
	const unsigned int LIBBSA_ERROR_FILESYSTEM_ERROR;  ///< There was an error encountered while performing a filesystem interaction (eg. reading, writing).
	const unsigned int LIBBSA_ERROR_BAD_STRING;  ///< A UTF-8 string contains characters that do not have Windows-1252 code points, or vice versa.
	const unsigned int LIBBSA_ERROR_ZLIB_ERROR;  ///< zlib reported an error during file compression or decompression.
	const unsigned int LIBBSA_ERROR_PARSE_FAIL;  ///< There was an error in parsing a BSA.

	/**
	@brief Matches the value of the highest-numbered return code.
	@details Provided in case clients wish to incorporate additional return codes in their implementation and desire some method of avoiding value conflicts.
	*/
	const unsigned int LIBBSA_RETURN_MAX;
	/* No doubt there will be more... */

	///@}
	/*********************//**
	@name BSA Version Flags
	@brief Used to specify the type of BSA to create. Only one should be specified at any one time.
	*************************/
	///@{

	const unsigned int LIBBSA_VERSION_TES3;  ///< Specifies the BSA structure supported by TES III: Morrowind.
	const unsigned int LIBBSA_VERSION_TES4;  ///< Specifies the BSA structure supported by TES IV: Oblivion.
	const unsigned int LIBBSA_VERSION_TES5;  ///< Specifies the BSA structure supported by TES V:Skyrim, Fallout 3, Fallout: New Vegas.

	///@}
	/*********************//**
	@name BSA Compression Flags
	@brief Used to specify the compression level to use for a BSA. Only one should be specified at any one time.
	*************************/
	///@{

	const unsigned int LIBBSA_COMPRESS_LEVEL_0;    ///< Use no compression.
	const unsigned int LIBBSA_COMPRESS_LEVEL_1;  ///< Use the lowest level of compression.
	const unsigned int LIBBSA_COMPRESS_LEVEL_2;
	const unsigned int LIBBSA_COMPRESS_LEVEL_3;
	const unsigned int LIBBSA_COMPRESS_LEVEL_4;
	const unsigned int LIBBSA_COMPRESS_LEVEL_5;
	const unsigned int LIBBSA_COMPRESS_LEVEL_6;
	const unsigned int LIBBSA_COMPRESS_LEVEL_7;
	const unsigned int LIBBSA_COMPRESS_LEVEL_8;
	const unsigned int LIBBSA_COMPRESS_LEVEL_9;  ///< Use the highest level of compression.
	const unsigned int LIBBSA_COMPRESS_LEVEL_NOCHANGE;  ///< Use the same level of compression as was used in the opened BSA.


	public ref class BSANET
	{
	public:

		/*------------------------------
		Types
		------------------------------*/

		/* Holds the source and destination paths for an asset to be added to a BSA.
		These paths must be valid until the BSA is saved, as they are not actually
		written until then. */
		/**
		@struct bsa_asset
		@brief A structure that holds data about a file in a BSA.
		@details Maps the external filesystem path of an asset to a path internal to the BSA. Used when adding a file to a BSA. The external path must be remain valid after adding the asset until bsa_save() is next called, after which it is no longer necessary.
		@var bsa_asset::sourcePath The external filesystem path to the asset.
		@var bsa_asset::destPath The path of the asset within the BSA.
		*/
		typedef ref struct bsa_asset {
			String^ sourcePath;  //The path of the asset in the external filesystem.
			String^ destPath;    //The path of the asset when it is in the BSA.
		};

		// handle for use by the wrapper
		_bsa_handle_int* bh;

		// constructor for the wrapper
		BSANET() {};

		///@}

		/*********************//**
		@name Version Functions
		*************************/
		///@{

		/**
		@brief Checks for library compatibility.
		@details Checks whether the loaded libbsa is compatible with the given version of libbsa, abstracting library stability policy away from clients. The version numbering used is major.minor.patch.
		@param versionMajor The major version number to check.
		@param versionMinor The minor version number to check.
		@param versionPatch The patch version number to check.
		@returns True if the library versions are compatible, false otherwise.
		*/
		bool bsa_is_compatible(const unsigned int versionMajor, const unsigned int versionMinor, const unsigned int versionPatch);
		/**
		@brief Gets the library version.
		@details Outputs the major, minor and patch version numbers for the loaded libbsa. The version numbering used is major.minor.patch.
		@param versionMajor A pointer to the major version number.
		@param versionMinor A pointer to the minor version number.
		@param versionPatch A pointer to the patch version number.
		*/
		void bsa_get_version(unsigned int ^ const versionMajor, unsigned int ^ const versionMinor, unsigned int ^ const versionPatch);


		///@}

		/*********************************//**
		@name Error Handling Functions
		*************************************/
		///@{

		/**
		@brief Returns the message for the last error or warning encountered.
		@details Outputs a string giving the a message containing the details of the last error or warning encountered by a function. Each time this function is called, the memory for the previous message is freed, so only one error message is available at any one time.
		@param details A pointer to the error details string outputted by the function.
		@returns A return code.
		*/
		unsigned int bsa_get_error_message(String^ details);

		/**
		@brief Frees the memory allocated to the last error details string.
		*/
		void bsa_cleanup();

		///@}


		/***************************************//**
		@name Lifecycle Management Functions
		*******************************************/
		///@{

		/**
		@brief Initialise a new BSA handle.
		@details Opens a BSA file, outputting a handle that holds an index of its contents. If the file doesn't exist then a handle for a new file will be created. You can create multiple handles.
		@param bh A pointer to the handle that is created by the function.
		@param path A string containing the relative or absolute path to the BSA file to be opened.
		@returns A return code.
		*/
		unsigned int bsa_open(String^ path);

		/**
		@brief Save a BSA at the given path. Not yet implemented.
		@details Opens a BSA file, outputting a handle that holds an index of its contents. If the file doesn't exist then a handle for a new file will be created. You can create multiple handles.
		@param bh The handle the function acts on.
		@param path A string containing the relative or absolute path to the BSA file to be saved to.
		@param flags A version flag and a compression flag combined using the bitwise OR operator.
		@returns A return code.
		*/
		unsigned int bsa_save(String^ path, const unsigned int flags);

		/**
		@brief Closes an existing handle.
		@details Closes an existing handle, freeing any memory allocated during its use.
		@param bh The handle to be destroyed.
		*/
		void bsa_close();

		///@}


		/***************************************//**
		@name Content Reading Functions
		*******************************************/
		///@{

		/**
		@brief Selectively outputs asset paths in a BSA.
		@details Gets all the assets indexed in a handle with internal paths that match the given regular expression.
		@param bh The handle the function acts on.
		@param contentPath The regular expression to match asset paths against.
		@param assetPaths The outputted array of asset paths. If no matching assets are found, this will be `NULL`.
		@param numAssets The size of the outputted array. If no matching assets are found, this will be `0`.
		@returns A return code.
		*/
		unsigned int bsa_get_assets(String^ contentPath, cli::array<String^>^ assetPaths);

		/**
		@brief Checks if a specific asset is in a BSA.
		@param bh The handle the function acts on.
		@param assetPath The internal asset path to look for.
		@param result The result of the check: `true` if the asset was found, `false` otherwise.
		@returns A return code.
		*/
		unsigned int bsa_contains_asset(String^ assetPath, bool^ result);

		///@}


		/***************************************//**
		@name Content Writing Functions
		@brief These functions are not yet implemented.
		*******************************************/
		///@{

		/**
		@brief Replaces all the assets in a BSA handle. Not yet implemented.
		@details Replaces the index in a BSA handle with a new index containing the given assets.
		@param bh The handle the function acts on.
		@param assets The inputted array of assets.
		@param numAssets The size of the inputted array.
		@returns A return code.
		*/
		unsigned int bsa_set_assets(cli::array<bsa_asset^>^ assets);

		/**
		@brief Adds an asset to a BSA handle. Not yet implemented.
		@details If an asset with the same path already exists in the BSA handle, this function will return with an error code.
		@param bh The handle the function acts on.
		@param asset The asset to be added.
		@returns A return code.
		*/
		unsigned int bsa_add_asset(bsa_asset^ asset);

		/**
		@brief Removes an asset from a BSA handle. Not yet implemented.
		@param bh The handle the function acts on.
		@param assetPath The asset to be removed.
		@returns A return code.
		*/
		unsigned int bsa_remove_asset(String^ assetPath);

		///@}


		/***************************************//**
		@name Content Extraction Functions
		*******************************************/
		///@{

		/**
		@brief Selectively extracts assets from a BSA.
		@details Extracts all the assets with internal paths that match the given regular expression to the given destination path, maintaining the directory structure they had inside the BSA.
		@param bh The handle the function acts on.
		@param contentPath The regular expression to match asset paths against.
		@param destPath The folder path to which assets should be extracted.
		@param assetPaths An array of the internal paths of the assets extracted. If no assets are extracted, this will be `NULL`.
		@param numAssets The size of the outputted array.
		@param overwrite If an asset is to be extracted to a path that already exists, this decides what will happen. If `true`, the existing file will be overwritten, otherwise the asset will not be extracted.
		@returns A return code.
		*/
		unsigned int bsa_extract_assets(String^ contentPath, String^ destPath, cli::array<String^>^ assetPaths, bool overwrite);

		/* Extracts a specific asset, found at assetPath, from a given BSA, to destPath. */

		/**
		@brief Extracts an asset from a BSA.
		@details Extracts the given asset to the given location. If a file already exists at the destination path, this function will return with an error code.
		@param bh The handle the function acts on.
		@param assetPath The path of the asset inside the BSA.
		@param destPath The file path to which the asset should be extracted.
		@param overwrite If the asset is to be extracted to a path that already exists, this decides what will happen. If `true`, the existing file will be overwritten, otherwise the asset will not be extracted.
		@returns A return code.
		*/
		unsigned int bsa_extract_asset(String^ assetPath, String^ destPath, bool overwrite);

		// TODO: 
		//unsigned int bsa_extract_asset_to_memory(bsa_handle bh, String^ assetPath, cli::array<System::Byte>^ _data);

		///@}


		/***************************************//**
		@name Misc. Functions
		*******************************************/
		///@{

		/**
		@brief Calculates the CRC32 of an asset.
		@details Calculates the 32-bit CRC of the given asset without extracting it. The CRC parameters are those used by the Boost.CRC library's `crc_32_type` type.
		@param bh The handle the function acts on.
		@param assetPath The internal asset path to calculate the CRC32 of.
		@param checksum The calculated checksum.
		@returns A return code.
		*/
		// TODO:
		//unsigned int bsa_calc_checksum(bsa_handle bh, String^ assetPath, System::Byte^ checksum);

		///@}
	};
}