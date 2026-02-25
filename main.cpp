#include <cinolib/meshes/meshes.h>
#include <cinolib/gl/glcanvas.h>
#include <cinolib/gl/volume_mesh_controls.h>
#include "cinolib/hex_transition_schemes.h"
#include <cinolib/dual_mesh.h>
#include <cinolib/meshes/polyhedralmesh.h>
#include <cinolib/export_cluster.h>
#include <cinolib/hex_shift_indices.h>

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

void hex_rebase(const uint hex_in[8],
                const int offset,
                uint hex_out[8])
{

    switch (offset)
    {

    case 0:
        hex_out[0] = hex_in[0];
        hex_out[1] = hex_in[1];
        hex_out[2] = hex_in[2];
        hex_out[3] = hex_in[3];
        hex_out[4] = hex_in[4];
        hex_out[5] = hex_in[5];
        hex_out[6] = hex_in[6];
        hex_out[7] = hex_in[7];
        return;

    case 1:
        hex_out[0] = hex_in[7];
        hex_out[1] = hex_in[6];
        hex_out[2] = hex_in[5];
        hex_out[3] = hex_in[4];
        hex_out[4] = hex_in[3];
        hex_out[5] = hex_in[2];
        hex_out[6] = hex_in[1];
        hex_out[7] = hex_in[0];
        return;

    case 2:
        hex_out[0] = hex_in[4];
        hex_out[1] = hex_in[5];
        hex_out[2] = hex_in[1];
        hex_out[3] = hex_in[0];
        hex_out[4] = hex_in[7];
        hex_out[5] = hex_in[6];
        hex_out[6] = hex_in[2];
        hex_out[7] = hex_in[3];
        return;

    case 3:
        hex_out[0] = hex_in[6];
        hex_out[1] = hex_in[5];
        hex_out[2] = hex_in[1];
        hex_out[3] = hex_in[2];
        hex_out[4] = hex_in[7];
        hex_out[5] = hex_in[4];
        hex_out[6] = hex_in[0];
        hex_out[7] = hex_in[3];
        return;

    case 4:
        hex_out[0] = hex_in[7];
        hex_out[1] = hex_in[6];
        hex_out[2] = hex_in[2];
        hex_out[3] = hex_in[3];
        hex_out[4] = hex_in[4];
        hex_out[5] = hex_in[5];
        hex_out[6] = hex_in[1];
        hex_out[7] = hex_in[0];
        return;

    case 5:
        hex_out[0] = hex_in[4];
        hex_out[1] = hex_in[7];
        hex_out[2] = hex_in[3];
        hex_out[3] = hex_in[0];
        hex_out[4] = hex_in[5];
        hex_out[5] = hex_in[6];
        hex_out[6] = hex_in[2];
        hex_out[7] = hex_in[1];
        return;

    default:
        throw std::runtime_error("Invalid Hex Offset");

        return;
    }
}

void hex_around_axis(const uint hex_in[8],
                     const int offset,
                     uint hex_out[8])
{
    switch (offset)
    {
    case 0:
        hex_out[0] = hex_in[0];
        hex_out[1] = hex_in[1];
        hex_out[2] = hex_in[2];
        hex_out[3] = hex_in[3];
        hex_out[4] = hex_in[4];
        hex_out[5] = hex_in[5];
        hex_out[6] = hex_in[6];
        hex_out[7] = hex_in[7];
        break;

    case 1:
        hex_out[0] = hex_in[1];
        hex_out[1] = hex_in[2];
        hex_out[2] = hex_in[3];
        hex_out[3] = hex_in[0];
        hex_out[4] = hex_in[5];
        hex_out[5] = hex_in[6];
        hex_out[6] = hex_in[7];
        hex_out[7] = hex_in[4];
        break;

    case 2:
        hex_out[0] = hex_in[2];
        hex_out[1] = hex_in[3];
        hex_out[2] = hex_in[0];
        hex_out[3] = hex_in[1];
        hex_out[4] = hex_in[6];
        hex_out[5] = hex_in[7];
        hex_out[6] = hex_in[4];
        hex_out[7] = hex_in[5];
        break;

    case 3:
        hex_out[0] = hex_in[3];
        hex_out[1] = hex_in[0];
        hex_out[2] = hex_in[1];
        hex_out[3] = hex_in[2];
        hex_out[4] = hex_in[7];
        hex_out[5] = hex_in[4];
        hex_out[6] = hex_in[5];
        hex_out[7] = hex_in[6];
        break;
    }
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

    uint pid = 0;
    std::vector<uint> fids_vec;
    fids_vec.push_back(0);
    // fids_vec.push_back(1);
    fids_vec.push_back(2);
    // fids_vec.push_back(3);
    // fids_vec.push_back(4);
    // fids_vec.push_back(5);
    uint fid_first = fids_vec[0]; // Scegli la prima faccia del poliedro
    double lambda = 1 / 3.0;      // posizione di split lungo l'edge

    std::vector<uint> faces_of_poly = {0, 1, 2, 3, 4, 5};

    vec3d split_point;
    std::vector<uint> verts_og = poly_mesh.poly_verts_id(0);
    std::vector<uint> poly_first_vids, poly_second_vids, poly_third_vids, poly_fourth_vids, poly_fifth_vids, poly_sixth_vids, poly_seventh_vids;
    std::vector<uint> verts_arranged = verts_og;
    std::vector<uint> new_vids;
    hex_rebase(verts_og.data(), 2, verts_arranged.data());

    switch (fids_vec.size())
    {
    case 1:
    {
        std::cout << "Split along face " << fids_vec[0] << std::endl;
        // padding della faccia 0

        split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[0]) + lambda * poly_mesh.vert(verts_arranged[4]);
        new_vids.push_back(poly_mesh.vert_add(split_point));
        split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[1]) + lambda * poly_mesh.vert(verts_arranged[5]);
        new_vids.push_back(poly_mesh.vert_add(split_point));
        split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[2]) + lambda * poly_mesh.vert(verts_arranged[6]);
        new_vids.push_back(poly_mesh.vert_add(split_point));
        split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[3]) + lambda * poly_mesh.vert(verts_arranged[7]);
        new_vids.push_back(poly_mesh.vert_add(split_point));

        poly_first_vids.clear();

        poly_first_vids.push_back(verts_arranged[0]);
        poly_first_vids.push_back(verts_arranged[1]);
        poly_first_vids.push_back(verts_arranged[2]);
        poly_first_vids.push_back(verts_arranged[3]);
        poly_first_vids.push_back(new_vids[0]);
        poly_first_vids.push_back(new_vids[1]);
        poly_first_vids.push_back(new_vids[2]);
        poly_first_vids.push_back(new_vids[3]);

        poly_second_vids.clear();

        poly_second_vids.push_back(new_vids[0]);
        poly_second_vids.push_back(new_vids[1]);
        poly_second_vids.push_back(new_vids[2]);
        poly_second_vids.push_back(new_vids[3]);
        poly_second_vids.push_back(verts_arranged[4]);
        poly_second_vids.push_back(verts_arranged[5]);
        poly_second_vids.push_back(verts_arranged[6]);
        poly_second_vids.push_back(verts_arranged[7]);

        poly_mesh.poly_add(poly_first_vids);
        poly_mesh.poly_add(poly_second_vids);

        break;
    }
    case 2:
    {
        std::cout << "Split along faces " << fids_vec[0] << " and " << fids_vec[1] << std::endl;
        /// caso in cui le facce siano opposte
        if (!poly_mesh.faces_are_adjacent(fids_vec[0], fids_vec[1]))
        {
            /// padding delle facce 3 e 5

            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[0]) + lambda * poly_mesh.vert(verts_arranged[1]);
            uint AB = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[1]) + lambda * poly_mesh.vert(verts_arranged[0]);
            uint BA = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[2]) + lambda * poly_mesh.vert(verts_arranged[3]);
            uint CD = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[3]) + lambda * poly_mesh.vert(verts_arranged[2]);
            uint DC = (poly_mesh.vert_add(split_point));

            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[4]) + lambda * poly_mesh.vert(verts_arranged[5]);
            uint EF = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[5]) + lambda * poly_mesh.vert(verts_arranged[4]);
            uint FE = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[6]) + lambda * poly_mesh.vert(verts_arranged[7]);
            uint GH = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[7]) + lambda * poly_mesh.vert(verts_arranged[6]);
            uint HG = (poly_mesh.vert_add(split_point));

            // AB, BA, CD, DC, EF, FE, GH, HG;
            poly_first_vids.clear();
            poly_first_vids.push_back(AB);
            poly_first_vids.push_back(BA);
            poly_first_vids.push_back(CD);
            poly_first_vids.push_back(DC);
            poly_first_vids.push_back(EF);
            poly_first_vids.push_back(FE);
            poly_first_vids.push_back(GH);
            poly_first_vids.push_back(HG);

            // A, AB, DC, D, E, EF, HG, H;
            poly_second_vids.clear();
            poly_second_vids.push_back(verts_arranged[0]); // A
            poly_second_vids.push_back(AB);
            poly_second_vids.push_back(DC);
            poly_second_vids.push_back(verts_arranged[3]); // D
            poly_second_vids.push_back(verts_arranged[4]); // E
            poly_second_vids.push_back(EF);
            poly_second_vids.push_back(HG);
            poly_second_vids.push_back(verts_arranged[7]); // H

            // BA, B, C, CD, FE, F, G, GH
            poly_third_vids.clear();
            poly_third_vids.push_back(BA);
            poly_third_vids.push_back(verts_arranged[1]); // B
            poly_third_vids.push_back(verts_arranged[2]); // C
            poly_third_vids.push_back(CD);
            poly_third_vids.push_back(FE);
            poly_third_vids.push_back(verts_arranged[5]); // F
            poly_third_vids.push_back(verts_arranged[6]); // G
            poly_third_vids.push_back(GH);

            poly_mesh.poly_add(poly_first_vids);
            poly_mesh.poly_add(poly_second_vids);
            poly_mesh.poly_add(poly_third_vids);
        }
        else // caso in cui le facce siano adiacenti
        {
            // padding delle facce 4 e 5

            // bottom face
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[0]) + lambda * poly_mesh.vert(verts_arranged[1]);
            uint AB = poly_mesh.vert_add(split_point);
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[2]) + lambda * poly_mesh.vert(verts_arranged[1]);
            uint CB = poly_mesh.vert_add(split_point);
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[3]) + lambda * poly_mesh.vert(verts_arranged[1]);
            uint DB = poly_mesh.vert_add(split_point);
            // top face
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[4]) + lambda * poly_mesh.vert(verts_arranged[5]);
            uint EF = poly_mesh.vert_add(split_point);
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[6]) + lambda * poly_mesh.vert(verts_arranged[5]);
            uint GF = poly_mesh.vert_add(split_point);
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[7]) + lambda * poly_mesh.vert(verts_arranged[5]);
            uint HF = poly_mesh.vert_add(split_point);

            poly_first_vids.clear();
            poly_first_vids.push_back(AB);
            poly_first_vids.push_back(verts_arranged[1]); // B
            poly_first_vids.push_back(CB);
            poly_first_vids.push_back(DB);
            poly_first_vids.push_back(EF);
            poly_first_vids.push_back(verts_arranged[5]); // F
            poly_first_vids.push_back(GF);
            poly_first_vids.push_back(HF);

            poly_second_vids.clear();
            poly_second_vids.push_back(verts_arranged[0]); // A
            poly_second_vids.push_back(AB);
            poly_second_vids.push_back(DB);
            poly_second_vids.push_back(verts_arranged[3]); // D
            poly_second_vids.push_back(verts_arranged[4]); // E
            poly_second_vids.push_back(EF);
            poly_second_vids.push_back(HF);
            poly_second_vids.push_back(verts_arranged[7]); // H

            poly_third_vids.clear();
            poly_third_vids.push_back(verts_arranged[3]); // D
            poly_third_vids.push_back(DB);
            poly_third_vids.push_back(CB);
            poly_third_vids.push_back(verts_arranged[2]); // C
            poly_third_vids.push_back(verts_arranged[7]); // H
            poly_third_vids.push_back(HF);
            poly_third_vids.push_back(GF);
            poly_third_vids.push_back(verts_arranged[6]); // G

            poly_mesh.poly_add(poly_first_vids);
            poly_mesh.poly_add(poly_second_vids);
            poly_mesh.poly_add(poly_third_vids);
        }
        break;
    }
    case 3:
    {
        std::cout << "Split along faces " << fids_vec[0] << ", " << fids_vec[1] << " and " << fids_vec[2] << std::endl;

        // facce ad angolo, tutte le facce sono adiacenti tra loro

        bool corner =
            (poly_mesh.faces_are_adjacent(fids_vec[0], fids_vec[1]) &&
             poly_mesh.faces_are_adjacent(fids_vec[1], fids_vec[2]) &&
             poly_mesh.faces_are_adjacent(fids_vec[2], fids_vec[0]));

        if (corner)
        {

            // padding delle facce 0, 2 e 3

            // vertici:

            // bottom -
            // AD 0-3: n0
            // BD 1-3: n1
            // CD 2-3: n2

            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[0]) + lambda * poly_mesh.vert(verts_arranged[3]);
            uint AD = poly_mesh.vert_add(split_point);
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[1]) + lambda * poly_mesh.vert(verts_arranged[3]);
            uint BD = poly_mesh.vert_add(split_point);
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[2]) + lambda * poly_mesh.vert(verts_arranged[3]);
            uint CD = poly_mesh.vert_add(split_point);

            // top -
            // EH 4-7
            // FH 5-7
            // GH 6-7

            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[4]) + lambda * poly_mesh.vert(verts_arranged[7]);
            uint EH = poly_mesh.vert_add(split_point);
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[5]) + lambda * poly_mesh.vert(verts_arranged[7]);
            uint FH = poly_mesh.vert_add(split_point);
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[6]) + lambda * poly_mesh.vert(verts_arranged[7]);
            uint GH = poly_mesh.vert_add(split_point);

            // mid-out -
            // AE 0-4: n6
            // BF 1-5: n7
            // CG 2-6: n8
            // DH 3-7: n9

            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[0]) + lambda * poly_mesh.vert(verts_arranged[4]);
            uint AE = poly_mesh.vert_add(split_point);
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[1]) + lambda * poly_mesh.vert(verts_arranged[5]);
            uint BF = poly_mesh.vert_add(split_point);
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[2]) + lambda * poly_mesh.vert(verts_arranged[6]);
            uint CG = poly_mesh.vert_add(split_point);
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[3]) + lambda * poly_mesh.vert(verts_arranged[7]);
            uint DH = poly_mesh.vert_add(split_point);

            // mid-in -
            // ADEH n0-n3: n10
            // BDFH n1-n4: n11
            // CDGH n2-n5: n12

            split_point = (1 - lambda) * poly_mesh.vert(AD) + lambda * poly_mesh.vert(EH);
            uint ADEH = poly_mesh.vert_add(split_point);
            split_point = (1 - lambda) * poly_mesh.vert(BD) + lambda * poly_mesh.vert(FH);
            uint BDFH = poly_mesh.vert_add(split_point);
            split_point = (1 - lambda) * poly_mesh.vert(CD) + lambda * poly_mesh.vert(GH);
            uint CDGH = poly_mesh.vert_add(split_point);

            // DH ADEH BDFH CDGH H EH FH GH
            poly_first_vids.clear();

            poly_first_vids.push_back(DH);
            poly_first_vids.push_back(ADEH);
            poly_first_vids.push_back(BDFH);
            poly_first_vids.push_back(CDGH);
            poly_first_vids.push_back(verts_arranged[7]); // H
            poly_first_vids.push_back(EH);
            poly_first_vids.push_back(FH);
            poly_first_vids.push_back(GH);

            //  D AD BD CD DH ADEH BDFH CDGH
            poly_second_vids.clear();

            poly_second_vids.push_back(verts_arranged[3]); // D
            poly_second_vids.push_back(AD);
            poly_second_vids.push_back(BD);
            poly_second_vids.push_back(CD);
            poly_second_vids.push_back(DH);
            poly_second_vids.push_back(ADEH);
            poly_second_vids.push_back(BDFH);
            poly_second_vids.push_back(CDGH);

            // C B BD CD CG BF BDFH CDGH
            poly_third_vids.clear();

            poly_third_vids.push_back(verts_arranged[2]); // C
            poly_third_vids.push_back(verts_arranged[1]); // B
            poly_third_vids.push_back(BD);
            poly_third_vids.push_back(CD);
            poly_third_vids.push_back(CG);
            poly_third_vids.push_back(BF);
            poly_third_vids.push_back(BDFH);
            poly_third_vids.push_back(CDGH);

            // B, A, AD, BD, BF, AE, ADEH, BDFH;
            poly_fourth_vids.clear();

            poly_fourth_vids.push_back(verts_arranged[1]); // B
            poly_fourth_vids.push_back(verts_arranged[0]); // A
            poly_fourth_vids.push_back(AD);
            poly_fourth_vids.push_back(BD);
            poly_fourth_vids.push_back(BF);
            poly_fourth_vids.push_back(AE);
            poly_fourth_vids.push_back(ADEH);
            poly_fourth_vids.push_back(BDFH);

            // BF, BDFH, CDGH, CG, F, FH, GH, G;

            poly_fifth_vids.clear();

            poly_fifth_vids.push_back(BF);
            poly_fifth_vids.push_back(BDFH);
            poly_fifth_vids.push_back(CDGH);
            poly_fifth_vids.push_back(CG);
            poly_fifth_vids.push_back(verts_arranged[5]); // F
            poly_fifth_vids.push_back(FH);
            poly_fifth_vids.push_back(GH);
            poly_fifth_vids.push_back(verts_arranged[6]); // G

            // BF, AE, ADEH, BDFH, F, E, EH, FH;
            poly_sixth_vids.clear();

            poly_sixth_vids.push_back(BF);
            poly_sixth_vids.push_back(AE);
            poly_sixth_vids.push_back(ADEH);
            poly_sixth_vids.push_back(BDFH);
            poly_sixth_vids.push_back(verts_arranged[5]); // F
            poly_sixth_vids.push_back(verts_arranged[4]); // E
            poly_sixth_vids.push_back(EH);
            poly_sixth_vids.push_back(FH);

            poly_mesh.poly_add(poly_first_vids);
            poly_mesh.poly_add(poly_second_vids);
            poly_mesh.poly_add(poly_third_vids);
            poly_mesh.poly_add(poly_fourth_vids);
            poly_mesh.poly_add(poly_fifth_vids);
            poly_mesh.poly_add(poly_sixth_vids);
        }
        else // facce a forma di U
        {
            // padding delle facce 3, 4, e 5

            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[0]) + lambda * poly_mesh.vert(verts_arranged[1]);
            uint AB = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[1]) + lambda * poly_mesh.vert(verts_arranged[0]);
            uint BA = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[3]) + lambda * poly_mesh.vert(verts_arranged[1]);
            uint DB = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[2]) + lambda * poly_mesh.vert(verts_arranged[0]);
            uint CA = (poly_mesh.vert_add(split_point));

            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[4]) + lambda * poly_mesh.vert(verts_arranged[5]);
            uint EF = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[5]) + lambda * poly_mesh.vert(verts_arranged[4]);
            uint FE = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[6]) + lambda * poly_mesh.vert(verts_arranged[4]);
            uint GE = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[7]) + lambda * poly_mesh.vert(verts_arranged[5]);
            uint HF = (poly_mesh.vert_add(split_point));

            // AB, BA, CA, DB, EF, FE, GE ,HF

            poly_first_vids.clear();
            poly_first_vids.push_back(AB);
            poly_first_vids.push_back(BA);
            poly_first_vids.push_back(CA);
            poly_first_vids.push_back(DB);
            poly_first_vids.push_back(EF);
            poly_first_vids.push_back(FE);
            poly_first_vids.push_back(GE);
            poly_first_vids.push_back(HF);

            // A, AB, DB, D, E, EF, HF, H;
            poly_second_vids.clear();
            poly_second_vids.push_back(verts_arranged[0]); // A
            poly_second_vids.push_back(AB);
            poly_second_vids.push_back(DB);
            poly_second_vids.push_back(verts_arranged[3]); // D
            poly_second_vids.push_back(verts_arranged[4]); // E
            poly_second_vids.push_back(EF);
            poly_second_vids.push_back(HF);
            poly_second_vids.push_back(verts_arranged[7]); // H

            // D, DB, CA, C, H, HF, GE, G;
            poly_third_vids.clear();
            poly_third_vids.push_back(verts_arranged[3]); // D
            poly_third_vids.push_back(DB);
            poly_third_vids.push_back(CA);
            poly_third_vids.push_back(verts_arranged[2]); // C
            poly_third_vids.push_back(verts_arranged[7]); // H
            poly_third_vids.push_back(HF);
            poly_third_vids.push_back(GE);
            poly_third_vids.push_back(verts_arranged[6]); // G

            // B, C, CA, BA, F, G, GE, FE
            poly_fourth_vids.clear();
            poly_fourth_vids.push_back(verts_arranged[1]); // B
            poly_fourth_vids.push_back(verts_arranged[2]); // C
            poly_fourth_vids.push_back(CA);
            poly_fourth_vids.push_back(BA);
            poly_fourth_vids.push_back(verts_arranged[5]); // F
            poly_fourth_vids.push_back(verts_arranged[6]); // G
            poly_fourth_vids.push_back(GE);
            poly_fourth_vids.push_back(FE);

            poly_mesh.poly_add(poly_first_vids);
            poly_mesh.poly_add(poly_second_vids);
            poly_mesh.poly_add(poly_third_vids);
            poly_mesh.poly_add(poly_fourth_vids);
        }

        break;
    }
    case 4:
    {
        std::cout << "Split along faces " << fids_vec[0] << ", " << fids_vec[1] << ", " << fids_vec[2] << " and " << fids_vec[3] << std::endl;
        std::vector<uint> faces_not_to_pad;
        // verifica che le facce escluse dal padding siano adiacenti

        for (auto face : faces_of_poly)
        {
            if (std::find(fids_vec.begin(), fids_vec.end(), face) == fids_vec.end())
            {
                faces_not_to_pad.push_back(face);
            }
        }

        if (poly_mesh.faces_are_adjacent(faces_not_to_pad[0], faces_not_to_pad[1]))
        {
            // caso in cui le facce da escludere siano adiacenti tra loro
            // padding delle facce 0, 1, 2 e 3

            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[0]) + lambda * poly_mesh.vert(verts_arranged[7]);
            uint AH = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[1]) + lambda * poly_mesh.vert(verts_arranged[7]);
            uint BH = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[2]) + lambda * poly_mesh.vert(verts_arranged[7]);
            uint CH = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[3]) + lambda * poly_mesh.vert(verts_arranged[7]);
            uint DH = (poly_mesh.vert_add(split_point));

            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[4]) + lambda * poly_mesh.vert(verts_arranged[3]);
            uint ED = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[5]) + lambda * poly_mesh.vert(verts_arranged[3]);
            uint FD = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[6]) + lambda * poly_mesh.vert(verts_arranged[3]);
            uint GD = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[7]) + lambda * poly_mesh.vert(verts_arranged[3]);
            uint HD = (poly_mesh.vert_add(split_point));

            // AH, BH, CH, DH, ED, FD, GD, HD
            poly_first_vids.clear();
            poly_first_vids.push_back(AH);
            poly_first_vids.push_back(BH);
            poly_first_vids.push_back(CH);
            poly_first_vids.push_back(DH);
            poly_first_vids.push_back(ED);
            poly_first_vids.push_back(FD);
            poly_first_vids.push_back(GD);
            poly_first_vids.push_back(HD);

            // A, B, C, D, AH, BH, CH, DH;
            poly_second_vids.clear();
            poly_second_vids.push_back(verts_arranged[0]); // A
            poly_second_vids.push_back(verts_arranged[1]); // B
            poly_second_vids.push_back(verts_arranged[2]); // C
            poly_second_vids.push_back(verts_arranged[3]); // D
            poly_second_vids.push_back(AH);
            poly_second_vids.push_back(BH);
            poly_second_vids.push_back(CH);
            poly_second_vids.push_back(DH);

            // ED, FD, GD, HD, E, F, G, H;
            poly_third_vids.clear();
            poly_third_vids.push_back(ED);
            poly_third_vids.push_back(FD);
            poly_third_vids.push_back(GD);
            poly_third_vids.push_back(HD);
            poly_third_vids.push_back(verts_arranged[4]); // E
            poly_third_vids.push_back(verts_arranged[5]); // F
            poly_third_vids.push_back(verts_arranged[6]); // G
            poly_third_vids.push_back(verts_arranged[7]); // H

            // B, C, CH, BH, F, G, GD, FD;

            poly_fourth_vids.clear();
            poly_fourth_vids.push_back(verts_arranged[1]); // B
            poly_fourth_vids.push_back(verts_arranged[2]); // C
            poly_fourth_vids.push_back(CH);
            poly_fourth_vids.push_back(BH);
            poly_fourth_vids.push_back(verts_arranged[5]); // F
            poly_fourth_vids.push_back(verts_arranged[6]); // G
            poly_fourth_vids.push_back(GD);
            poly_fourth_vids.push_back(FD);

            // A, B, BH, AH, E, F, FD, ED

            poly_fifth_vids.clear();
            poly_fifth_vids.push_back(verts_arranged[0]); // A
            poly_fifth_vids.push_back(verts_arranged[1]); // B
            poly_fifth_vids.push_back(BH);
            poly_fifth_vids.push_back(AH);
            poly_fifth_vids.push_back(verts_arranged[4]); // E
            poly_fifth_vids.push_back(verts_arranged[5]); // F
            poly_fifth_vids.push_back(FD);
            poly_fifth_vids.push_back(ED);

            poly_mesh.poly_add(poly_first_vids);
            poly_mesh.poly_add(poly_second_vids);
            poly_mesh.poly_add(poly_third_vids);
            poly_mesh.poly_add(poly_fourth_vids);
            poly_mesh.poly_add(poly_fifth_vids);
        }
        else
        {
            // caso in cui le facce da escludere siano opposte
            // padding delle facce 2, 3, 4 e 5

            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[0]) + lambda * poly_mesh.vert(verts_arranged[2]);
            uint AC = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[1]) + lambda * poly_mesh.vert(verts_arranged[3]);
            uint BD = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[2]) + lambda * poly_mesh.vert(verts_arranged[0]);
            uint CA = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[3]) + lambda * poly_mesh.vert(verts_arranged[1]);
            uint DB = (poly_mesh.vert_add(split_point));

            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[4]) + lambda * poly_mesh.vert(verts_arranged[6]);
            uint EG = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[5]) + lambda * poly_mesh.vert(verts_arranged[7]);
            uint FH = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[6]) + lambda * poly_mesh.vert(verts_arranged[4]);
            uint GE = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[7]) + lambda * poly_mesh.vert(verts_arranged[5]);
            uint HF = (poly_mesh.vert_add(split_point));

            // AC, BD, CA, DB, EG, FH, GF, HF
            poly_first_vids.clear();
            poly_first_vids.push_back(AC);
            poly_first_vids.push_back(BD);
            poly_first_vids.push_back(CA);
            poly_first_vids.push_back(DB);
            poly_first_vids.push_back(EG);
            poly_first_vids.push_back(FH);
            poly_first_vids.push_back(GE);
            poly_first_vids.push_back(HF);

            // A, D, DB, AC, E, H, HF, EG
            poly_second_vids.clear();
            poly_second_vids.push_back(verts_arranged[0]); // A
            poly_second_vids.push_back(verts_arranged[3]); // D
            poly_second_vids.push_back(DB);
            poly_second_vids.push_back(AC);
            poly_second_vids.push_back(verts_arranged[4]); // E
            poly_second_vids.push_back(verts_arranged[7]); // H
            poly_second_vids.push_back(HF);
            poly_second_vids.push_back(EG);

            // DB, CA, C, D, HF, GE, G, H
            poly_third_vids.clear();
            poly_third_vids.push_back(DB);
            poly_third_vids.push_back(CA);
            poly_third_vids.push_back(verts_arranged[2]); // C
            poly_third_vids.push_back(verts_arranged[3]); // D
            poly_third_vids.push_back(HF);
            poly_third_vids.push_back(GE);
            poly_third_vids.push_back(verts_arranged[6]); // G
            poly_third_vids.push_back(verts_arranged[7]); // H

            // B, C, CA, BD, F, G, GE, FH
            poly_fourth_vids.clear();
            poly_fourth_vids.push_back(verts_arranged[1]); // B
            poly_fourth_vids.push_back(verts_arranged[2]); // C
            poly_fourth_vids.push_back(CA);
            poly_fourth_vids.push_back(BD);
            poly_fourth_vids.push_back(verts_arranged[5]); // F
            poly_fourth_vids.push_back(verts_arranged[6]); // G
            poly_fourth_vids.push_back(GE);
            poly_fourth_vids.push_back(FH);

            // B, A, AC, BD, F, E, EG, FH
            poly_fifth_vids.clear();
            poly_fifth_vids.push_back(verts_arranged[1]); // B
            poly_fifth_vids.push_back(verts_arranged[0]); // A
            poly_fifth_vids.push_back(AC);
            poly_fifth_vids.push_back(BD);
            poly_fifth_vids.push_back(verts_arranged[5]); // F
            poly_fifth_vids.push_back(verts_arranged[4]); // E
            poly_fifth_vids.push_back(EG);
            poly_fifth_vids.push_back(FH);

            poly_mesh.poly_add(poly_first_vids);
            poly_mesh.poly_add(poly_second_vids);
            poly_mesh.poly_add(poly_third_vids);
            poly_mesh.poly_add(poly_fourth_vids);
            poly_mesh.poly_add(poly_fifth_vids);
        }

        break;
    }
    case 5:
    {
        std::cout << "Split along faces " << fids_vec[0] << ", " << fids_vec[1] << ", " << fids_vec[2] << ", " << fids_vec[3] << " and " << fids_vec[4] << std::endl;

        split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[0]) + lambda * poly_mesh.vert(verts_arranged[6]);
        uint AG = (poly_mesh.vert_add(split_point));
        split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[1]) + lambda * poly_mesh.vert(verts_arranged[7]);
        uint BH = (poly_mesh.vert_add(split_point));
        split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[2]) + lambda * poly_mesh.vert(verts_arranged[7]);
        uint CH = (poly_mesh.vert_add(split_point));
        split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[3]) + lambda * poly_mesh.vert(verts_arranged[6]);
        uint DG = (poly_mesh.vert_add(split_point));

        split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[4]) + lambda * poly_mesh.vert(verts_arranged[2]);
        uint EC = (poly_mesh.vert_add(split_point));
        split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[5]) + lambda * poly_mesh.vert(verts_arranged[3]);
        uint FD = (poly_mesh.vert_add(split_point));
        split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[6]) + lambda * poly_mesh.vert(verts_arranged[3]);
        uint GD = (poly_mesh.vert_add(split_point));
        split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[7]) + lambda * poly_mesh.vert(verts_arranged[2]);
        uint HC = (poly_mesh.vert_add(split_point));

        // AG, BH, CH DG, EC, FD, GD, HC
        poly_first_vids.clear();
        poly_first_vids.push_back(AG);
        poly_first_vids.push_back(BH);
        poly_first_vids.push_back(CH);
        poly_first_vids.push_back(DG);
        poly_first_vids.push_back(EC);
        poly_first_vids.push_back(FD);
        poly_first_vids.push_back(GD);
        poly_first_vids.push_back(HC);

        // A, B, C, D, AG, BH, CH, DG;
        poly_second_vids.clear();
        poly_second_vids.push_back(verts_arranged[0]); // A
        poly_second_vids.push_back(verts_arranged[1]); // B
        poly_second_vids.push_back(verts_arranged[2]); // C
        poly_second_vids.push_back(verts_arranged[3]); // D
        poly_second_vids.push_back(AG);
        poly_second_vids.push_back(BH);
        poly_second_vids.push_back(CH);
        poly_second_vids.push_back(DG);

        // EC, FD, GD, HC, G, H, E, F;
        poly_third_vids.clear();
        poly_third_vids.push_back(EC);
        poly_third_vids.push_back(FD);
        poly_third_vids.push_back(GD);
        poly_third_vids.push_back(HC);
        poly_third_vids.push_back(verts_arranged[4]); // E
        poly_third_vids.push_back(verts_arranged[5]); // F
        poly_third_vids.push_back(verts_arranged[6]); // G
        poly_third_vids.push_back(verts_arranged[7]); // H

        // A, D, DG, AG, E, H, HC, EC;
        poly_fourth_vids.clear();
        poly_fourth_vids.push_back(verts_arranged[0]); // A
        poly_fourth_vids.push_back(verts_arranged[3]); // D
        poly_fourth_vids.push_back(DG);
        poly_fourth_vids.push_back(AG);
        poly_fourth_vids.push_back(verts_arranged[4]); // E
        poly_fourth_vids.push_back(verts_arranged[7]); // H
        poly_fourth_vids.push_back(HC);
        poly_fourth_vids.push_back(EC);

        // B, C, CH, BH, F, G, GD, FD;

        poly_fifth_vids.clear();
        poly_fifth_vids.push_back(verts_arranged[1]); // B
        poly_fifth_vids.push_back(verts_arranged[2]); // C
        poly_fifth_vids.push_back(CH);
        poly_fifth_vids.push_back(BH);
        poly_fifth_vids.push_back(verts_arranged[5]); // F
        poly_fifth_vids.push_back(verts_arranged[6]); // G
        poly_fifth_vids.push_back(GD);
        poly_fifth_vids.push_back(FD);

        // A, B, BH, AG, E, F, FD, EC;
        poly_sixth_vids.clear();
        poly_sixth_vids.push_back(verts_arranged[0]); // A
        poly_sixth_vids.push_back(verts_arranged[1]); // B
        poly_sixth_vids.push_back(BH);
        poly_sixth_vids.push_back(AG);
        poly_sixth_vids.push_back(verts_arranged[4]); // E
        poly_sixth_vids.push_back(verts_arranged[5]); // F
        poly_sixth_vids.push_back(FD);
        poly_sixth_vids.push_back(EC);

        poly_mesh.poly_add(poly_first_vids);
        poly_mesh.poly_add(poly_second_vids);
        poly_mesh.poly_add(poly_third_vids);
        poly_mesh.poly_add(poly_fourth_vids);
        poly_mesh.poly_add(poly_fifth_vids);
        poly_mesh.poly_add(poly_sixth_vids);

        break;
    }
    case 6:
    {
        std::cout << "Split along  all faces" << std::endl;

        split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[0]) + lambda * poly_mesh.vert(verts_arranged[6]);
        uint AG = (poly_mesh.vert_add(split_point));
        split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[1]) + lambda * poly_mesh.vert(verts_arranged[7]);
        uint BH = (poly_mesh.vert_add(split_point));
        split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[2]) + lambda * poly_mesh.vert(verts_arranged[4]);
        uint CE = (poly_mesh.vert_add(split_point));
        split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[3]) + lambda * poly_mesh.vert(verts_arranged[5]);
        uint DF = (poly_mesh.vert_add(split_point));

        split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[4]) + lambda * poly_mesh.vert(verts_arranged[2]);
        uint EC = (poly_mesh.vert_add(split_point));
        split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[5]) + lambda * poly_mesh.vert(verts_arranged[3]);
        uint FD = (poly_mesh.vert_add(split_point));
        split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[6]) + lambda * poly_mesh.vert(verts_arranged[0]);
        uint GA = (poly_mesh.vert_add(split_point));
        split_point = (1 - lambda) * poly_mesh.vert(verts_arranged[7]) + lambda * poly_mesh.vert(verts_arranged[1]);
        uint HB = (poly_mesh.vert_add(split_point));

        // AG, BH, CE, DF, EC, FD, GA, HB;
        poly_first_vids.clear();
        poly_first_vids.push_back(AG);
        poly_first_vids.push_back(BH);
        poly_first_vids.push_back(CE);
        poly_first_vids.push_back(DF);
        poly_first_vids.push_back(EC);
        poly_first_vids.push_back(FD);
        poly_first_vids.push_back(GA);
        poly_first_vids.push_back(HB);

        // A, B, C, D, AG, BH, CE, DF;
        poly_second_vids.clear();
        poly_second_vids.push_back(verts_arranged[0]); // A
        poly_second_vids.push_back(verts_arranged[1]); // B
        poly_second_vids.push_back(verts_arranged[2]); // C
        poly_second_vids.push_back(verts_arranged[3]); // D
        poly_second_vids.push_back(AG);
        poly_second_vids.push_back(BH);
        poly_second_vids.push_back(CE);
        poly_second_vids.push_back(DF);

        // EC, FD, GA, HB, E, F, G, H;
        poly_third_vids.clear();
        poly_third_vids.push_back(EC);
        poly_third_vids.push_back(FD);
        poly_third_vids.push_back(GA);
        poly_third_vids.push_back(HB);
        poly_third_vids.push_back(verts_arranged[4]); // E
        poly_third_vids.push_back(verts_arranged[5]); // F
        poly_third_vids.push_back(verts_arranged[6]); // G
        poly_third_vids.push_back(verts_arranged[7]); // H

        // DF, CE, C, D, HB, GA, G, H;
        poly_fourth_vids.clear();
        poly_fourth_vids.push_back(DF);
        poly_fourth_vids.push_back(CE);
        poly_fourth_vids.push_back(verts_arranged[2]); // C
        poly_fourth_vids.push_back(verts_arranged[3]); // D
        poly_fourth_vids.push_back(HB);
        poly_fourth_vids.push_back(GA);
        poly_fourth_vids.push_back(verts_arranged[6]); // G
        poly_fourth_vids.push_back(verts_arranged[7]); // H

        // A, B, BH, AG, E, F, FD, EC;
        poly_fifth_vids.clear();
        poly_fifth_vids.push_back(verts_arranged[0]); // A
        poly_fifth_vids.push_back(verts_arranged[1]); // B
        poly_fifth_vids.push_back(BH);
        poly_fifth_vids.push_back(AG);
        poly_fifth_vids.push_back(verts_arranged[4]); // E
        poly_fifth_vids.push_back(verts_arranged[5]); // F
        poly_fifth_vids.push_back(FD);
        poly_fifth_vids.push_back(EC);

        // B, C, CE, BH, F, G, GA, FD;
        poly_sixth_vids.clear();
        poly_sixth_vids.push_back(verts_arranged[1]); // B
        poly_sixth_vids.push_back(verts_arranged[2]); // C
        poly_sixth_vids.push_back(CE);
        poly_sixth_vids.push_back(BH);
        poly_sixth_vids.push_back(verts_arranged[5]); // F
        poly_sixth_vids.push_back(verts_arranged[6]); // G
        poly_sixth_vids.push_back(GA);
        poly_sixth_vids.push_back(FD);

        // AG, DF, D, A, EC, HB, H, E;
        poly_seventh_vids.clear();
        poly_seventh_vids.push_back(AG);
        poly_seventh_vids.push_back(DF);
        poly_seventh_vids.push_back(verts_arranged[3]); // D
        poly_seventh_vids.push_back(verts_arranged[0]); // A
        poly_seventh_vids.push_back(EC);
        poly_seventh_vids.push_back(HB);
        poly_seventh_vids.push_back(verts_arranged[7]); // H
        poly_seventh_vids.push_back(verts_arranged[4]); // E

        poly_mesh.poly_add(poly_first_vids);
        poly_mesh.poly_add(poly_second_vids);
        poly_mesh.poly_add(poly_third_vids);
        poly_mesh.poly_add(poly_fourth_vids);
        poly_mesh.poly_add(poly_fifth_vids);
        poly_mesh.poly_add(poly_sixth_vids);
        poly_mesh.poly_add(poly_seventh_vids);

        break;
    }
    default:
        std::cout << "No possible split" << std::endl;
        break;
    }

    poly_mesh.poly_remove(0); // rimuovo il poliedro originale

    poly_mesh.update_bbox();
    poly_mesh.update_quality();
    poly_mesh.update_normals();

    poly_mesh.updateGL();

    // stampa il numero di poliedri dopo lo split
    std::cout << "Number of polys after split: " << poly_mesh.num_polys() << std::endl;
    // Stampa i vertici di ogni poliedro
    for (uint pid = 0; pid < poly_mesh.num_polys(); ++pid)
    {
        std::vector<uint> poly_verts = poly_mesh.poly_verts_id(pid);
        std::cout << "Poly " << pid << " verts: ";
        for (uint vid : poly_verts)
        {
            std::cout << vid << " ";
        }
        std::cout << std::endl;
    }

    GLcanvas gui;
    VolumeMeshControls<DrawablePolyhedralmesh<>> menu(&poly_mesh, &gui, "Hex Mesh Controls");

    // gui.push(&poly_mesh);
    gui.push(&poly_mesh);
    // gui.push(&poly_singularity);

    gui.push(&menu);

    gui.launch();
    return 0;
}
