#include <cinolib/meshes/meshes.h>
#include <cinolib/gl/glcanvas.h>
#include <cinolib/gl/volume_mesh_controls.h>
#include "cinolib/hex_transition_schemes.h"
#include <cinolib/dual_mesh.h>

#include <cinolib/meshes/polyhedralmesh.h>
#include <cinolib/export_cluster.h>

#define T_PATTERN Vert_side_WB
#define STR_(x) #x
#define STR(x) STR_(x)

using namespace cinolib;

void mesh_singularity(
    AbstractPolyhedralMesh<> &poly_mesh,
    DrawablePolyhedralmesh<> &poly_singularity)
{
    using namespace cinolib;
    // std::string path = "/singularities/";
    std::vector<std::vector<uint>> polys_vec = poly_mesh.vector_polys();
    for (uint i = 0; i < polys_vec.size(); ++i)
    {
        poly_singularity.clear();
        poly_mesh.poly_data(i).label = i;
        export_cluster(poly_mesh, i, poly_singularity);
        poly_singularity.updateGL();
        // std::cout << "Exporting singularity cluster " << i << std::endl;
        poly_singularity.save(std::string((STR(T_PATTERN)) + std::to_string(i) + ".mesh").c_str());
    }
}

int main(int argc, char **argv)
{
    using namespace cinolib;

    DrawablePolyhedralmesh<> m(vec3d_from_serialized_xyz(T_PATTERN::verts), T_PATTERN::faces, T_PATTERN::polys, T_PATTERN::winding);
    DrawablePolyhedralmesh<> poly_mesh;
    float ang_thresh = float(to_rad(60.0));
    m.edge_mark_sharp_creases(ang_thresh);

    dual_mesh(m, poly_mesh, true);
    poly_mesh.translate(vec3d(m.bbox().delta_x() * 1.5, 0, 0));
    poly_mesh.updateGL();

    poly_mesh.save( ("dual_"+   std::string(STR(T_PATTERN)) + ".mesh").c_str());

    // singularity extraction

    // std::string filename = "poly.mesh";

    DrawablePolyhedralmesh<> poly_singularity;
    mesh_singularity(poly_mesh, poly_singularity);

    // poly_singularity.translate(vec3d(poly_mesh.bbox().delta_x() * 1.5, 0, 0));

    GLcanvas gui;

    // VolumeMeshControls<DrawablePolyhedralmesh<>> menu(&m, &gui, "Hex Mesh Controls");
    // gui.push(&m);
    // gui.push(&poly_mesh);
    gui.push(&poly_singularity);

    // gui.push(&menu);

    // gui.launch();
    return 0;
}
