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
//   Both should be for Ropsten or Main Polygon Network
#define CURL_HOST_URL              /*LOCAL_GANACHE_URL*/POLYGON_PUBLIC_URL
#define IMMUTABLE_ACTIVATE_CONTRACT /*GANACHE_ACTIVATE_CONTRACT*/POLYGON_ACTIVATE_CONTRACT
#define IMMUTABLE_CREATOR_CONTRACT /*GANACHE_CREATOR_CONTRACT*/POLYGON_CREATOR_CONTRACT

// Options are Polygon, Ropsten or Local Ganache. Default is public Polygon.
#define POLYGON_PUBLIC_URL         "https://polygon-rpc.com/"
#define POLYGON_INFURA_URL         "https://polygon-mainnet.infura.io/v3/"
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

// Current 2.0
// Keccak256 ("activateStatus(uint256,uint256,uint256)") =
// 0x1da7d8648240b9b4db8c4f11fcd46bf2ccd74be6fdf20e075e3cd1541a3ecae1
#define ACTIVATE_STATUS_ID            "0x1da7d864"
// Current 2.0
// Keccak256 ("creatorReleaseHashDetails(uint256)") =
// 0x2d768793fda5eb47e3b2a15fe3b4fd968f844eb7a9f464990378245ba1e00607
#define CREATOR_STATUS_ID             "0x2d768793"

// 1.0 deprecated
//#define ROPSTEN_LICENSE_CONTRACT     "0x21027DD05168A559330649721D3600196aB0aeC2"

// Polygon, Ropsten and testnet activate and creator token contracts
#define POLYGON_ACTIVATE_CONTRACT     "0x5464dA569E4E93b0bd2BB7d4D46936B1E17E2642"
#define ROPSTEN_ACTIVATE_CONTRACT     "0xfD782aC79E8862247b0128849D3D880a8194A18B"
#define GANACHE_ACTIVATE_CONTRACT     "0x67B5656d60a809915323Bf2C40A8bEF15A152e3e"

#define POLYGON_CREATOR_CONTRACT "0x02a5d5C9c22eeDfAbE54c42Cd81F907Ffb27567C"
#define ROPSTEN_CREATOR_CONTRACT "0xA33A9545e0b8cf4F541fbe593E32EeA2d705c67b"
#define GANACHE_CREATOR_CONTRACT "0xD833215cBcc3f914bD1C9ece3EE7BF8B14f841bb"

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
