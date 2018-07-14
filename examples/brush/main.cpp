#include <allegro.h>
#include "marstd.h"

static void render();

int main(int argc, char* argv[])
{

    allegro_init();
    install_keyboard();
    set_color_depth(desktop_color_depth());
    int w, h;
    get_desktop_resolution(&w, &h);
    set_gfx_mode(GFX_AUTODETECT_WINDOWED, w*3/4, h*3/4, 0, 0);
    
    while (!key[KEY_ESC])
    {

        render();

    }

} END_OF_MAIN();

static void render()
{

    static float r = 0;
    
    r += 0.01;
    
    CBrush brush;

    int cx = SCREEN_W / 2;
    int cy = SCREEN_H / 2;
    
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
    
    vsync();

    clear(screen);
    
    while (poly)
    {
        CEdge* edge = poly->edge_head;
        while (edge)
        {
//            edge->p[2] = 0;
//            edge->edge2->p[2] = 0;
            line(screen, cx+(int)(edge->p[0]+edge->p[2]*0.5), cy+(int)(edge->p[1]+edge->p[2]*0.5), cx+(int)(edge->edge2->p[0]+edge->edge2->p[2]*0.5), cy+(int)(edge->edge2->p[1]+edge->edge2->p[2]*0.5), makecol(255, 255, 255));
            edge = edge->next;
        }
        poly = poly->next;
    }
    
    delete mesh;

}
