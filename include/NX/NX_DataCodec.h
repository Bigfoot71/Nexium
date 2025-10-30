/* NX_DataCodec.h -- API declaration for Nexium's data codec module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_DATA_CODEC_H
#define NX_DATA_CODEC_H

#include "./NX_API.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

/**
 * @brief Compresses binary data using zlib DEFLATE algorithm
 *
 * Creates a compressed buffer with an 8-byte header containing the uncompressed size
 * followed by zlib-compressed data. The caller must free the returned buffer using NX_Free().
 *
 * Format: [8 bytes: uncompressed size][compressed data with zlib header and checksum]
 *
 * @param data Pointer to the data to compress
 * @param dataSize Size of the input data in bytes
 * @param outputSize Pointer to receive the size of the compressed output (header + compressed data)
 * @return Pointer to the allocated compressed buffer, or NULL on failure
 */
NXAPI void* NX_CompressData(const void* data, size_t dataSize, size_t* outputSize);

/**
 * @brief Decompresses binary data compressed by NX_CompressData()
 * 
 * Reads the 8-byte uncompressed size header, allocates the appropriate buffer, and decompresses
 * the zlib data. The caller must free the returned buffer using NX_Free().
 * 
 * @param data Pointer to the compressed data (with 8-byte header)
 * @param dataSize Size of the compressed data including the header
 * @param outputSize Pointer to receive the size of the decompressed output
 * @return Pointer to the allocated decompressed buffer, or NULL on failure
 */
NXAPI void* NX_DecompressData(const void* data, size_t dataSize, size_t* outputSize);

/**
 * @brief Compresses a null-terminated text string using zlib DEFLATE algorithm
 * 
 * Convenience wrapper around NX_CompressData() for text strings. Uses strlen() to determine
 * the text length and compresses without including the null terminator.
 * The caller must free the returned buffer using NX_Free().
 * 
 * @param text Pointer to the null-terminated text string to compress
 * @param outputSize Pointer to receive the size of the compressed output
 * @return Pointer to the allocated compressed buffer, or NULL on failure
 */
NXAPI void* NX_CompressText(const char* text, size_t* outputSize);

/**
 * @brief Decompresses text data and adds a null terminator
 * 
 * Decompresses data compressed by NX_CompressText() and automatically appends a null terminator,
 * making the result safe to use as a C string. The caller must free the returned buffer using NX_Free().
 * 
 * @param data Pointer to the compressed data (with 8-byte header)
 * @param dataSize Size of the compressed data including the header
 * @return Pointer to the allocated null-terminated string, or NULL on failure
 */
NXAPI char* NX_DecompressText(const void* data, size_t dataSize);

/**
 * @brief Encodes binary data to Base64 ASCII string
 * 
 * Converts binary data to Base64 encoding (RFC 4648). The output is a null-terminated ASCII string.
 * The caller must free the returned buffer using NX_Free().
 * 
 * @param data Pointer to the binary data to encode
 * @param dataSize Size of the input data in bytes
 * @param outputSize Pointer to receive the size of the encoded string (excluding null terminator)
 * @return Pointer to the allocated null-terminated Base64 string, or NULL on failure
 */
NXAPI char* NX_EncodeBase64(const void* data, size_t dataSize, size_t* outputSize);

/**
 * @brief Decodes a Base64 ASCII string to binary data
 * 
 * Converts a Base64-encoded string (RFC 4648) back to binary data. The input string length
 * must be a multiple of 4. The caller must free the returned buffer using NX_Free().
 * 
 * @param text Pointer to the null-terminated Base64 string
 * @param outputSize Pointer to receive the size of the decoded binary data
 * @return Pointer to the allocated binary data buffer, or NULL on failure
 */
NXAPI void* NX_DecodeBase64(const char* text, size_t* outputSize);

/**
 * @brief Computes the CRC32 checksum of data
 * 
 * Calculates a 32-bit cyclic redundancy check (CRC32) value for error detection.
 * 
 * @param data Pointer to the data to checksum
 * @param dataSize Size of the data in bytes
 * @return The CRC32 checksum value
 */
NXAPI uint32_t NX_ComputeCRC32(void* data, size_t dataSize);

/**
 * @brief Computes the MD5 hash of data
 * 
 * Calculates a 128-bit MD5 hash. Returns a pointer to a static array of 4 uint32_t values (16 bytes total).
 * The returned pointer is valid until the next call to this function.
 * 
 * @param data Pointer to the data to hash
 * @param dataSize Size of the data in bytes
 * @return Pointer to a static uint32_t[4] array containing the MD5 hash
 */
NXAPI const uint32_t* NX_ComputeMD5(void* data, size_t dataSize);

/**
 * @brief Computes the SHA-1 hash of data
 * 
 * Calculates a 160-bit SHA-1 hash. Returns a pointer to a static array of 5 uint32_t values (20 bytes total).
 * The returned pointer is valid until the next call to this function.
 * 
 * @param data Pointer to the data to hash
 * @param dataSize Size of the data in bytes
 * @return Pointer to a static uint32_t[5] array containing the SHA-1 hash
 */
NXAPI const uint32_t* NX_ComputeSHA1(void* data, size_t dataSize);

/**
 * @brief Computes the SHA-256 hash of data
 * 
 * Calculates a 256-bit SHA-256 hash. Returns a pointer to a static array of 8 uint32_t values (32 bytes total).
 * The returned pointer is valid until the next call to this function.
 * 
 * @param data Pointer to the data to hash
 * @param dataSize Size of the data in bytes
 * @return Pointer to a static uint32_t[8] array containing the SHA-256 hash
 */
NXAPI const uint32_t* NX_ComputeSHA256(void* data, size_t dataSize);

#endif // NX_DATA_CODEC_H
