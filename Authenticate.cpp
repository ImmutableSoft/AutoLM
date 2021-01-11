/***********************************************************************/
/*                                                                     */
/*   Module:  authenticate.cpp                                         */
/*   Version: 2020.0                                                   */
/*   Purpose: Command line local authenticate file with blockchain     */
/*                                                                     */
/*---------------------------------------------------------------------*/
/*                                                                     */
/*                 Copyright © 2020 ImmutableSoft Inc.                 */
/*                                                                     */
/* Permission is hereby granted, free of charge, to any person         */
/* obtaining a copy of this software and associated documentation      */
/* files (the “Software”), to deal in the Software without             */
/* restriction, including without limitation the rights to use, copy,  */
/* modify, merge, publish, distribute, sublicense, and/or sell copies  */
/* of the Software, and to permit persons to whom the Software is      */
/* furnished to do so, subject to the following conditions:            */
/*                                                                     */
/* The above copyright notice and this permission notice shall be      */
/* included in all copies or substantial portions of the Software.     */
/*                                                                     */
/* THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND,     */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF  */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND               */
/* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS */
/* BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN  */
/* ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN   */
/* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE    */
/* SOFTWARE.                                                           */
/*                                                                     */
/***********************************************************************/
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "autolm.h"
#include "base/sha256.h"

using std::string;

/***********************************************************************/
/*        main: Main application entry point                           */
/*                                                                     */
/*      Inputs: argc = the number of command line parameters (8 to 9)  */
/*              argv = array of individual command line parameters     */
/*                                                                     */
/*     Returns: Zero on successful license lookup, otherwise error     */
/*             license value output to stdio after querying blockchain */
/*             Output of zero is not valid, greater than zero is valid */
/*                                                                     */
/***********************************************************************/
int main(int argc, const char **argv)
{
  int res;
  FILE* pFILE;
  char buffer[1024], uri[512];
  size_t result;
  unsigned char digest[SHA256::DIGEST_SIZE];
  ui64 entityId, productId, releaseId, languages, version;
  SHA256 ctx = SHA256();

  // Executable name, Filename, Infura Product ID
  if ((argc < 3) || (argc > 3))
  {
    printf("Invalid number of arguments %d", argc);
    puts("");
    puts("authenticate <file name> <infura id>");
    puts("");
    puts("  Authenticate a digital file with creator release.");
    puts("    Returns version of release on success.");
    puts("");
    puts("  <file name> The path to a local file to verify on chain");
    puts("  <infura id> Your infura.io product id");

    return -1;
  }

  // Clear the digest before SHA256 computation
  memset(digest, 0, SHA256::DIGEST_SIZE);

  /*-------------------------------------------------------------------*/
  /* First, open the file in binary mode passed from command line .    */
  /*-------------------------------------------------------------------*/
  pFILE = fopen(argv[1], "rb");
  if (pFILE == NULL) { fputs("File error", stderr); exit(1); }

  // Initialize the computation of SHA256 checksum of file
  ctx.Sha256Init();

  // Read a portion of the file into the buffer and update SHA256
  while ((result = fread(buffer, 1, 1024, pFILE)) > 0)
    ctx.Sha256Update((unsigned char*)buffer, (unsigned int)result);

  // Finalize the SHA256 checksum
  ctx.Sha256Final(digest);

  // Close the file as it is no longer needed
  fclose(pFILE);

  // The SHA256 checksum of the whole file is now complete

  // Convert the checksum (digest) into a hex string
  char buf[2 * SHA256::DIGEST_SIZE + 1];
  buf[2 * SHA256::DIGEST_SIZE] = 0;
  for (ui32 i = 0; i < SHA256::DIGEST_SIZE; i++)
    sprintf(&buf[i * 2], "%02x", digest[i]);

  // If checksum complete (file exists, etc.), check blockchain
  printf("  File %s\n  SHA256 checksum: %s\n", argv[1], buf);

  // Lookup the file information from the SHA256 checksum
  res = EthereumAuthenticateFile((const char *)buf, argv[2], &entityId, &productId,
                                 &releaseId, &languages, &version, uri);

  // Output the resulting file information if success
  if (res == 0)
  {
    puts("  File authentication found on blockchain");
    printf("    Version %d.%d.%d.%d\n",
           (ui32)((version & 0xFFFF000000000000) >> 48),
           (ui32)((version & 0x0000FFFF00000000) >> 32),
           (ui32)((version & 0x00000000FFFF0000) >> 16),
           (ui32)((version & 0x000000000000FFFF)));
    printf("    Release #%llu for entity %llu and product %llu.\n",
           releaseId, entityId, productId);

    // Display if URI requested and result is not empty string
    if (uri && (uri[0] != '\0'))
      printf("    URI is %s\n", uri);
    else
      printf(" URI is empty\n");
  }

  // Otherwise an error occurred so inform the user
  else
  {
    if (res == curlPerformFailed)
      printf("ERROR - Curl HTTPS request failed.\n Is URL correct? Is OpenSSL used and Cacert.pem required/up to date?\n");
    else if (res == blockchainNotFound)
      printf("ERROR - Release not found for SHA256 checksum of this file");
    else if (res == blockchainAuthenticationFailed)
      printf("ERROR - Infura or HTTPS error. Is the Infura product id correct?");
    else
      printf(" ERROR - File unverified, error %d!\n", res);
  }

  // Return the result of the file authentication lookup
  return res;
}
