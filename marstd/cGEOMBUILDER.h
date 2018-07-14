#ifndef __cGEOMBUILDER_h__
#define __cGEOMBUILDER_h__

//////////////////////////////////////////////////////////////////////
// (c)2002 Marcel Smit
//
// p.smit@hccnet.nl
//
// This code may not be used in a commercial product without my permission.
// If you redistribute it, this message must remain intact.
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
/** @file cVECTOR.h: CVector implementation. */
//////////////////////////////////////////////////////////////////////

#include <math.h>
#include "cDEBUG.h"
#include "cMATRIX.h"

//---------------------------------------------------------------------------

/// Geometry: Geometry builder object.
/**
 * The geometry builder provides methods to dynamically generate geometry like cubes, spheres and cilinders.
 * All generated geometry is transformed by the geometry builder's matrix. Unlike other
 * classes the geometry builder cannot be instantiated by the programmer. The geometry builder class is a so
 * called singleton, which means only one instance exists of which a reference is acquired by called the I() method.
\code
// Example:

CMesh mesh;

CGeomBuilder::I().matrix.push();
CGeomBuilder::I().matrix.identity();
CGeomBuilder::I().matrix.scale(2.0, 3.0, 1.0);

CGeomBuilder::I().cube(mesh);

CGeomBuilder::I().matrix.pop();
\endcode
 */
class CGeomBuilder {

 private:

    CGeomBuilder()
    {
    }
    
    ~CGeomBuilder()
    {
    }
    
 public:
    
	/**
     * Get interface method. Call this to acquire an instace of the geometry builder object.
     */
     
    static CGeomBuilder& I()
    {
        static CGeomBuilder geomBuilder;
        return geomBuilder;
    }
    
 public:
 
    CMatrix matrix;
    
 public:

    void cube(CMesh& mesh)
    {
    }
    
    void cilinder(CMesh& mesh, int sides)
    {
    }
    
    void cone(CMesh& mesh, int sides)
    {
    }
    
    void sphere(CMesh& mesh, int div1, int div2);
    {
    }
    
    void donut(CMesh& mesh)
    {
    }
    
};

#endif
