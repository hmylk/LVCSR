/*******************************************************************************
 *
 *                       HCCL PROPRIETARY INFORMATION
 *
 * This software is supplied under the terms of the license agreement
 *   or nondisclosure agreement with HCCL and may not be copied
 * or disclosed except in accordance with the terms of that agreement.
 *
 *                Copyright (c) 2002 HCCL. All Rights Reserved.
 *
 *******************************************************************************/

#ifndef aaaDECODER_NODE_OPTIONSaaa
#define aaaDECODER_NODE_OPTIONSaaa

#define OPTION_WE 0x01            // WE node
#define OPTION_SP 0x02            // tee node
#define OPTION_SOLE 0x04            // sole child node
#define OPTION_SE 0x08            // sentence end node
#define OPTION_SME 0x10		 // the state node of the end of a mono phone
#define OPTION_SLE 0x20		// the state node of the end of a syllable;forbidden to use
#define OPTION_SWE 0x40       // the state node of the end of a word
#define OPTION_SSE 0x80		// the state node of the end of a state phone
#define OPTION_MFAKE 0x100      // the state node with multiple parent
#define OPTION_ENTER 0x200
#define OPTION_SFI 0x400
#define OPTION_FAKE 0x800
#define OPTION_EXIT 0x1000
#define OPTION_SSF 0x2000
#define OPTION_SVWE 0x4000
#define OPTION_FWE 0x8000

#endif





