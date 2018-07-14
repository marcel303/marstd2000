#ifndef __cVERTEX_h__
#define __cVERTEX_h__

//////////////////////////////////////////////////////////////////////
// (c)2002 Marcel Smit
//
// p.smit@hccnet.nl
//
// This code may not be used in a commercial product without my permission.
// If you redistribute it, this message must remain intact. 
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
/** @file cVERTEX.h: CVertex implementation. */
//////////////////////////////////////////////////////////////////////

#include <vector>

#define VERTEX_TEX_COUNT	2		///< The maximum number of texture coordinates per vertex. Must at least be 1.
#define VERTEX_TEX_COMPONENTS	3		///< The number of texture components. Must at least be 3.

//---------------------------------------------------------------------------

/// Geometry: Vertex type.
/**
 * The vertex type is used to represent vertices for 3D models which can be rendered by eg. OpenGL.
 * The CVertex class is not derivaed from CVector because the operator[] is hard to access when
 * dealing with pointers to CVertex structures. Eg: (*vertex)[0] = 0.0 is IMHO less appealing than
 * vertex->p[0] = 0.0.
 */
class CVertex {

 public:
 
	CVertex();
	~CVertex();

 public:

	CVector p;						///< XYZ position.
	float c[4];						///< RGBA colour. Range [0.0, 1.0].
	float t[VERTEX_TEX_COUNT][VERTEX_TEX_COMPONENTS];	///< UVST texture coordinates. Range [0.0, 1.0].

 public:

	/**
	 * Sets four colour components. Alpha is defined as 1.0 by default.
	 */
	void set_colour(float r, float g, float b, float a=1.0);
	/**
	 * Sets texture coordinates for texture 0. Third texture coordinate defaults to 0.0.
	 */
	void set_texture(float u, float v, float w=0.0);
	/**
	 * Sets texture coordinates for texture texture.
	 * @param texture Index of texture coordinates to set. Range [0..VERTEX_TEX_COUNT-1].
	 * @see set_texture()
	 */
	void set_texture(int texture, float u, float v, float w=0.0);

 public:

	/**
	 * Copies vertex.
	 */
	CVertex& operator=(CVertex vertex);

};

typedef std::vector<CVertex> CVertexPtr;
typedef std::vector<CVertex*> CPVertexPtr;

#include "inline/cVERTEX.inl"

//---------------------------------------------------------------------------

#endif
