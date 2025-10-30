#include <cinolib/meshes/meshes.h>
#include <cinolib/gl/glcanvas.h>
#include <cinolib/gl/volume_mesh_controls.h>
#include "cinolib/hex_transition_schemes.h"


int main(int argc, char **argv)
{
    using namespace cinolib;

    // original meshes

    //dual_mesh();

    std::vector<vec3d> vertex_coordinates_whole_mesh;
    vertex_coordinates_whole_mesh.reserve(
        vec3d_from_serialized_xyz(Flat::verts).size() +
        vec3d_from_serialized_xyz(Edge_WB::verts).size()
    );
    vertex_coordinates_whole_mesh.insert(
        vertex_coordinates_whole_mesh.end(),
        vec3d_from_serialized_xyz(Flat::verts).begin(),
        vec3d_from_serialized_xyz(Flat::verts).end()
    );
    vertex_coordinates_whole_mesh.insert(
        vertex_coordinates_whole_mesh.end(),
        vec3d_from_serialized_xyz(Edge_WB::verts).begin(),
        vec3d_from_serialized_xyz(Edge_WB::verts).end()
    );

    DrawablePolyhedralmesh<> m1(vertex_coordinates_whole_mesh, Flat::faces, Flat::polys, Flat::winding);
    std::vector<double> mesh_verts_double = Edge_WB::verts;

    for (int i = 0; i < mesh_verts_double.size(); ++i)
    {
        if (i % 3 == 0)
        {
            double &x = mesh_verts_double[i];
            x = x + 1.5;
        }
    }

    DrawablePolyhedralmesh<> m2(vec3d_from_serialized_xyz(mesh_verts_double), Edge_WB::faces, Edge_WB::polys, Edge_WB::winding);

    GLcanvas gui;
    VolumeMeshControls<DrawablePolyhedralmesh<>> menu(&m1, &gui);
    gui.push(&m1);
    gui.push(&m2);
    gui.push(&menu);

    return gui.launch();
}
