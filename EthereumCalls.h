/***********************************************************************/
/*                                                                     */
/*   Module:  EthereumCalls.h                                          */
/*   Version: 2020.0                                                   */
/*   Purpose: Header file for the Ethereum calls into the Immutable    */
/*            Ecosystem Smart Contracts                                */
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
#ifndef _ETHEREUMCALLS_H
#define _ETHEREUMCALLS_H
#ifdef _MIBSIM
#include "common.h"
#include "sha1.h"
#include "md5.h"
#else
#include "base/common.h"
#include "base/sha1.h"
#include "base/md5.h"
#endif

/***********************************************************************/
/* Configuration                                                       */
/***********************************************************************/

/*
 * Build options
 */

// Application Ethereum Blockchain Connectivity Options
//
// Be sure to use the same network for both URL and contract address
//   Both should be for Ropsten or Main Network
#define CURL_HOST_URL              /*LOCAL_GANACHE_URL*/ROPSTEN_INFURA_URL
#define IMMUTABLE_LICENSE_CONTRACT /*GANACHE_ACTIVATE_CONTRACT*/ROPSTEN_ACTIVATE_CONTRACT
#define IMMUTABLE_PRODUCT_CONTRACT /*GANACHE_AUTHENTICATE_CONTRACT*/ROPSTEN_AUTHENTICATE_CONTRACT

// Options are Ropsten or Local Ganache. Main Network coming soon!
#define ROPSTEN_INFURA_URL         "https://ropsten.infura.io/v3/"
#define LOCAL_GANACHE_URL          "http://localhost:8545/"

// Debugging options
#define AUTOLM_DEBUG               0 // 1 to Enable debug output
#if AUTOLM_DEBUG
#define   PRINTF                   printf
#else
#define   PRINTF(...)          
#endif

// Immutable Ecosystem
//   DO NOT EDIT BELOW

// Deprecated 1.0
// Keccak256 ("licenseStatus(uint256, uint256, uint256)") =
// 0x9277d3d6b97556c788e9717ce4902c3a0c92314558dc2f0dad1e0d0727f04629
//#define LICENSE_STATUS_ID          "0x9277d3d6"

// Current 2.0
// Keccak256 ("activateStatus(uint256,uint256,uint256)") =
// 0x1da7d8648240b9b4db8c4f11fcd46bf2ccd74be6fdf20e075e3cd1541a3ecae1
#define LICENSE_STATUS_ID             "0x1da7d864"
// Current 2.0
// Keccak256 ("productReleaseHashDetails(uint256)") =
// 0x383b0dc957c44bc30e1199cccf10da0a4b332b9c07e800a620df21dd251df7e3
#define PRODUCT_STATUS_ID             "0x383b0dc9"

// 1.0 deprecated
//#define ROPSTEN_LICENSE_CONTRACT     "0x21027DD05168A559330649721D3600196aB0aeC2"

// Ropsten activate and product contracts
#define ROPSTEN_ACTIVATE_CONTRACT     "0x5A516379F798b1D5b1875fb3efDCdbCfe199De42"
#define GANACHE_ACTIVATE_CONTRACT     "0x67B5656d60a809915323Bf2C40A8bEF15A152e3e"

#define ROPSTEN_AUTHENTICATE_CONTRACT "0x4DA001F154683A67F5B817229c34Ce50aa6F3281"
#define GANACHE_AUTHENTICATE_CONTRACT "0xD833215cBcc3f914bD1C9ece3EE7BF8B14f841bb"

/***********************************************************************/
/* Global function declarations                                        */
/***********************************************************************/
int EthereumValidateActivation(ui64 entityId, ui64 productId,
  char* hashId, char* infuraId, time_t* exp_date, ui64* languages,
  ui64* version_plat);

int EthereumAuthenticateFile(const char* hashId, const char* infuraId,
  ui64* entityId, ui64* productId, ui64* releaseId, ui64* languages,
  ui64* version, char* uri);

#endif /* _ETHEREUMCALLS_H */
