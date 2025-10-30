/* NX_Filesystem.h -- API declaration for Nexium's filesystem module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_FILESYSTEM_H
#define NX_FILESYSTEM_H

#include "./NX_API.h"
#include <stdbool.h>
#include <stddef.h>

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Add directory or archive to search path
 * @param path Physical path to directory or archive
 * @param appendToEnd If true, add to end of search order; if false, add to beginning
 * @return True on success, false on failure
 */
NXAPI bool NX_AddSearchPath(const char* path, bool appendToEnd);

/**
 * @brief Remove directory or archive from search path
 * @param path Path that was previously added
 * @return True on success, false on failure
 */
NXAPI bool NX_RemoveSearchPath(const char* path);

/**
 * @brief Get list of all search paths in order
 * @return Null-terminated array of strings, must be freed with NX_FreeSearchPaths()
 */
NXAPI char** NX_GetSearchPaths(void);

/**
 * @brief Free search paths list returned by NX_GetSearchPaths()
 * @param paths Array to free
 */
NXAPI void NX_FreeSearchPaths(char** paths);

/**
 * @brief Mount archive to virtual file system
 * @param archivePath Physical path to archive (ZIP, 7Z, etc.)
 * @param mountPoint Virtual mount point (NULL for root)
 * @param appendToEnd If true, add to end of search order
 * @return True on success, false on failure
 */
NXAPI bool NX_MountArchive(const char* archivePath, const char* mountPoint, bool appendToEnd);

/**
 * @brief Unmount archive from virtual file system
 * @param archivePath Archive path that was previously mounted
 * @return True on success, false on failure
 */
NXAPI bool NX_UnmountArchive(const char* archivePath);

/**
 * @brief Get current write directory
 * @return Write directory path, or NULL if not set
 */
NXAPI const char* NX_GetWriteDir(void);

/**
 * @brief Set write directory for file operations
 * @param path Physical directory path for writing files
 * @return True on success, false on failure
 */
NXAPI bool NX_SetWriteDir(const char* path);

/**
 * @brief Get executable's base directory
 * @return Base directory path (read-only)
 */
NXAPI const char* NX_GetBaseDir(void);

/**
 * @brief Get user preferences directory
 * @param org Organization name
 * @param app Application name
 * @return Preferences directory path (platform-specific)
 */
NXAPI const char* NX_GetPrefDir(const char* org, const char* app);

/**
 * @brief Check if file or directory exists in virtual file system
 * @param filePath Virtual file path
 * @return True if exists, false otherwise
 */
NXAPI bool NX_FileExists(const char* filePath);

/**
 * @brief Check if path is a directory
 * @param path Virtual path to check
 * @return True if directory, false otherwise
 */
NXAPI bool NX_IsDirectory(const char* path);

/**
 * @brief Check if path is a regular file
 * @param path Virtual path to check
 * @return True if file, false otherwise
 */
NXAPI bool NX_IsFile(const char* path);

/**
 * @brief Get file size without opening
 * @param filePath Virtual file path
 * @return File size in bytes, or 0 on error
 */
NXAPI size_t NX_GetFileSize(const char* filePath);

/**
 * @brief Get real physical path of virtual file
 * @param filePath Virtual file path
 * @return Physical directory containing the file, or NULL if not found
 */
NXAPI const char* NX_GetRealPath(const char* filePath);

/**
 * @brief List directory contents
 * @param dirPath Virtual directory path
 * @return Null-terminated array of filenames, must be freed with NX_FreeDirectoryList()
 */
NXAPI char** NX_ListDirectory(const char* dirPath);

/**
 * @brief Free directory list returned by NX_ListDirectory()
 * @param list Array to free
 */
NXAPI void NX_FreeDirectoryList(char** list);

/**
 * @brief Create directory in write directory
 * @param dirPath Virtual directory path
 * @return True on success, false on failure
 */
NXAPI bool NX_CreateDirectory(const char* dirPath);

/**
 * @brief Delete file from write directory
 * @param filePath Virtual file path
 * @return True on success, false on failure
 */
NXAPI bool NX_DeleteFile(const char* filePath);

/**
 * @brief Load binary file into memory
 * @param filePath Virtual file path
 * @param size Pointer to store file size
 * @return File data buffer (must be freed), or NULL on failure
 */
NXAPI void* NX_LoadFile(const char* filePath, size_t* size);

/**
 * @brief Load text file into memory (null-terminated)
 * @param filePath Virtual file path
 * @return Text buffer (must be freed), or NULL on failure
 */
NXAPI char* NX_LoadFileText(const char* filePath);

/**
 * @brief Write binary data to file in write directory
 * @param filePath Virtual file path
 * @param data Data to write
 * @param size Data size in bytes
 * @return True on success, false on failure
 */
NXAPI bool NX_WriteFile(const char* filePath, const void* data, size_t size);

/**
 * @brief Write text data to file in write directory
 * @param filePath Virtual file path
 * @param data Text data to write
 * @param size Data size in bytes
 * @return True on success, false on failure
 */
NXAPI bool NX_WriteFileText(const char* filePath, const char* data, size_t size);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_FILESYSTEM_H
