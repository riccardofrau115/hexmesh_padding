#include <cinolib/meshes/meshes.h>
#include <cinolib/gl/glcanvas.h>
#include <cinolib/gl/volume_mesh_controls.h>
#include "cinolib/hex_transition_schemes.h"
#include <cinolib/dual_mesh.h>

#include <cinolib/meshes/polyhedralmesh.h>

using namespace cinolib;

void mesh_singularity(
    const AbstractPolyhedralMesh<> &primal,
    Polyhedralmesh<> &poly_singularity)
{
    std::vector<std::vector<uint>> polys_vec = primal.vector_polys();
    poly_singularity.clear();
    // poly_singularity.poly_add(polys_vec[0]);
    auto poly = polys_vec[0];
    for (size_t i = 0; i < poly.size(); i++)
    {
        //std::cout << "Vertex " << i << " of poly p is vert id " << poly[i] << std::endl;
        poly_singularity.vert_add(primal.vert(poly[i]));
    }

}

int main(int argc, char **argv)
{
    using namespace cinolib;

    DrawablePolyhedralmesh<> mesh_basis(vec3d_from_serialized_xyz(Flat::verts), Flat::faces, Flat::polys, Flat::winding);
    DrawablePolyhedralmesh<> poly_mesh;
    float ang_thresh = float(to_rad(60.0));
    mesh_basis.edge_mark_sharp_creases(ang_thresh);

    dual_mesh(mesh_basis, poly_mesh, true);
    poly_mesh.translate(vec3d(mesh_basis.bbox().delta_x() * 1.5, 0, 0));
    poly_mesh.updateGL();

    // singularity extraction
    DrawablePolyhedralmesh<> poly_singularity;
    mesh_singularity(poly_mesh, poly_singularity);
    
    poly_singularity.translate(vec3d(poly_mesh.bbox().delta_x() * 1.5, 0, 0));
    poly_singularity.updateGL();


    GLcanvas gui;
    VolumeMeshControls<DrawablePolyhedralmesh<>> menu(&poly_mesh, &gui, "Hex Mesh Controls");
    gui.push(&mesh_basis);
    gui.push(&poly_mesh);
    gui.push(&poly_singularity);
    gui.push(&menu);

    return gui.launch();
}
