#include <cinolib/meshes/meshes.h>
#include <cinolib/gl/glcanvas.h>
#include <cinolib/gl/volume_mesh_controls.h>
#include "cinolib/hex_transition_schemes.h"
#include <cinolib/dual_mesh.h>

int main(int argc, char **argv)
{
    using namespace cinolib;


    DrawablePolyhedralmesh<> m(vec3d_from_serialized_xyz(Flat::verts), Flat::faces, Flat::polys, Flat::winding);

    DrawablePolyhedralmesh<> poly_mesh;
    dual_mesh(m, poly_mesh, true);
    poly_mesh.translate(vec3d(m.bbox().delta_x()*1.5,0,0));
    
    
    
    poly_mesh.updateGL();

    GLcanvas gui;
    //VolumeMeshControls<Hexmesh> menu(&hm, &gui, "Hex Mesh Controls");
    gui.push(&m);
    gui.push(&poly_mesh);
    //gui.push(&menu);

    return gui.launch();
}
