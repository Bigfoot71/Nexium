/* NX_Filesystem.cpp -- API definition for Nexium's filesystem module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/NX_Filesystem.h>
#include <NX/NX_Memory.h>

#include <SDL3/SDL_stdinc.h>
#include <physfs.h>

// ============================================================================
// PUBLIC API
// ============================================================================

bool NX_AddSearchPath(const char* path, bool appendToEnd)
{
    return PHYSFS_mount(path, NULL, appendToEnd ? 1 : 0) != 0;
}

bool NX_RemoveSearchPath(const char* path)
{
    return PHYSFS_unmount(path) != 0;
}

char** NX_GetSearchPaths()
{
    return PHYSFS_getSearchPath();
}

void NX_FreeSearchPaths(char** paths)
{
    PHYSFS_freeList(paths);
}

bool NX_MountArchive(const char* archivePath, const char* mountPoint, bool appendToEnd)
{
    return PHYSFS_mount(archivePath, mountPoint, appendToEnd ? 1 : 0) != 0;
}

bool NX_UnmountArchive(const char* archivePath)
{
    return PHYSFS_unmount(archivePath) != 0;
}

const char* NX_GetWriteDir()
{
    return PHYSFS_getWriteDir();
}

bool NX_SetWriteDir(const char* path)
{
    return PHYSFS_setWriteDir(path) != 0;
}

const char* NX_GetBaseDir()
{
    return PHYSFS_getBaseDir();
}

const char* NX_GetPrefDir(const char* org, const char* app)
{
    return PHYSFS_getPrefDir(org, app);
}

bool NX_FileExists(const char* filePath)
{
    return PHYSFS_exists(filePath) != 0;
}

bool NX_IsDirectory(const char* path)
{
    PHYSFS_Stat stat;
    if (PHYSFS_stat(path, &stat) == 0) {
        return false;
    }
    return stat.filetype == PHYSFS_FILETYPE_DIRECTORY;
}

bool NX_IsFile(const char* path)
{
    PHYSFS_Stat stat;
    if (PHYSFS_stat(path, &stat) == 0) {
        return false;
    }
    return stat.filetype == PHYSFS_FILETYPE_REGULAR;
}

size_t NX_GetFileSize(const char* filePath)
{
    PHYSFS_Stat stat;
    if (PHYSFS_stat(filePath, &stat) == 0) {
        return -1;
    }
    return stat.filesize;
}

const char* NX_GetRealPath(const char* filePath)
{
    return PHYSFS_getRealDir(filePath);
}

char** NX_ListDirectory(const char* dirPath)
{
    return PHYSFS_enumerateFiles(dirPath);
}

void NX_FreeDirectoryList(char** list)
{
    PHYSFS_freeList(list);
}

bool NX_CreateDirectory(const char* dirPath)
{
    return PHYSFS_mkdir(dirPath) != 0;
}

bool NX_DeleteFile(const char* filePath)
{
    return PHYSFS_delete(filePath) != 0;
}

void* NX_LoadFile(const char* filePath, size_t* size)
{
    if (!filePath || !size) {
        return NULL;
    }

    PHYSFS_File* file = PHYSFS_openRead(filePath);
    if (!file) {
        *size = 0;
        return NULL;
    }

    PHYSFS_sint64 fileSize = PHYSFS_fileLength(file);
    if (fileSize < 0) {
        PHYSFS_close(file);
        *size = 0;
        return NULL;
    }

    void* buffer = NX_Malloc(fileSize);
    if (!buffer) {
        PHYSFS_close(file);
        *size = 0;
        return NULL;
    }

    PHYSFS_sint64 bytesRead = PHYSFS_readBytes(file, buffer, (PHYSFS_uint64)fileSize);
    PHYSFS_close(file);

    if (bytesRead != fileSize) {
        NX_Free(buffer);
        *size = 0;
        return NULL;
    }

    *size = (size_t)fileSize;
    return buffer;
}

char* NX_LoadFileText(const char* filePath)
{
    if (!filePath) {
        return NULL;
    }

    PHYSFS_File* file = PHYSFS_openRead(filePath);
    if (!file) {
        return NULL;
    }

    PHYSFS_sint64 fileSize = PHYSFS_fileLength(file);
    if (fileSize < 0) {
        PHYSFS_close(file);
        return NULL;
    }

    char* buffer = NX_Malloc<char>(fileSize + 1);
    if (!buffer) {
        PHYSFS_close(file);
        return NULL;
    }

    PHYSFS_sint64 bytesRead = PHYSFS_readBytes(file, buffer, (PHYSFS_uint64)fileSize);
    PHYSFS_close(file);

    if (bytesRead != fileSize) {
        NX_Free(buffer);
        return NULL;
    }

    buffer[fileSize] = '\0';

    return buffer;
}

bool NX_WriteFile(const char* filePath, const void* data, size_t size)
{
    if (!filePath || !data || size == 0) {
        return false;
    }

    PHYSFS_File* file = PHYSFS_openWrite(filePath);
    if (!file) {
        return false;
    }

    PHYSFS_sint64 bytesWritten = PHYSFS_writeBytes(file, data, (PHYSFS_uint64)size);
    PHYSFS_close(file);

    return bytesWritten == (PHYSFS_sint64)size;
}

bool NX_WriteFileText(const char* filePath, const char* data, size_t size)
{
    if (!filePath || !data || size == 0) {
        return false;
    }

    PHYSFS_File* file = PHYSFS_openWrite(filePath);
    if (!file) {
        return false;
    }

    PHYSFS_sint64 bytesWritten = PHYSFS_writeBytes(file, data, (PHYSFS_uint64)size);
    PHYSFS_close(file);

    return bytesWritten == (PHYSFS_sint64)size;
}
