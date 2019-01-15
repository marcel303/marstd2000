#ifndef __marstd_h__
#define __marstd_h__

//////////////////////////////////////////////////////////////////////
// (c)2002 Marcel Smit
//
// p.smit@hccnet.nl
//
// This code may not be used in a commercial product without my permission.
// If you redistribute it, this message must remain intact. 
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
/** @file marstd.h: MARSTD main include file. */
//////////////////////////////////////////////////////////////////////

/**

\mainpage MARSTD/MARX Docs
<TT>
//////////////////////////////////////////////////////////////////////<BR>
// (c)2002 Marcel Smit<BR>
//<BR>
// p.smit@hccnet.nl<BR>
//<BR>
// This code may not be used in a commercial product without my permission.<BR>
// If you redistribute it, this message must remain intact. <BR>
//////////////////////////////////////////////////////////////////////<BR>
</TT>
<BR>
MARSTD is a librairy written by Marcel Smit that provides some basic classes to
define and manipulate geometry. It also includes some math functions, geometry
generators, a file class, and more.<BR>
<BR>
Take a look at the examples for a good introduction to the librairy. Use these docs as a reference to fill in the gaps.
The classes are all meant to be easy and natural to use.<BR>
<BR>
If you wrote some commercial program using this code, I'd like to know. Please mail me if you have.
*/

//#define MARSTD_3DNOW		///< Uncomment to use 3DNow! instructions.

// DEBUG CODE
#include "cDEBUG.h"

// NEW CODE
#include "cLIST.h"		// dll: helper macros
#include "cVECTOR.h"		// geometry: vector
#include "cVERTEX.h"		// geometry: vertex
#include "cSPHERE.h"		// geometry: sphere
#include "cPOLY.h"
#include "cCPOLY.h"
#include "cMATRIX.h"		// math: matrix w/stack
#include "cISOSURFACE.h"	// geometry: metaballs
#include "cFILE.h"		// io: file

// OLD CODE. DON'T USE!
#include "cBSP.h"		// bsp operations
#include "cBRUSH.h"		// geometry: brushes
#include "Ccsg.h"		// geometry: csg
#include "cBEZIER.h"		// geometry: bezier patches

#endif
