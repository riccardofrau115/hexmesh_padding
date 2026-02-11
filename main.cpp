#include <cinolib/meshes/meshes.h>
#include <cinolib/gl/glcanvas.h>
#include <cinolib/gl/volume_mesh_controls.h>
#include "cinolib/hex_transition_schemes.h"
#include <cinolib/dual_mesh.h>

#include <cinolib/meshes/polyhedralmesh.h>
#include <cinolib/export_cluster.h>

#define T_PATTERN Flat
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
        poly_singularity.save(std::string((STR(T_PATTERN)) + std::string("_") + std::to_string(i) + ".mesh").c_str());
    }
}

std::vector<uint> edges_with_one_common_vert(const uint fid, const AbstractPolyhedralMesh<> &hex_mesh)
{
    std::vector<uint> result;

    // Itera su tutti gli edge della mesh
    for (uint eid = 0; eid < hex_mesh.num_edges(); ++eid)
    {
        // Controlla se l'edge ha esattamente un vertice nella faccia
        uint v0 = hex_mesh.edge_vert_id(eid, 0);
        uint v1 = hex_mesh.edge_vert_id(eid, 1);
        bool v0_in_face = hex_mesh.face_contains_vert(fid, v0);
        bool v1_in_face = hex_mesh.face_contains_vert(fid, v1);

        // Aggiungi l'edge se ha esattamente un vertice in comune
        if ((v0_in_face && !v1_in_face) || (!v0_in_face && v1_in_face))
        {
            result.push_back(eid);
        }
    }

    return result;
}

int main(int argc, char **argv)
{
    using namespace cinolib;

    // Vertici del cubo unitario (v0–v7)
    std::vector<vec3d> verts = {};
    verts.push_back(vec3d{0, 0, 0});
    verts.push_back(vec3d{0, 0, 1});
    verts.push_back(vec3d{1, 0, 1});
    verts.push_back(vec3d{1, 0, 0});
    verts.push_back(vec3d{0, 1, 0});
    verts.push_back(vec3d{0, 1, 1});
    verts.push_back(vec3d{1, 1, 1});
    verts.push_back(vec3d{1, 1, 0});

    std::vector<std::vector<uint>> faces = {
        {0, 3, 2, 1}, // bottom
        {4, 5, 6, 7}, // top
        {0, 1, 5, 4}, // front
        {1, 2, 6, 5}, // right
        {2, 3, 7, 6}, // back
        {3, 0, 4, 7}  // left
    };

    std::vector<std::vector<uint>> polys = {{0, 1, 2, 3, 4, 5}};
    std::vector<std::vector<bool>> polys_face_winding = {{true, true, true, true, true, true}};
    DrawablePolyhedralmesh<> poly_mesh(verts, faces, polys, polys_face_winding);

    // DrawablePolyhedralmesh<> poly_mesh(vec3d_from_serialized_xyz(T_PATTERN::verts), T_PATTERN::faces, T_PATTERN::polys, T_PATTERN::winding);
    // const char *filename = "transition_dualized_polys/Flat/Flat_3.mesh";
    // std::vector<vec3d> verts;
    // std::vector<std::vector<uint>> polys;
    // std::vector<int> vert_labels;
    // std::vector<int> poly_labels;
    // read_MESH(filename, verts, polys, vert_labels, poly_labels);
    // poly_mesh.init(verts, polys, vert_labels, poly_labels);
    // // Per un poliedro con pid = 0
    // uint pid = 0;
    // // 1. Ottieni tutte le facce del poliedro
    // std::vector<uint> poly_faces = poly_mesh.adj_p2f(pid);
    // // 2. Scegli la faccia che vuoi usare come piano di divisione
    // // (es. la seconda faccia)
    // uint split_fid = poly_faces[1];
    // // 3. Ottieni i vertici di quella faccia
    // std::vector<uint> split_face_verts = poly_mesh.face_verts_id(split_fid);
    // // 4. Esegui lo split
    // uint new_fid = poly_mesh.poly_split_along_new_face(pid, split_face_verts);
    // poly_mesh.update_bbox();
    // poly_mesh.update_quality();
    // poly_mesh.update_normals();

    std::vector<uint> first_poly_vids, second_poly_vids;
    uint fid = 0;            
    uint pid = 0;            
    double lambda = 1 / 3.0; // posizione di split lungo l'edge
    
    std::vector<uint> vids = poly_mesh.face_verts_id(fid); // o mesh.adj_f2v(fid)
    std::vector<uint> eids;
    
    for (uint vid : vids)
    {
        std::vector<uint> new_eids = poly_mesh.poly_v2e(pid, vid);  
        eids.insert(eids.end(), new_eids.begin(), new_eids.end());
    }
    REMOVE_DUPLICATES_FROM_VEC(eids);


    for (uint eid : eids)
    {
        uint v0 = poly_mesh.edge_vert_id(eid, 0);
        uint v1 = poly_mesh.edge_vert_id(eid, 1);
        bool v0_in_face = poly_mesh.face_contains_vert(fid, v0);
        bool v1_in_face = poly_mesh.face_contains_vert(fid, v1);
        // Solo edge con esattamente un vertice nella faccia
        if (v0_in_face != v1_in_face)
        {
            // Calcola il punto più vicino alla faccia
            vec3d split_point;
            if (v0_in_face)
            {
                split_point = poly_mesh.edge_sample_at(eid, lambda);
            }
            else
            {
                split_point = poly_mesh.edge_sample_at(eid, 1.0 - lambda);
            }
            uint new_vid = poly_mesh.vert_add(split_point);
        }
    }

    first_poly_vids.clear();
    second_poly_vids.clear();

    first_poly_vids.push_back(0);
    first_poly_vids.push_back(1);
    first_poly_vids.push_back(2);
    first_poly_vids.push_back(3);

    first_poly_vids.push_back(9);
    first_poly_vids.push_back(8);
    first_poly_vids.push_back(10);
    first_poly_vids.push_back(11);

    second_poly_vids.push_back(9);
    second_poly_vids.push_back(8);
    second_poly_vids.push_back(10);
    second_poly_vids.push_back(11);

    second_poly_vids.push_back(4);
    second_poly_vids.push_back(5);
    second_poly_vids.push_back(6);
    second_poly_vids.push_back(7);

    uint first_polid = poly_mesh.poly_add(first_poly_vids);
    uint second_polid = poly_mesh.poly_add(second_poly_vids);
    poly_mesh.poly_remove(0); // rimuovo il poliedro originale

    poly_mesh.update_bbox();
    poly_mesh.update_quality();
    poly_mesh.update_normals();

    poly_mesh.updateGL();

    // Stampa tutti i vertici di tutti i poliedri presenti nella mesh
    for (uint pid = 0; pid < poly_mesh.num_polys(); ++pid)
    {
        std::cout << "\n=== Poliedro " << pid << " ===" << std::endl;
        std::vector<uint> poly_verts = poly_mesh.poly_verts_id(pid);
        std::cout << "Vertici (" << poly_verts.size() << "): ";
        for (uint vid : poly_verts)
        {
            std::cout << vid << " ";
        }
        std::cout << std::endl;
        std::cout << "facce (" << poly_mesh.adj_p2f(pid).size() << "): ";
        uint count = 0;
        for (uint fid : poly_mesh.adj_p2f(pid))
        {
            std::cout << fid << count << " ";
            count++;
        }
    }

    // Ordinamento dei vertici ?TODO?
    // Divido i vertici tra base inferiore e superiore, calcolando l'asse principale
    // Ordinamento angolare delle due facce top e bottom rispetto all'asse principale
    // Allineamento di fase: minimizzazione della torsione

    // Stampa tutte le facce con vertici ed edge
    // for (uint fid = 0; fid < poly_mesh.num_faces(); ++fid)
    // {
    //     std::cout << "\n=== Faccia " << fid << " ===" << std::endl;
    //     // Stampa vertici della faccia
    //     std::vector<uint> face_verts = poly_mesh.face_verts_id(fid);
    //     std::cout << "Vertici (" << face_verts.size() << "): ";
    //     for (uint vid : face_verts)
    //     {
    //         std::cout << vid << " ";
    //     }
    //     std::cout << std::endl;
    //     // Stampa edge della faccia
    //     std::vector<uint> face_edges = poly_mesh.adj_f2e(fid);
    //     std::cout << "Edge (" << face_edges.size() << "): " << std::endl;
    //     for (uint eid : face_edges)
    //     {
    //         uint v0 = poly_mesh.edge_vert_id(eid, 0);
    //         uint v1 = poly_mesh.edge_vert_id(eid, 1);
    //         std::cout << "  Edge " << eid << ": [" << v0 << ", " << v1 << "]" << std::endl;
    //     }
    // }

    // Dualizzazione della mesh
    // float ang_thresh = float(to_rad(60.0));
    // poly_mesh.edge_mark_sharp_creases(ang_thresh);
    // dual_mesh(poly_mesh, poly_mesh, true);
    // poly_mesh.translate(vec3d(poly_mesh.bbox().delta_x() * 1.5, 0, 0));
    // poly_mesh.updateGL();
    // poly_mesh.save( ("dual_"+   std::string(STR(T_PATTERN)) + ".mesh").c_str());
    // singularity extraction
    // std::string filename = "poly.mesh";
    // DrawablePolyhedralmesh<> poly_singularity;
    // mesh_singularity(poly_mesh, poly_singularity);
    // poly_singularity.translate(vec3d(poly_mesh.bbox().delta_x() * 1.5, 0, 0));

    // Aggiungi marker con l'ID di ogni edge
    // for (uint eid = 0; eid < poly_mesh.num_edges(); ++eid)
    // {
    //     // Calcola il punto medio dell'edge
    //     vec3d v0 = poly_mesh.edge_vert(eid, 0);
    //     vec3d v1 = poly_mesh.edge_vert(eid, 1);
    //     vec3d midpoint = (v0 + v1) * 0.5;
    //     // Aggiungi marker con l'ID dell'edge
    //     gui.push_marker(midpoint, std::to_string(eid), Color::BLACK(), 2.0f, 12.0f);
    // }

    GLcanvas gui;
    VolumeMeshControls<DrawablePolyhedralmesh<>> menu(&poly_mesh, &gui, "Hex Mesh Controls");

    // gui.push(&poly_mesh);
    gui.push(&poly_mesh);
    // gui.push(&poly_singularity);

    gui.push(&menu);

    gui.launch();
    return 0;
}
