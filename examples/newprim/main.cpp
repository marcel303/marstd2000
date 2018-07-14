/* New Primitives Example App.
 * ---------------------------
 * Illustrates how to use the new primitive classes. Old code has been completely
 * removed though.
 */

#include <allegro.h>
#include "../../marstd/marx.h"
#include "../../marstd/marx.cpp"

int main() {

	if (!MARX::testenv_initialize())
		exit(-1);

	CPoly poly;
	for (int i=0; i<5; i++) {
		CEdge& edge = *poly.add(new CEdge);
		edge.p[0] = SCREEN_W*0.5 + sin(i*0.2*2.0*M_PI) * SCREEN_H/3.0;
		edge.p[1] = SCREEN_H*0.5 + cos(i*0.2*2.0*M_PI) * SCREEN_H/3.0;
	}
	poly.init();

  	while (!key[KEY_ESC]) {

		static int frame = 0;

		CEdge* edge1 = poly.edge_head;
  		while (edge1) {
			CEdge* edge2 = edge1->next ? edge1->next : poly.edge_head;

			CVector tmp1, tmp2;
			tmp1 = edge1->p;
			tmp2 = edge2->p;

			// Draw edge.
  			line(screen, (int)tmp1[0], (int)tmp1[1], (int)tmp2[0], (int)tmp2[1], makecol(255, 255, 255));

  			// Draw edge normal.
			CVector mid = (tmp1 + tmp2) * 0.5;
			CVector tmp3 = mid + edge1->plane.normal * 20.0;
  			line(screen, (int)mid[0], (int)mid[1], (int)tmp3[0], (int)tmp3[1], makecol(255, 255, 0));

  			// Draw edge edge_normal.
  			tmp3 = edge1->p + edge1->edge_plane.normal * 10000.0;
  			line(screen, (int)tmp1[0], (int)tmp1[1], (int)tmp3[0], (int)tmp3[1], makecol(0, 255, 0));

  			edge1 = edge1->next;
  		}

  		frame++;

	}	
		
	if (!MARX::testenv_shutdown())
 		exit(-1);
   
	return 0;     		

} END_OF_MAIN();
