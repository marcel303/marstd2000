#ifndef __aCOLLISION_h__
#define __aCOLLISION_h__

#if 0
/**
 * Moving sphere -> edge collision detection..
 *   (C) Marcel Smit 2002. p.smit@hccnet.nl.
 * @param sphere Sphere that will move along a linear path.
 * @param delta Distance the sphere moves this timeslice.
 * @param edge_v1 First vertex of edge.
 * @param edge_v2 Second vertex of edge.
 * @param impact Vector that will store the point of collision.
 * @param t Float that will store the timeslice travelled until impact.
 * @return True if collision occured, false otherwise.
 */
#endif

bool sp_collision(CSphere sphere, CVector delta, CPoly* poly, CVector& impact, float& t);

#endif
