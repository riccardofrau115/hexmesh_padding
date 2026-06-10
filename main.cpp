#include <cinolib/meshes/meshes.h>
#include <cinolib/gl/glcanvas.h>
#include <cinolib/gl/volume_mesh_controls.h>
#include "cinolib/hex_transition_schemes.h"
#include <cinolib/dual_mesh.h>
#include <cinolib/meshes/polyhedralmesh.h>
#include <cinolib/export_cluster.h>
#include <cinolib/hex_shift_indices.h>
#include <cinolib/grid_mesh.h>

#define T_PATTERN Flat
#define STR_(x) #x
#define STR(x) STR_(x)

using namespace cinolib;
const std::vector<uint> faces_of_poly = {0, 1, 2, 3, 4, 5};

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

    default:
        throw std::runtime_error("Invalid Hex Rotation");
        break;
    }
}

// ottieni i vertici di faccia nel cubo "proiettato"
std::unordered_set<uint> vertices_from_face(const std::vector<uint> &vertices, const uint fid)
{
    std::unordered_set<uint> face_verts;
    switch (fid)
    {
    case 0:
        face_verts.insert(vertices[HEXA_FACES[0][0]]);
        face_verts.insert(vertices[HEXA_FACES[0][1]]);
        face_verts.insert(vertices[HEXA_FACES[0][2]]);
        face_verts.insert(vertices[HEXA_FACES[0][3]]);
        break;
    case 1:
        face_verts.insert(vertices[HEXA_FACES[1][0]]);
        face_verts.insert(vertices[HEXA_FACES[1][1]]);
        face_verts.insert(vertices[HEXA_FACES[1][2]]);
        face_verts.insert(vertices[HEXA_FACES[1][3]]);
        break;
    case 2:
        face_verts.insert(vertices[HEXA_FACES[2][0]]);
        face_verts.insert(vertices[HEXA_FACES[2][1]]);
        face_verts.insert(vertices[HEXA_FACES[2][2]]);
        face_verts.insert(vertices[HEXA_FACES[2][3]]);
        break;
    case 3:
        face_verts.insert(vertices[HEXA_FACES[3][0]]);
        face_verts.insert(vertices[HEXA_FACES[3][1]]);
        face_verts.insert(vertices[HEXA_FACES[3][2]]);
        face_verts.insert(vertices[HEXA_FACES[3][3]]);
        break;
    case 4:
        face_verts.insert(vertices[HEXA_FACES[4][0]]);
        face_verts.insert(vertices[HEXA_FACES[4][1]]);
        face_verts.insert(vertices[HEXA_FACES[4][2]]);
        face_verts.insert(vertices[HEXA_FACES[4][3]]);
        break;
    case 5:
        face_verts.insert(vertices[HEXA_FACES[5][0]]);
        face_verts.insert(vertices[HEXA_FACES[5][1]]);
        face_verts.insert(vertices[HEXA_FACES[5][2]]);
        face_verts.insert(vertices[HEXA_FACES[5][3]]);
        break;
    default:
        throw std::runtime_error("Invalid Hex Face");
        // break;
    }
    return face_verts;
}

// ELIMINATA
uint face_from_vertices_local(const std::vector<uint> &original_verts, const std::unordered_set<uint> &face_verts)
{
    std::vector<uint> pattern;
    for (auto vert : face_verts)
    {
        auto it = std::find(original_verts.begin(), original_verts.end(), vert);
        uint index = std::distance(original_verts.begin(), it);
        pattern.push_back(index);
    }
    std::sort(pattern.begin(), pattern.end());

    // confronto il pattern con tutti i pattern standard di faccia
    static std::vector<std::vector<uint>> standard_patterns;
    standard_patterns.push_back({0, 1, 2, 3});
    standard_patterns.push_back({1, 2, 5, 6});
    standard_patterns.push_back({4, 5, 6, 7});
    standard_patterns.push_back({0, 3, 4, 7});
    standard_patterns.push_back({0, 1, 4, 5});
    standard_patterns.push_back({2, 3, 6, 7});

    auto it = std::find(standard_patterns.begin(), standard_patterns.end(), pattern);
    auto index = std::distance(standard_patterns.begin(), it);

    return index;
}

// verifica che le facce da paddare (input og) corrispondano alle facce dello schema statico (projected)
bool check_rotation(const std::vector<std::unordered_set<uint>> &vertices_pjface, const std::vector<std::unordered_set<uint>> &vertices_ogface, std::vector<uint> faces_to_pad, std::vector<uint> face_static)
{
    if (faces_to_pad.size() != face_static.size())
    {
        throw std::runtime_error("Number of faces to pad does not match the number of static faces");
    }

    bool flag = true;
    for (auto fid : face_static)
    {
        uint correspondence_face_id = -1;
        for (int i = 0; i < 6; i++)
        {
            if (vertices_pjface[fid] == vertices_ogface[i])
            {
                correspondence_face_id = i;
                break;
            }
        }

        // faces to pad è della configurazione originale
        bool found = std::find(faces_to_pad.begin(), faces_to_pad.end(), correspondence_face_id) != faces_to_pad.end();
        flag = flag && found;
    }
    return flag;
}

std::vector<uint> arrange_rotation(const std::vector<uint> &verts_og,
                                   const std::vector<uint> &faces_to_pad,
                                   std::vector<uint> &verts_rebase,
                                   std::vector<uint> &verts_twisted,
                                   std::vector<uint> faces_static,
                                   std::vector<std::unordered_set<uint>> &vertices_ogface

)
{
    std::vector<uint> rotation_indeces;
    uint i, j;
    bool rotation = false;
    std::vector<std::unordered_set<uint>> vertices_pjface(6);

    for (i = 0; i < 6; i++)
    {
        // std::cout << "Trying rotation " << i << std::endl;
        hex_rebase(verts_og.data(), i, verts_rebase.data());
        for (j = 0; j < 4; j++)
        {
            hex_around_axis(verts_rebase.data(), j, verts_twisted.data());

            // vertici di faccia con la configurazione proiettata
            for (int i = 0; i < 6; i++)
            {
                vertices_pjface[i] = vertices_from_face(verts_twisted, i);
            }

            rotation = check_rotation(vertices_pjface, vertices_ogface, faces_to_pad, faces_static);
            if (rotation)
                break;
        }

        if (rotation)
            break;
    }

    if (!rotation)
        throw std::runtime_error("No valid rotation found for the given faces to pad and static faces.");

    rotation_indeces.push_back(i);
    rotation_indeces.push_back(j);
    //std::cout << "Rotation found: " << i << " " << j << std::endl;
    return rotation_indeces;
}

bool pad_faces(
    DrawableHexmesh<> &poly_mesh,
    const uint pid,
    const std::vector<uint> faces_to_pad)
{

    double lambda = 1 / 3.0; // posizione di split lungo l'edge

    vec3d split_point;
    std::vector<uint> verts_og = poly_mesh.poly_verts_id(pid);

    std::vector<std::vector<uint>> new_polys_vids;
    std::vector<uint> verts_rebase = verts_og;
    std::vector<uint> verts_twisted = verts_og;
    std::vector<uint> new_single_poly_vids;
    std::vector<uint> new_vids;

    // vertici di faccia con la configurazione iniziale
    std::vector<std::unordered_set<uint>> vertices_ogface(6);
    for (int i = 0; i < 6; i++)
    {
        vertices_ogface[i] = vertices_from_face(verts_og, i);
    }

    // converto faces to pad dal fid di mesh al valore singolo del poliedro
    // per fare questo ottengo i vertici delle facce da paddare e lo ricerco nella configurazione tradotta precedentemente

    // key = fid, value = set di vertici
    std::vector<uint> faces_to_pad_converted(faces_to_pad.size());
    for (uint i = 0; i < faces_to_pad.size(); i++)
    {
        uint fid = faces_to_pad[i];
        std::unordered_set<uint> face_verts;
        auto f = poly_mesh.face_verts_id(fid);
        face_verts.insert(f.begin(), f.end());

        for (uint j = 0; j < 6; j++) //  verifico a quale faccia corrisponda la singola faccia da paddare nella configurazione tradotta
        {
            if (vertices_ogface[j] == face_verts)
            {
                faces_to_pad_converted[i] = j;
                break;
            }
        }
        
    }

    bool padding_flag = true;
    switch (faces_to_pad_converted.size())
    {
    case 1:
    {
        //std::cout << "Split along face " << faces_to_pad_converted[0] << std::endl;
        arrange_rotation(verts_og, faces_to_pad_converted, verts_rebase, verts_twisted, {0}, vertices_ogface);
        // padding della faccia 0

        split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[0]) + lambda * poly_mesh.vert(verts_twisted[4]);
        uint AE = (poly_mesh.vert_add(split_point));
        split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[1]) + lambda * poly_mesh.vert(verts_twisted[5]);
        uint BF = (poly_mesh.vert_add(split_point));
        split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[2]) + lambda * poly_mesh.vert(verts_twisted[6]);
        uint CG = (poly_mesh.vert_add(split_point));
        split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[3]) + lambda * poly_mesh.vert(verts_twisted[7]);
        uint DH = (poly_mesh.vert_add(split_point));

        new_polys_vids.clear();
        new_single_poly_vids.clear();

        // A B C D AE BF CG DH
        new_single_poly_vids.push_back(verts_twisted[0]); // A
        new_single_poly_vids.push_back(verts_twisted[1]); // B
        new_single_poly_vids.push_back(verts_twisted[2]); // C
        new_single_poly_vids.push_back(verts_twisted[3]); // D
        new_single_poly_vids.push_back(AE);
        new_single_poly_vids.push_back(BF);
        new_single_poly_vids.push_back(CG);
        new_single_poly_vids.push_back(DH);

        new_polys_vids.push_back(new_single_poly_vids);
        new_single_poly_vids.clear();

        // AE BF CG DH E F G H
        new_single_poly_vids.push_back(AE);
        new_single_poly_vids.push_back(BF);
        new_single_poly_vids.push_back(CG);
        new_single_poly_vids.push_back(DH);
        new_single_poly_vids.push_back(verts_twisted[4]); // E
        new_single_poly_vids.push_back(verts_twisted[5]); // F
        new_single_poly_vids.push_back(verts_twisted[6]); // G
        new_single_poly_vids.push_back(verts_twisted[7]); // H

        new_polys_vids.push_back(new_single_poly_vids);

        break;
    }
    case 2:
    {

        //std::cout << "Split along faces " << faces_to_pad_converted[0] << " and " << faces_to_pad_converted[1] << std::endl;
        /// caso in cui le facce siano opposte
        if (!poly_mesh.faces_are_adjacent(faces_to_pad_converted[0], faces_to_pad_converted[1]))
        {
            // padding delle facce 1 e 3
            std::vector<uint> rotation_found = arrange_rotation(verts_og, faces_to_pad_converted, verts_rebase, verts_twisted, {3, 1}, vertices_ogface);

            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[0]) + lambda * poly_mesh.vert(verts_twisted[1]);
            uint AB = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[1]) + lambda * poly_mesh.vert(verts_twisted[0]);
            uint BA = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[2]) + lambda * poly_mesh.vert(verts_twisted[3]);
            uint CD = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[3]) + lambda * poly_mesh.vert(verts_twisted[2]);
            uint DC = (poly_mesh.vert_add(split_point));

            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[4]) + lambda * poly_mesh.vert(verts_twisted[5]);
            uint EF = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[5]) + lambda * poly_mesh.vert(verts_twisted[4]);
            uint FE = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[6]) + lambda * poly_mesh.vert(verts_twisted[7]);
            uint GH = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[7]) + lambda * poly_mesh.vert(verts_twisted[6]);
            uint HG = (poly_mesh.vert_add(split_point));

            new_polys_vids.clear();
            new_single_poly_vids.clear();

            // AB, BA, CD, DC, EF, FE, GH, HG;
            new_single_poly_vids.clear();
            new_single_poly_vids.push_back(AB);
            new_single_poly_vids.push_back(BA);
            new_single_poly_vids.push_back(CD);
            new_single_poly_vids.push_back(DC);
            new_single_poly_vids.push_back(EF);
            new_single_poly_vids.push_back(FE);
            new_single_poly_vids.push_back(GH);
            new_single_poly_vids.push_back(HG);
            new_polys_vids.push_back(new_single_poly_vids);

            // A, AB, DC, D, E, EF, HG, H;
            new_single_poly_vids.clear();
            new_single_poly_vids.push_back(verts_twisted[0]); // A
            new_single_poly_vids.push_back(AB);
            new_single_poly_vids.push_back(DC);
            new_single_poly_vids.push_back(verts_twisted[3]); // D
            new_single_poly_vids.push_back(verts_twisted[4]); // E
            new_single_poly_vids.push_back(EF);
            new_single_poly_vids.push_back(HG);
            new_single_poly_vids.push_back(verts_twisted[7]); // H
            new_polys_vids.push_back(new_single_poly_vids);

            // BA, B, C, CD, FE, F, G, GH
            new_single_poly_vids.clear();
            new_single_poly_vids.push_back(BA);
            new_single_poly_vids.push_back(verts_twisted[1]); // B
            new_single_poly_vids.push_back(verts_twisted[2]); // C
            new_single_poly_vids.push_back(CD);
            new_single_poly_vids.push_back(FE);
            new_single_poly_vids.push_back(verts_twisted[5]); // F
            new_single_poly_vids.push_back(verts_twisted[6]); // G
            new_single_poly_vids.push_back(GH);
            new_polys_vids.push_back(new_single_poly_vids);
        }
        else
        {
            // caso in cui le facce siano adiacenti

            // padding delle facce 3 e 5
            std::vector<uint> rotation_found = arrange_rotation(verts_og, faces_to_pad_converted, verts_rebase, verts_twisted, {3, 5}, vertices_ogface);

            // bottom face
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[0]) + lambda * poly_mesh.vert(verts_twisted[1]);
            uint AB = poly_mesh.vert_add(split_point);
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[2]) + lambda * poly_mesh.vert(verts_twisted[1]);
            uint CB = poly_mesh.vert_add(split_point);
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[3]) + lambda * poly_mesh.vert(verts_twisted[1]);
            uint DB = poly_mesh.vert_add(split_point);
            // top face
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[4]) + lambda * poly_mesh.vert(verts_twisted[5]);
            uint EF = poly_mesh.vert_add(split_point);
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[6]) + lambda * poly_mesh.vert(verts_twisted[5]);
            uint GF = poly_mesh.vert_add(split_point);
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[7]) + lambda * poly_mesh.vert(verts_twisted[5]);
            uint HF = poly_mesh.vert_add(split_point);

            new_polys_vids.clear();

            new_single_poly_vids.clear();
            new_single_poly_vids.push_back(AB);
            new_single_poly_vids.push_back(verts_twisted[1]); // B
            new_single_poly_vids.push_back(CB);
            new_single_poly_vids.push_back(DB);
            new_single_poly_vids.push_back(EF);
            new_single_poly_vids.push_back(verts_twisted[5]); // F
            new_single_poly_vids.push_back(GF);
            new_single_poly_vids.push_back(HF);
            new_polys_vids.push_back(new_single_poly_vids);

            new_single_poly_vids.clear();
            new_single_poly_vids.push_back(verts_twisted[0]); // A
            new_single_poly_vids.push_back(AB);
            new_single_poly_vids.push_back(DB);
            new_single_poly_vids.push_back(verts_twisted[3]); // D
            new_single_poly_vids.push_back(verts_twisted[4]); // E
            new_single_poly_vids.push_back(EF);
            new_single_poly_vids.push_back(HF);
            new_single_poly_vids.push_back(verts_twisted[7]); // H
            new_polys_vids.push_back(new_single_poly_vids);

            new_single_poly_vids.clear();
            new_single_poly_vids.push_back(verts_twisted[3]); // D
            new_single_poly_vids.push_back(DB);
            new_single_poly_vids.push_back(CB);
            new_single_poly_vids.push_back(verts_twisted[2]); // C
            new_single_poly_vids.push_back(verts_twisted[7]); // H
            new_single_poly_vids.push_back(HF);
            new_single_poly_vids.push_back(GF);
            new_single_poly_vids.push_back(verts_twisted[6]); // G
            new_polys_vids.push_back(new_single_poly_vids);
        }
        break;
    }
    case 3:
    {
        //std::cout << "Split along faces " << faces_to_pad_converted[0] << ", " << faces_to_pad_converted[1] << " and " << faces_to_pad_converted[2] << std::endl;
        // facce ad angolo, tutte le facce sono adiacenti tra loro

        bool corner =
            (poly_mesh.faces_are_adjacent(faces_to_pad_converted[0], faces_to_pad_converted[1]) &&
             poly_mesh.faces_are_adjacent(faces_to_pad_converted[1], faces_to_pad_converted[2]) &&
             poly_mesh.faces_are_adjacent(faces_to_pad_converted[2], faces_to_pad_converted[0]));

        if (corner)
        {
            // padding delle facce 0,1,4
            arrange_rotation(verts_og, faces_to_pad_converted, verts_rebase, verts_twisted, {0, 1, 4}, vertices_ogface);

            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[0]) + lambda * poly_mesh.vert(verts_twisted[3]);
            uint AD = poly_mesh.vert_add(split_point);
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[1]) + lambda * poly_mesh.vert(verts_twisted[3]);
            uint BD = poly_mesh.vert_add(split_point);
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[2]) + lambda * poly_mesh.vert(verts_twisted[3]);
            uint CD = poly_mesh.vert_add(split_point);

            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[4]) + lambda * poly_mesh.vert(verts_twisted[7]);
            uint EH = poly_mesh.vert_add(split_point);
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[5]) + lambda * poly_mesh.vert(verts_twisted[7]);
            uint FH = poly_mesh.vert_add(split_point);
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[6]) + lambda * poly_mesh.vert(verts_twisted[7]);
            uint GH = poly_mesh.vert_add(split_point);

            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[0]) + lambda * poly_mesh.vert(verts_twisted[4]);
            uint AE = poly_mesh.vert_add(split_point);
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[1]) + lambda * poly_mesh.vert(verts_twisted[5]);
            uint BF = poly_mesh.vert_add(split_point);
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[2]) + lambda * poly_mesh.vert(verts_twisted[6]);
            uint CG = poly_mesh.vert_add(split_point);
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[3]) + lambda * poly_mesh.vert(verts_twisted[7]);
            uint DH = poly_mesh.vert_add(split_point);

            split_point = (1 - lambda) * poly_mesh.vert(AD) + lambda * poly_mesh.vert(EH);
            uint ADEH = poly_mesh.vert_add(split_point);
            split_point = (1 - lambda) * poly_mesh.vert(BD) + lambda * poly_mesh.vert(FH);
            uint BDFH = poly_mesh.vert_add(split_point);
            split_point = (1 - lambda) * poly_mesh.vert(CD) + lambda * poly_mesh.vert(GH);
            uint CDGH = poly_mesh.vert_add(split_point);

            new_polys_vids.clear();

            // DH ADEH BDFH CDGH H EH FH GH
            new_single_poly_vids.clear();
            new_single_poly_vids.push_back(DH);
            new_single_poly_vids.push_back(ADEH);
            new_single_poly_vids.push_back(BDFH);
            new_single_poly_vids.push_back(CDGH);
            new_single_poly_vids.push_back(verts_twisted[7]); // H
            new_single_poly_vids.push_back(EH);
            new_single_poly_vids.push_back(FH);
            new_single_poly_vids.push_back(GH);
            new_polys_vids.push_back(new_single_poly_vids);

            //  D AD BD CD DH ADEH BDFH CDGH
            new_single_poly_vids.clear();
            new_single_poly_vids.push_back(verts_twisted[3]); // D
            new_single_poly_vids.push_back(AD);
            new_single_poly_vids.push_back(BD);
            new_single_poly_vids.push_back(CD);
            new_single_poly_vids.push_back(DH);
            new_single_poly_vids.push_back(ADEH);
            new_single_poly_vids.push_back(BDFH);
            new_single_poly_vids.push_back(CDGH);
            new_polys_vids.push_back(new_single_poly_vids);

            // C B BD CD CG BF BDFH CDGH
            new_single_poly_vids.clear();
            new_single_poly_vids.push_back(verts_twisted[2]); // C
            new_single_poly_vids.push_back(verts_twisted[1]); // B
            new_single_poly_vids.push_back(BD);
            new_single_poly_vids.push_back(CD);
            new_single_poly_vids.push_back(CG);
            new_single_poly_vids.push_back(BF);
            new_single_poly_vids.push_back(BDFH);
            new_single_poly_vids.push_back(CDGH);
            new_polys_vids.push_back(new_single_poly_vids);

            // B, A, AD, BD, BF, AE, ADEH, BDFH;
            new_single_poly_vids.clear();
            new_single_poly_vids.push_back(verts_twisted[1]); // B
            new_single_poly_vids.push_back(verts_twisted[0]); // A
            new_single_poly_vids.push_back(AD);
            new_single_poly_vids.push_back(BD);
            new_single_poly_vids.push_back(BF);
            new_single_poly_vids.push_back(AE);
            new_single_poly_vids.push_back(ADEH);
            new_single_poly_vids.push_back(BDFH);
            new_polys_vids.push_back(new_single_poly_vids);

            // BF, BDFH, CDGH, CG, F, FH, GH, G;

            new_single_poly_vids.clear();
            new_single_poly_vids.push_back(BF);
            new_single_poly_vids.push_back(BDFH);
            new_single_poly_vids.push_back(CDGH);
            new_single_poly_vids.push_back(CG);
            new_single_poly_vids.push_back(verts_twisted[5]); // F
            new_single_poly_vids.push_back(FH);
            new_single_poly_vids.push_back(GH);
            new_single_poly_vids.push_back(verts_twisted[6]); // G
            new_polys_vids.push_back(new_single_poly_vids);

            // BF, AE, ADEH, BDFH, F, E, EH, FH;
            new_single_poly_vids.clear();
            new_single_poly_vids.push_back(BF);
            new_single_poly_vids.push_back(AE);
            new_single_poly_vids.push_back(ADEH);
            new_single_poly_vids.push_back(BDFH);
            new_single_poly_vids.push_back(verts_twisted[5]); // F
            new_single_poly_vids.push_back(verts_twisted[4]); // E
            new_single_poly_vids.push_back(EH);
            new_single_poly_vids.push_back(FH);
            new_polys_vids.push_back(new_single_poly_vids);
        }
        else // facce a forma di U
        {
            // padding delle facce 5, 1, 3
            arrange_rotation(verts_og, faces_to_pad_converted, verts_rebase, verts_twisted, {5, 1, 3}, vertices_ogface);

            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[0]) + lambda * poly_mesh.vert(verts_twisted[1]);
            uint AB = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[1]) + lambda * poly_mesh.vert(verts_twisted[0]);
            uint BA = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[3]) + lambda * poly_mesh.vert(verts_twisted[1]);
            uint DB = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[2]) + lambda * poly_mesh.vert(verts_twisted[0]);
            uint CA = (poly_mesh.vert_add(split_point));

            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[4]) + lambda * poly_mesh.vert(verts_twisted[5]);
            uint EF = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[5]) + lambda * poly_mesh.vert(verts_twisted[4]);
            uint FE = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[6]) + lambda * poly_mesh.vert(verts_twisted[4]);
            uint GE = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[7]) + lambda * poly_mesh.vert(verts_twisted[5]);
            uint HF = (poly_mesh.vert_add(split_point));

            new_polys_vids.clear();

            // AB, BA, CA, DB, EF, FE, GE ,HF
            new_single_poly_vids.clear();
            new_single_poly_vids.push_back(AB);
            new_single_poly_vids.push_back(BA);
            new_single_poly_vids.push_back(CA);
            new_single_poly_vids.push_back(DB);
            new_single_poly_vids.push_back(EF);
            new_single_poly_vids.push_back(FE);
            new_single_poly_vids.push_back(GE);
            new_single_poly_vids.push_back(HF);
            new_polys_vids.push_back(new_single_poly_vids);

            // A, AB, DB, D, E, EF, HF, H;
            new_single_poly_vids.clear();
            new_single_poly_vids.push_back(verts_twisted[0]); // A
            new_single_poly_vids.push_back(AB);
            new_single_poly_vids.push_back(DB);
            new_single_poly_vids.push_back(verts_twisted[3]); // D
            new_single_poly_vids.push_back(verts_twisted[4]); // E
            new_single_poly_vids.push_back(EF);
            new_single_poly_vids.push_back(HF);
            new_single_poly_vids.push_back(verts_twisted[7]); // H
            new_polys_vids.push_back(new_single_poly_vids);

            // D, DB, CA, C, H, HF, GE, G;
            new_single_poly_vids.clear();
            new_single_poly_vids.push_back(verts_twisted[3]); // D
            new_single_poly_vids.push_back(DB);
            new_single_poly_vids.push_back(CA);
            new_single_poly_vids.push_back(verts_twisted[2]); // C
            new_single_poly_vids.push_back(verts_twisted[7]); // H
            new_single_poly_vids.push_back(HF);
            new_single_poly_vids.push_back(GE);
            new_single_poly_vids.push_back(verts_twisted[6]); // G
            new_polys_vids.push_back(new_single_poly_vids);

            // B, C, CA, BA, F, G, GE, FE
            new_single_poly_vids.clear();
            new_single_poly_vids.push_back(verts_twisted[1]); // B
            new_single_poly_vids.push_back(verts_twisted[2]); // C
            new_single_poly_vids.push_back(CA);
            new_single_poly_vids.push_back(BA);
            new_single_poly_vids.push_back(verts_twisted[5]); // F
            new_single_poly_vids.push_back(verts_twisted[6]); // G
            new_single_poly_vids.push_back(GE);
            new_single_poly_vids.push_back(FE);
            new_polys_vids.push_back(new_single_poly_vids);
        }

        break;
    }
    case 4:
    {
        //std::cout << "Split along faces " << faces_to_pad_converted[0] << ", " << faces_to_pad_converted[1] << ", " << faces_to_pad_converted[2] << " and " << faces_to_pad_converted[3] << std::endl;
        std::vector<uint> faces_not_to_pad;

        // verifica che le facce escluse dal padding siano adiacenti
        for (auto face : faces_of_poly)
        {
            if (std::find(faces_to_pad_converted.begin(), faces_to_pad_converted.end(), face) == faces_to_pad_converted.end())
            {
                faces_not_to_pad.push_back(face);
            }
        }

        if (poly_mesh.faces_are_adjacent(faces_not_to_pad[0], faces_not_to_pad[1]))
        {
            // caso in cui le facce da escludere siano adiacenti tra loro
            // padding delle facce 0, 1, 2, 4
            arrange_rotation(verts_og, faces_to_pad_converted, verts_rebase, verts_twisted, {0, 1, 2, 4}, vertices_ogface);

            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[0]) + lambda * poly_mesh.vert(verts_twisted[7]);
            uint AH = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[1]) + lambda * poly_mesh.vert(verts_twisted[7]);
            uint BH = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[2]) + lambda * poly_mesh.vert(verts_twisted[7]);
            uint CH = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[3]) + lambda * poly_mesh.vert(verts_twisted[7]);
            uint DH = (poly_mesh.vert_add(split_point));

            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[4]) + lambda * poly_mesh.vert(verts_twisted[3]);
            uint ED = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[5]) + lambda * poly_mesh.vert(verts_twisted[3]);
            uint FD = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[6]) + lambda * poly_mesh.vert(verts_twisted[3]);
            uint GD = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[7]) + lambda * poly_mesh.vert(verts_twisted[3]);
            uint HD = (poly_mesh.vert_add(split_point));

            new_polys_vids.clear();

            // AH, BH, CH, DH, ED, FD, GD, HD
            new_single_poly_vids.clear();
            new_single_poly_vids.push_back(AH);
            new_single_poly_vids.push_back(BH);
            new_single_poly_vids.push_back(CH);
            new_single_poly_vids.push_back(DH);
            new_single_poly_vids.push_back(ED);
            new_single_poly_vids.push_back(FD);
            new_single_poly_vids.push_back(GD);
            new_single_poly_vids.push_back(HD);
            new_polys_vids.push_back(new_single_poly_vids);

            // A, B, C, D, AH, BH, CH, DH;
            new_single_poly_vids.clear();
            new_single_poly_vids.push_back(verts_twisted[0]); // A
            new_single_poly_vids.push_back(verts_twisted[1]); // B
            new_single_poly_vids.push_back(verts_twisted[2]); // C
            new_single_poly_vids.push_back(verts_twisted[3]); // D
            new_single_poly_vids.push_back(AH);
            new_single_poly_vids.push_back(BH);
            new_single_poly_vids.push_back(CH);
            new_single_poly_vids.push_back(DH);
            new_polys_vids.push_back(new_single_poly_vids);

            // ED, FD, GD, HD, E, F, G, H;
            new_single_poly_vids.clear();
            new_single_poly_vids.push_back(ED);
            new_single_poly_vids.push_back(FD);
            new_single_poly_vids.push_back(GD);
            new_single_poly_vids.push_back(HD);
            new_single_poly_vids.push_back(verts_twisted[4]); // E
            new_single_poly_vids.push_back(verts_twisted[5]); // F
            new_single_poly_vids.push_back(verts_twisted[6]); // G
            new_single_poly_vids.push_back(verts_twisted[7]); // H
            new_polys_vids.push_back(new_single_poly_vids);

            // B, C, CH, BH, F, G, GD, FD;

            new_single_poly_vids.clear();
            new_single_poly_vids.push_back(verts_twisted[1]); // B
            new_single_poly_vids.push_back(verts_twisted[2]); // C
            new_single_poly_vids.push_back(CH);
            new_single_poly_vids.push_back(BH);
            new_single_poly_vids.push_back(verts_twisted[5]); // F
            new_single_poly_vids.push_back(verts_twisted[6]); // G
            new_single_poly_vids.push_back(GD);
            new_single_poly_vids.push_back(FD);
            new_polys_vids.push_back(new_single_poly_vids);

            // A, B, BH, AH, E, F, FD, ED

            new_single_poly_vids.clear();
            new_single_poly_vids.push_back(verts_twisted[0]); // A
            new_single_poly_vids.push_back(verts_twisted[1]); // B
            new_single_poly_vids.push_back(BH);
            new_single_poly_vids.push_back(AH);
            new_single_poly_vids.push_back(verts_twisted[4]); // E
            new_single_poly_vids.push_back(verts_twisted[5]); // F
            new_single_poly_vids.push_back(FD);
            new_single_poly_vids.push_back(ED);
            new_polys_vids.push_back(new_single_poly_vids);
        }
        else
        {
            // caso in cui le facce da escludere siano opposte
            // padding delle facce 1, 3, 4 e 5
            arrange_rotation(verts_og, faces_to_pad_converted, verts_rebase, verts_twisted, {1, 3, 4, 5}, vertices_ogface);

            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[0]) + lambda * poly_mesh.vert(verts_twisted[2]);
            uint AC = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[1]) + lambda * poly_mesh.vert(verts_twisted[3]);
            uint BD = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[2]) + lambda * poly_mesh.vert(verts_twisted[0]);
            uint CA = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[3]) + lambda * poly_mesh.vert(verts_twisted[1]);
            uint DB = (poly_mesh.vert_add(split_point));

            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[4]) + lambda * poly_mesh.vert(verts_twisted[6]);
            uint EG = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[5]) + lambda * poly_mesh.vert(verts_twisted[7]);
            uint FH = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[6]) + lambda * poly_mesh.vert(verts_twisted[4]);
            uint GE = (poly_mesh.vert_add(split_point));
            split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[7]) + lambda * poly_mesh.vert(verts_twisted[5]);
            uint HF = (poly_mesh.vert_add(split_point));

            new_polys_vids.clear();

            // AC, BD, CA, DB, EG, FH, GF, HF
            new_single_poly_vids.clear();
            new_single_poly_vids.push_back(AC);
            new_single_poly_vids.push_back(BD);
            new_single_poly_vids.push_back(CA);
            new_single_poly_vids.push_back(DB);
            new_single_poly_vids.push_back(EG);
            new_single_poly_vids.push_back(FH);
            new_single_poly_vids.push_back(GE);
            new_single_poly_vids.push_back(HF);
            new_polys_vids.push_back(new_single_poly_vids);

            // A, D, DB, AC, E, H, HF, EG
            new_single_poly_vids.clear();
            new_single_poly_vids.push_back(verts_twisted[0]); // A
            new_single_poly_vids.push_back(verts_twisted[3]); // D
            new_single_poly_vids.push_back(DB);
            new_single_poly_vids.push_back(AC);
            new_single_poly_vids.push_back(verts_twisted[4]); // E
            new_single_poly_vids.push_back(verts_twisted[7]); // H
            new_single_poly_vids.push_back(HF);
            new_single_poly_vids.push_back(EG);
            new_polys_vids.push_back(new_single_poly_vids);

            // DB, CA, C, D, HF, GE, G, H
            new_single_poly_vids.clear();
            new_single_poly_vids.push_back(DB);
            new_single_poly_vids.push_back(CA);
            new_single_poly_vids.push_back(verts_twisted[2]); // C
            new_single_poly_vids.push_back(verts_twisted[3]); // D
            new_single_poly_vids.push_back(HF);
            new_single_poly_vids.push_back(GE);
            new_single_poly_vids.push_back(verts_twisted[6]); // G
            new_single_poly_vids.push_back(verts_twisted[7]); // H
            new_polys_vids.push_back(new_single_poly_vids);

            // B, C, CA, BD, F, G, GE, FH
            new_single_poly_vids.clear();
            new_single_poly_vids.push_back(verts_twisted[1]); // B
            new_single_poly_vids.push_back(verts_twisted[2]); // C
            new_single_poly_vids.push_back(CA);
            new_single_poly_vids.push_back(BD);
            new_single_poly_vids.push_back(verts_twisted[5]); // F
            new_single_poly_vids.push_back(verts_twisted[6]); // G
            new_single_poly_vids.push_back(GE);
            new_single_poly_vids.push_back(FH);
            new_polys_vids.push_back(new_single_poly_vids);

            // B, A, AC, BD, F, E, EG, FH
            new_single_poly_vids.clear();
            new_single_poly_vids.push_back(verts_twisted[1]); // B
            new_single_poly_vids.push_back(verts_twisted[0]); // A
            new_single_poly_vids.push_back(AC);
            new_single_poly_vids.push_back(BD);
            new_single_poly_vids.push_back(verts_twisted[5]); // F
            new_single_poly_vids.push_back(verts_twisted[4]); // E
            new_single_poly_vids.push_back(EG);
            new_single_poly_vids.push_back(FH);
            new_polys_vids.push_back(new_single_poly_vids);
        }

        break;
    }
    case 5:
    {
        //std::cout << "Split along faces " << faces_to_pad_converted[0] << ", " << faces_to_pad_converted[1] << ", " << faces_to_pad_converted[2] << ", " << faces_to_pad_converted[3] << " and " << faces_to_pad_converted[4] << std::endl;

        // padding delle facce 0, 1, 2, 3, 4
        arrange_rotation(verts_og, faces_to_pad_converted, verts_rebase, verts_twisted, {0, 1, 2, 3, 4}, vertices_ogface);

        split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[0]) + lambda * poly_mesh.vert(verts_twisted[6]);
        uint AG = (poly_mesh.vert_add(split_point));
        split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[1]) + lambda * poly_mesh.vert(verts_twisted[7]);
        uint BH = (poly_mesh.vert_add(split_point));
        split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[2]) + lambda * poly_mesh.vert(verts_twisted[7]);
        uint CH = (poly_mesh.vert_add(split_point));
        split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[3]) + lambda * poly_mesh.vert(verts_twisted[6]);
        uint DG = (poly_mesh.vert_add(split_point));

        split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[4]) + lambda * poly_mesh.vert(verts_twisted[2]);
        uint EC = (poly_mesh.vert_add(split_point));
        split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[5]) + lambda * poly_mesh.vert(verts_twisted[3]);
        uint FD = (poly_mesh.vert_add(split_point));
        split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[6]) + lambda * poly_mesh.vert(verts_twisted[3]);
        uint GD = (poly_mesh.vert_add(split_point));
        split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[7]) + lambda * poly_mesh.vert(verts_twisted[2]);
        uint HC = (poly_mesh.vert_add(split_point));

        new_polys_vids.clear();

        // AG, BH, CH DG, EC, FD, GD, HC
        new_single_poly_vids.clear();
        new_single_poly_vids.push_back(AG);
        new_single_poly_vids.push_back(BH);
        new_single_poly_vids.push_back(CH);
        new_single_poly_vids.push_back(DG);
        new_single_poly_vids.push_back(EC);
        new_single_poly_vids.push_back(FD);
        new_single_poly_vids.push_back(GD);
        new_single_poly_vids.push_back(HC);
        new_polys_vids.push_back(new_single_poly_vids);

        // A, B, C, D, AG, BH, CH, DG;
        new_single_poly_vids.clear();
        new_single_poly_vids.push_back(verts_twisted[0]); // A
        new_single_poly_vids.push_back(verts_twisted[1]); // B
        new_single_poly_vids.push_back(verts_twisted[2]); // C
        new_single_poly_vids.push_back(verts_twisted[3]); // D
        new_single_poly_vids.push_back(AG);
        new_single_poly_vids.push_back(BH);
        new_single_poly_vids.push_back(CH);
        new_single_poly_vids.push_back(DG);
        new_polys_vids.push_back(new_single_poly_vids);

        // EC, FD, GD, HC, G, H, E, F;
        new_single_poly_vids.clear();
        new_single_poly_vids.push_back(EC);
        new_single_poly_vids.push_back(FD);
        new_single_poly_vids.push_back(GD);
        new_single_poly_vids.push_back(HC);
        new_single_poly_vids.push_back(verts_twisted[4]); // E
        new_single_poly_vids.push_back(verts_twisted[5]); // F
        new_single_poly_vids.push_back(verts_twisted[6]); // G
        new_single_poly_vids.push_back(verts_twisted[7]); // H
        new_polys_vids.push_back(new_single_poly_vids);

        // A, D, DG, AG, E, H, HC, EC;
        new_single_poly_vids.clear();
        new_single_poly_vids.push_back(verts_twisted[0]); // A
        new_single_poly_vids.push_back(verts_twisted[3]); // D
        new_single_poly_vids.push_back(DG);
        new_single_poly_vids.push_back(AG);
        new_single_poly_vids.push_back(verts_twisted[4]); // E
        new_single_poly_vids.push_back(verts_twisted[7]); // H
        new_single_poly_vids.push_back(HC);
        new_single_poly_vids.push_back(EC);
        new_polys_vids.push_back(new_single_poly_vids);

        // B, C, CH, BH, F, G, GD, FD;

        new_single_poly_vids.clear();
        new_single_poly_vids.push_back(verts_twisted[1]); // B
        new_single_poly_vids.push_back(verts_twisted[2]); // C
        new_single_poly_vids.push_back(CH);
        new_single_poly_vids.push_back(BH);
        new_single_poly_vids.push_back(verts_twisted[5]); // F
        new_single_poly_vids.push_back(verts_twisted[6]); // G
        new_single_poly_vids.push_back(GD);
        new_single_poly_vids.push_back(FD);
        new_polys_vids.push_back(new_single_poly_vids);

        // A, B, BH, AG, E, F, FD, EC;
        new_single_poly_vids.clear();
        new_single_poly_vids.push_back(verts_twisted[0]); // A
        new_single_poly_vids.push_back(verts_twisted[1]); // B
        new_single_poly_vids.push_back(BH);
        new_single_poly_vids.push_back(AG);
        new_single_poly_vids.push_back(verts_twisted[4]); // E
        new_single_poly_vids.push_back(verts_twisted[5]); // F
        new_single_poly_vids.push_back(FD);
        new_single_poly_vids.push_back(EC);
        new_polys_vids.push_back(new_single_poly_vids);

        break;
    }
    case 6:
    {
        //std::cout << "Split along  all faces" << std::endl;
        // padding di tutte le facce, nessuna rotazione necessaria

        split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[0]) + lambda * poly_mesh.vert(verts_twisted[6]);
        uint AG = (poly_mesh.vert_add(split_point));
        split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[1]) + lambda * poly_mesh.vert(verts_twisted[7]);
        uint BH = (poly_mesh.vert_add(split_point));
        split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[2]) + lambda * poly_mesh.vert(verts_twisted[4]);
        uint CE = (poly_mesh.vert_add(split_point));
        split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[3]) + lambda * poly_mesh.vert(verts_twisted[5]);
        uint DF = (poly_mesh.vert_add(split_point));

        split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[4]) + lambda * poly_mesh.vert(verts_twisted[2]);
        uint EC = (poly_mesh.vert_add(split_point));
        split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[5]) + lambda * poly_mesh.vert(verts_twisted[3]);
        uint FD = (poly_mesh.vert_add(split_point));
        split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[6]) + lambda * poly_mesh.vert(verts_twisted[0]);
        uint GA = (poly_mesh.vert_add(split_point));
        split_point = (1 - lambda) * poly_mesh.vert(verts_twisted[7]) + lambda * poly_mesh.vert(verts_twisted[1]);
        uint HB = (poly_mesh.vert_add(split_point));

        new_polys_vids.clear();

        // AG, BH, CE, DF, EC, FD, GA, HB;
        new_single_poly_vids.clear();
        new_single_poly_vids.push_back(AG);
        new_single_poly_vids.push_back(BH);
        new_single_poly_vids.push_back(CE);
        new_single_poly_vids.push_back(DF);
        new_single_poly_vids.push_back(EC);
        new_single_poly_vids.push_back(FD);
        new_single_poly_vids.push_back(GA);
        new_single_poly_vids.push_back(HB);

        // A, B, C, D, AG, BH, CE, DF;
        new_single_poly_vids.clear();
        new_single_poly_vids.push_back(verts_twisted[0]); // A
        new_single_poly_vids.push_back(verts_twisted[1]); // B
        new_single_poly_vids.push_back(verts_twisted[2]); // C
        new_single_poly_vids.push_back(verts_twisted[3]); // D
        new_single_poly_vids.push_back(AG);
        new_single_poly_vids.push_back(BH);
        new_single_poly_vids.push_back(CE);
        new_single_poly_vids.push_back(DF);
        new_polys_vids.push_back(new_single_poly_vids);

        // EC, FD, GA, HB, E, F, G, H;
        new_single_poly_vids.clear();
        new_single_poly_vids.push_back(EC);
        new_single_poly_vids.push_back(FD);
        new_single_poly_vids.push_back(GA);
        new_single_poly_vids.push_back(HB);
        new_single_poly_vids.push_back(verts_twisted[4]); // E
        new_single_poly_vids.push_back(verts_twisted[5]); // F
        new_single_poly_vids.push_back(verts_twisted[6]); // G
        new_single_poly_vids.push_back(verts_twisted[7]); // H
        new_polys_vids.push_back(new_single_poly_vids);

        // DF, CE, C, D, HB, GA, G, H;
        new_single_poly_vids.clear();
        new_single_poly_vids.push_back(DF);
        new_single_poly_vids.push_back(CE);
        new_single_poly_vids.push_back(verts_twisted[2]); // C
        new_single_poly_vids.push_back(verts_twisted[3]); // D
        new_single_poly_vids.push_back(HB);
        new_single_poly_vids.push_back(GA);
        new_single_poly_vids.push_back(verts_twisted[6]); // G
        new_single_poly_vids.push_back(verts_twisted[7]); // H
        new_polys_vids.push_back(new_single_poly_vids);

        // A, B, BH, AG, E, F, FD, EC;
        new_single_poly_vids.clear();
        new_single_poly_vids.push_back(verts_twisted[0]); // A
        new_single_poly_vids.push_back(verts_twisted[1]); // B
        new_single_poly_vids.push_back(BH);
        new_single_poly_vids.push_back(AG);
        new_single_poly_vids.push_back(verts_twisted[4]); // E
        new_single_poly_vids.push_back(verts_twisted[5]); // F
        new_single_poly_vids.push_back(FD);
        new_single_poly_vids.push_back(EC);
        new_polys_vids.push_back(new_single_poly_vids);

        // B, C, CE, BH, F, G, GA, FD;
        new_single_poly_vids.clear();
        new_single_poly_vids.push_back(verts_twisted[1]); // B
        new_single_poly_vids.push_back(verts_twisted[2]); // C
        new_single_poly_vids.push_back(CE);
        new_single_poly_vids.push_back(BH);
        new_single_poly_vids.push_back(verts_twisted[5]); // F
        new_single_poly_vids.push_back(verts_twisted[6]); // G
        new_single_poly_vids.push_back(GA);
        new_single_poly_vids.push_back(FD);
        new_polys_vids.push_back(new_single_poly_vids);

        // AG, DF, D, A, EC, HB, H, E;
        new_single_poly_vids.clear();
        new_single_poly_vids.push_back(AG);
        new_single_poly_vids.push_back(DF);
        new_single_poly_vids.push_back(verts_twisted[3]); // D
        new_single_poly_vids.push_back(verts_twisted[0]); // A
        new_single_poly_vids.push_back(EC);
        new_single_poly_vids.push_back(HB);
        new_single_poly_vids.push_back(verts_twisted[7]); // H
        new_single_poly_vids.push_back(verts_twisted[4]); // E
        new_polys_vids.push_back(new_single_poly_vids);

        break;
    }
    default:
        std::cout << "No possible split" << std::endl;
        padding_flag = false;
        break;
    }

    for (const auto &poly_vids : new_polys_vids)
    {
        poly_mesh.poly_add(poly_vids);
    }

   
    poly_mesh.update_bbox();
    poly_mesh.update_quality();
    poly_mesh.update_normals();
    return padding_flag;
}

int main(int argc, char **argv)
{
    using namespace cinolib;

    // Vertici del cubo unitario (v0–v7)
    // std::vector<vec3d> verts = {};
    // verts.push_back(vec3d{0, 0, 0});
    // verts.push_back(vec3d{0, 0, 1});
    // verts.push_back(vec3d{1, 0, 1});
    // verts.push_back(vec3d{1, 0, 0});
    // verts.push_back(vec3d{0, 1, 0});
    // verts.push_back(vec3d{0, 1, 1});
    // verts.push_back(vec3d{1, 1, 1});
    // verts.push_back(vec3d{1, 1, 0});
    // std::vector<std::vector<uint>> faces = {
    //     {0, 3, 2, 1}, // bottom
    //     {2, 3, 7, 6}, // back
    //     {4, 5, 6, 7}, // top
    //     {0, 1, 5, 4}, // front
    //     {3, 0, 4, 7}, // left
    //     {1, 2, 6, 5}, // right
    // };
    // std::vector<std::vector<uint>> polys = {{0, 1, 2, 3, 4, 5}};
    // std::vector<std::vector<bool>> polys_face_winding = {{true, true, true, true, true, true}};

    // DrawablePolyhedralmesh<> poly_mesh(verts, faces, polys, polys_face_winding);

    DrawableHexmesh<> poly_mesh;
    grid_mesh(3, 3, 3, poly_mesh);

    // marko le facce di superficie
    std::map<uint, std::vector<uint>> surf_flags;
    for (uint fid = 0; fid < poly_mesh.num_faces(); ++fid)
    {
        if (poly_mesh.face_is_on_srf(fid))
        {
            uint pid = poly_mesh.adj_f2p(fid)[0];
            surf_flags[pid].push_back(fid);
        }
    }




    poly_mesh.poly_fix_orientation(); // orientamento coerente dei poliedri
    
    std::vector <uint> padded_polys;
    for (const auto &[key, list] : surf_flags)
    {
       if(pad_faces(poly_mesh, key, list))
       {
            padded_polys.push_back(key);
       }
    }

    // rimuovo i poliedri originali che sono stati splittati dopo il processo per preservare la coerenza della mesh
    for (uint i=0; i<padded_polys.size(); ++i)
    {
        poly_mesh.poly_remove(padded_polys[i]);
    }

    poly_mesh.updateGL();

    GLcanvas gui;
    VolumeMeshControls<DrawableHexmesh<>> menu(&poly_mesh, &gui, "Hex Mesh Controls");

    gui.push(&poly_mesh);
    gui.push(&menu);

    gui.launch();
    return 0;
}
