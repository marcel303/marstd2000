#include "framework.h"
#include "marstd.h"

static void render();

int main(int argc, char* argv[])
{

	setupPaths(CHIBI_RESOURCE_PATHS);
	
	framework.init(800, 600);
    
    while (!keyboard.wentDown(SDLK_ESCAPE))
    {

		framework.process();
		
        render();

    }

}

static void render()
{

    static float r = 0;
    
    r += 0.01;
    
    CBrush brush;

	int viewSx;
	int viewSy;
	framework.getCurrentViewportSize(viewSx, viewSy);
	
    int cx = viewSx / 2;
    int cy = viewSy / 2;
    
    for (int i = 0; i < 5; i++)
    {
		
        CPlane* plane = brush.add(new CPlane);
        
        plane->normal[0] = sin(r + i * 2.0 * M_PI / 5.0);
        plane->normal[2] = cos(r + i * 2.0 * M_PI / 5.0);

        plane->distance = 50;
        
    }
    
    #if 1
    
    CPlane* pTop = brush.add(new CPlane);
    pTop->normal[1] = 1;
    pTop->distance = 100;
    CPlane* pBottom = brush.add(new CPlane);
    pBottom->normal[1] = -1;
    pBottom->distance = 100;
    
    #endif

    CMesh* mesh = brush.mesh();
    
    CPoly* poly = mesh->poly_head;
	
    framework.beginDraw(0, 0, 0, 0);
	
    while (poly)
    {
        CEdge* edge = poly->edge_head;
        while (edge)
        {
//            edge->p[2] = 0;
//            edge->edge2->p[2] = 0;
			setColor(colorWhite);
            drawLine(cx+(int)(edge->p[0]+edge->p[2]*0.5), cy+(int)(edge->p[1]+edge->p[2]*0.5), cx+(int)(edge->edge2->p[0]+edge->edge2->p[2]*0.5), cy+(int)(edge->edge2->p[1]+edge->edge2->p[2]*0.5));
            edge = edge->next;
        }
        poly = poly->next;
    }
	
    framework.endDraw();
    
    delete mesh;

}
