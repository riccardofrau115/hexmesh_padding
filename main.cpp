#include <cinolib/meshes/meshes.h>
#include <cinolib/gl/glcanvas.h>
#include <cinolib/gl/volume_mesh_controls.h>
#include "cinolib/hex_transition_schemes.h"
#include <cinolib/dual_mesh.h>
#include <cinolib/meshes/polyhedralmesh.h>
#include <cinolib/export_cluster.h>
#include <cinolib/hex_shift_indices.h>
#include <cinolib/grid_mesh.h>
using namespace cinolib;
const double lambda = 1 / 3.0; // posizione di split lungo l'edge
struct PolyPadStructure
{
    std::set<uint> faces;
    std::set<uint> edges;
    std::set<uint> vertices;
};


// utilizziamo una lista di vertici e non una lista di edge perché l'ordinamento dei vertici implica una posizione diversa del nuovo punto
// TODO il caso in cui un edge abbia due split inversi da gestire
std::map<std::vector<uint>, uint> vertex_edge_padding_map; // new vertex id -> old vertices ids

std::vector<uint> padded_polys;

// creo una mappa che associa la faccia del poliedro originale alla lista di vertici dei poliedri creati
uint retrieve_create_vertex_edge(DrawableHexmesh<> &m, std::vector<uint> verts)
{
    // std::cout << "Retrieving/creating vertex for edge with vertices: " << verts[0] << " and " << verts[1] << std::endl;
    if (vertex_edge_padding_map.find(verts) != vertex_edge_padding_map.end())
    {
        return vertex_edge_padding_map[verts];
    }
    vec3d split_point = (1 - lambda) * m.vert(verts[0]) + lambda * m.vert(verts[1]);
    uint vid = m.vert_add(split_point);
    vertex_edge_padding_map[verts] = vid;
    return vid;
}

// devo usare per forza i vertici perché non sono ancora aggiunti al poliedro
std::map<std::vector<uint>, std::vector<uint>> faces_map_outin; // faccia originale -> lista di vertici di facce dei poliedri creati
std::unordered_map<uint, uint> faces_map_outin_mesh;            // id faccia poliedro creato -> id faccia poliedro originale

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

// verifica che le facce da paddare (input og) corrispondano alle facce dello schema statico (projected)
bool check_face_rotation(const std::vector<std::unordered_set<uint>> &vertices_pjface, const std::vector<std::unordered_set<uint>> &vertices_ogface, std::vector<uint> faces_to_pad, std::vector<uint> face_static)
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

std::vector<uint> arrange_face_rotation(const std::vector<uint> &verts_og,
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

            rotation = check_face_rotation(vertices_pjface, vertices_ogface, faces_to_pad, faces_static);
            if (rotation)
                break;
        }

        if (rotation)
            break;
    }

    if (!rotation)
    {
        std::cout << "Faces to pad: " << std::endl;
        for (auto fid : faces_to_pad)
        {
            std::cout << fid << " ";
        }
        std::cout << std::endl;
        throw std::runtime_error("No valid rotation found for the given faces to pad and static faces.");
    }

    rotation_indeces.push_back(i);
    rotation_indeces.push_back(j);
    // std::cout << "Rotation found: " << i << " " << j << std::endl;
    return rotation_indeces;
}

bool arrange_vert_rotation(const std::vector<uint> &verts_og,
                           std::vector<uint> &verts_rebase,
                           std::vector<uint> &verts_twisted,
                           const std::vector<uint> &verts_to_pad, // vertici indicati dall'input da paddare
                           const std::vector<uint> &verts_static  // vertici che sono implementati nello switch
)
{
    // verifico la rotazione tramite la posizione dei vertici, dato l'ordine
    // a0 b1 c2 d3 e4 f5 g6 h7

    uint i, j;
    bool rotation = true;

    for (i = 0; i < 6; i++)
    {
        // std::cout << "Trying rotation " << i << std::endl;
        hex_rebase(verts_og.data(), i, verts_rebase.data());
        for (j = 0; j < 4; j++)
        {
            hex_around_axis(verts_rebase.data(), j, verts_twisted.data());
            std::unordered_map<uint, uint> vert_position_map;
            for (uint i = 0; i < verts_twisted.size(); i++)
            {
                vert_position_map[verts_twisted[i]] = i;
                // std::cout << "Vertice " << verts_twisted[i] << " in posizione " << i << std::endl;
            }

            //  guarda che posiziona occupa il vertice da paddare
            for (uint vert : verts_to_pad)
            {
                uint pos = vert_position_map[vert]; // in che posizione è finito il vertice iniziale?

                if (std::find(verts_static.begin(), verts_static.end(), pos) == verts_static.end())
                {
                    // nel caso non si trovi corrispondenza di uno dei vertici si prova un'altra rotazione
                    rotation = false;
                    break;
                }
                rotation = true;
            }

            if (rotation)
            {
                break;
            }
        }

        if (rotation)
        {
            break;
        }
    }
    return rotation;
}

bool pad_poly(
    DrawableHexmesh<> &poly_mesh,
    const uint pid,
    const PolyPadStructure pp_struct

)
{
    std::vector<uint> faces_to_pad(pp_struct.faces.begin(), pp_struct.faces.end());
    std::set<uint> faces_to_pad_new;
    const std::vector<uint> edges_to_pad(pp_struct.edges.begin(), pp_struct.edges.end());
    const std::vector<uint> vertices_to_pad(pp_struct.vertices.begin(), pp_struct.vertices.end());

    /// LOGICA DI PADDING
    // if (padding_edges)
    // {
    //     padding_edge(og);
    //     if (padding_faces)
    //     {
    //         padding_face(padded);
    //     }
    // }
    // else if (padding_faces)
    // {
    //     padding_face(og);
    // }

    std::vector<uint> verts_og = poly_mesh.poly_verts_id(pid);
    std::vector<std::vector<uint>> new_polys_vids;
    std::vector<uint> verts_rebase = verts_og;
    std::vector<uint> verts_twisted = verts_og;
    std::vector<uint> new_single_poly_vids;
    std::vector<uint> new_vids;
    bool padding_flag = true;

    /// PAD EDGES
    if (edges_to_pad.size() > 0)
    {
        // TODO implementare la rotazione per edge
        // dato che attualmente utilizzo i vertici converto i vertici da mesh a poliedro
        // ottengo i vertici tramite pid
        std::unordered_map<uint, uint> vert_to_poly_map; // mappa da mesh a poliedro
        for (uint i = 0; i < verts_og.size(); i++)
        {
            vert_to_poly_map[verts_og[i]] = i;
        }

        // converto l'edge di mesh nei vertici del poliedro
        std::vector<uint> edges_converted_vertices;
        for (auto edge_id : edges_to_pad)
        {
            std::vector<uint> edge_verts = poly_mesh.edge_vert_ids(edge_id);
            for (uint v : edge_verts)
            {
                if (vert_to_poly_map.find(v) != vert_to_poly_map.end())
                {
                    edges_converted_vertices.push_back(vert_to_poly_map[v]);
                }
            }
        }

        switch (edges_to_pad.size())
        {
        case 1:
        {
            // padding di un edge
            // TODO rotazione per edge, attualmente ho il caso di edge unico e non c'è problema di ambiguità
            // padding statico dell'edge EH, vertici 4 e 7
            if (!arrange_vert_rotation({0, 1, 2, 3, 4, 5, 6, 7}, verts_rebase, verts_twisted, edges_converted_vertices, {4, 7}))
            {
                throw std::runtime_error("No valid rotation found for the given edge to pad and static vertices.");
            }

            uint A = verts_og[verts_twisted[0]];
            uint B = verts_og[verts_twisted[1]];
            uint C = verts_og[verts_twisted[2]];
            uint D = verts_og[verts_twisted[3]];
            uint E = verts_og[verts_twisted[4]];
            uint F = verts_og[verts_twisted[5]];
            uint G = verts_og[verts_twisted[6]];
            uint H = verts_og[verts_twisted[7]];
            uint EF = retrieve_create_vertex_edge(poly_mesh, {E, F});
            uint HG = retrieve_create_vertex_edge(poly_mesh, {H, G});
            uint EB = retrieve_create_vertex_edge(poly_mesh, {E, B});
            uint HC = retrieve_create_vertex_edge(poly_mesh, {H, C});
            uint EA = retrieve_create_vertex_edge(poly_mesh, {E, A});
            uint HD = retrieve_create_vertex_edge(poly_mesh, {H, D});

            new_polys_vids.clear();
            new_single_poly_vids.clear();

            new_single_poly_vids.push_back(A);
            new_single_poly_vids.push_back(B);
            new_single_poly_vids.push_back(C);
            new_single_poly_vids.push_back(D);
            new_single_poly_vids.push_back(EA);
            new_single_poly_vids.push_back(EB);
            new_single_poly_vids.push_back(HC);
            new_single_poly_vids.push_back(HD);

            faces_map_outin[{A, B, C, D}] = {A, B, C, D};
            faces_map_outin[{A, D, HD, EA}] = {A, D, H, E};
            faces_map_outin[{C, D, HD, HC}] = {C, D, H, G};
            faces_map_outin[{A, B, EB, EA}] = {A, B, E, F};

            new_polys_vids.push_back(new_single_poly_vids);
            new_single_poly_vids.clear();

            new_single_poly_vids.push_back(EB);
            new_single_poly_vids.push_back(B);
            new_single_poly_vids.push_back(C);
            new_single_poly_vids.push_back(HC);
            new_single_poly_vids.push_back(EF);
            new_single_poly_vids.push_back(F);
            new_single_poly_vids.push_back(G);
            new_single_poly_vids.push_back(HG);

            faces_map_outin[{G, F, B, C}] = {G, F, B, C};
            faces_map_outin[{G, HG, HC, C}] = {G, H, C, D};
            faces_map_outin[{F, EF, EB, B}] = {F, E, B, A};
            faces_map_outin[{E, F, G, H}] = {E, F, G, H};

            new_polys_vids.push_back(new_single_poly_vids);
            new_single_poly_vids.clear();

            new_single_poly_vids.push_back(EA);
            new_single_poly_vids.push_back(EB);
            new_single_poly_vids.push_back(HC);
            new_single_poly_vids.push_back(HD);
            new_single_poly_vids.push_back(E);
            new_single_poly_vids.push_back(EF);
            new_single_poly_vids.push_back(HG);
            new_single_poly_vids.push_back(H);

            faces_map_outin[{E, EF, HG, H}] = {E, F, G, H};
            faces_map_outin[{E, H, HD, EA}] = {E, H, D, A};
            faces_map_outin[{H, HG, HC, HD}] = {H, G, C, D};
            faces_map_outin[{E, EF, EB, EA}] = {E, F, B, A};

            new_polys_vids.push_back(new_single_poly_vids);

            break;
        }
        default:
            padding_flag = false;
            break;
        }

        std::vector<uint> new_poly_ids;
        if (padding_flag)
        {
            for (const auto &poly_vids : new_polys_vids)
            {
                new_poly_ids.push_back(poly_mesh.poly_add(poly_vids));
            }
        }

        // popolo la mappa di facce out ed in convertendo i vertici in fid
        for (const auto &[out_verts, in_verts] : faces_map_outin)
        {
            uint fid_out = poly_mesh.face_id(out_verts);
            uint fid_in = poly_mesh.face_id(in_verts);
            faces_map_outin_mesh[fid_out] = fid_in;
        }

        /// per ogni poliedro creato verifico che ci siano facce da paddare

        if (faces_to_pad.size() > 0)
        {
            PolyPadStructure pp;
            std::cout << "pid originale: " << pid << std::endl;
            for (uint new_pid : new_poly_ids)
            {
                faces_to_pad_new.clear();
                for (uint new_fid : poly_mesh.adj_p2f(new_pid))
                {

                    uint old_fid = faces_map_outin_mesh[new_fid];
                    auto it = std::find(faces_to_pad.begin(), faces_to_pad.end(), old_fid);
                    if (it != faces_to_pad.end())
                    {
                        faces_to_pad_new.insert(new_fid);
                    }
                    if (faces_to_pad_new.size() > 0)
                    {
                        pp.faces = faces_to_pad_new;
                    }
                }
                if (pad_poly(poly_mesh, new_pid, pp))
                {
                    padded_polys.push_back(new_pid);
                }
                std::cout << std::endl;
            }
            faces_to_pad.clear();
        }
    }

    /// PAD FACES
    else if (faces_to_pad.size() > 0)
    {
        // vertici di faccia con la configurazione iniziale
        std::vector<std::unordered_set<uint>> vertices_ogface(6);
        for (int i = 0; i < 6; i++)
        {
            vertices_ogface[i] = vertices_from_face(verts_og, i);
        }

        // converto dal fid di mesh al valore singolo del poliedro
        /// TODO ci serve davvero?
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

        switch (faces_to_pad_converted.size())
        {
        case 1:
        {
            // std::cout << "Split along face " << faces_to_pad_converted[0] << std::endl;
            arrange_face_rotation(verts_og, faces_to_pad_converted, verts_rebase, verts_twisted, {0}, vertices_ogface);
            // padding della faccia 0

            uint AE = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[0], verts_twisted[4]});
            uint BF = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[1], verts_twisted[5]});
            uint CG = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[2], verts_twisted[6]});
            uint DH = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[3], verts_twisted[7]});

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

            // std::cout << "Split along faces " << faces_to_pad_converted[0] << " and " << faces_to_pad_converted[1] << std::endl;
            /// caso in cui le facce siano opposte
            if (!poly_mesh.faces_are_adjacent(faces_to_pad_converted[0], faces_to_pad_converted[1]))
            {
                // padding delle facce 1 e 3
                arrange_face_rotation(verts_og, faces_to_pad_converted, verts_rebase, verts_twisted, {3, 1}, vertices_ogface);

                uint AB = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[0], verts_twisted[1]});
                uint BA = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[1], verts_twisted[0]});
                uint CD = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[2], verts_twisted[3]});
                uint DC = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[3], verts_twisted[2]});

                uint EF = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[4], verts_twisted[5]});
                uint FE = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[5], verts_twisted[4]});
                uint GH = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[6], verts_twisted[7]});
                uint HG = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[7], verts_twisted[6]});

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
                arrange_face_rotation(verts_og, faces_to_pad_converted, verts_rebase, verts_twisted, {3, 5}, vertices_ogface);

                // bottom face
                uint AB = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[0], verts_twisted[1]});
                uint CB = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[2], verts_twisted[1]});
                uint DB = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[3], verts_twisted[1]});
                // top face
                uint EF = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[4], verts_twisted[5]});
                uint GF = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[6], verts_twisted[5]});
                uint HF = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[7], verts_twisted[5]});

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
            // std::cout << "Split along faces " << faces_to_pad_converted[0] << ", " << faces_to_pad_converted[1] << " and " << faces_to_pad_converted[2] << std::endl;
            //  facce ad angolo, tutte le facce sono adiacenti tra loro

            bool corner =
                (poly_mesh.faces_are_adjacent(faces_to_pad_converted[0], faces_to_pad_converted[1]) &&
                 poly_mesh.faces_are_adjacent(faces_to_pad_converted[1], faces_to_pad_converted[2]) &&
                 poly_mesh.faces_are_adjacent(faces_to_pad_converted[2], faces_to_pad_converted[0]));

            if (corner)
            {
                // padding delle facce 0,1,4
                arrange_face_rotation(verts_og, faces_to_pad_converted, verts_rebase, verts_twisted, {0, 1, 4}, vertices_ogface);

                uint AH = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[0], verts_twisted[7]});
                uint BH = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[1], verts_twisted[7]});
                uint CH = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[2], verts_twisted[7]});
                uint DH = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[3], verts_twisted[7]});
                uint EH = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[4], verts_twisted[7]});
                uint FH = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[5], verts_twisted[7]});
                uint GH = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[6], verts_twisted[7]});

                new_polys_vids.clear();

                // AH BH CH DH EH FH GH H;
                new_single_poly_vids.clear();
                new_single_poly_vids.push_back(AH);
                new_single_poly_vids.push_back(BH);
                new_single_poly_vids.push_back(CH);
                new_single_poly_vids.push_back(DH);
                new_single_poly_vids.push_back(EH);
                new_single_poly_vids.push_back(FH);
                new_single_poly_vids.push_back(GH);
                new_single_poly_vids.push_back(verts_twisted[7]); // H
                new_polys_vids.push_back(new_single_poly_vids);

                //  A B C D AH BH CH DH;
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

                // BH B C CH FH F G GH;
                new_single_poly_vids.clear();
                new_single_poly_vids.push_back(BH);
                new_single_poly_vids.push_back(verts_twisted[1]); // B
                new_single_poly_vids.push_back(verts_twisted[2]); // C
                new_single_poly_vids.push_back(CH);
                new_single_poly_vids.push_back(FH);
                new_single_poly_vids.push_back(verts_twisted[5]); // F
                new_single_poly_vids.push_back(verts_twisted[6]); // G
                new_single_poly_vids.push_back(GH);
                new_polys_vids.push_back(new_single_poly_vids);

                // A B BH AH E F FH EH;
                new_single_poly_vids.clear();
                new_single_poly_vids.push_back(verts_twisted[0]); // A
                new_single_poly_vids.push_back(verts_twisted[1]); // B
                new_single_poly_vids.push_back(BH);
                new_single_poly_vids.push_back(AH);
                new_single_poly_vids.push_back(verts_twisted[4]); // E
                new_single_poly_vids.push_back(verts_twisted[5]); // F
                new_single_poly_vids.push_back(FH);
                new_single_poly_vids.push_back(EH);
                new_polys_vids.push_back(new_single_poly_vids);
            }
            else // facce a forma di U
            {
                // padding delle facce 5, 1, 3
                arrange_face_rotation(verts_og, faces_to_pad_converted, verts_rebase, verts_twisted, {5, 1, 3}, vertices_ogface);

                uint AB = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[0], verts_twisted[1]});
                uint BA = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[1], verts_twisted[0]});
                uint DB = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[3], verts_twisted[1]});
                uint CA = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[2], verts_twisted[0]});

                uint EF = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[4], verts_twisted[5]});
                uint FE = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[5], verts_twisted[4]});
                uint GE = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[6], verts_twisted[4]});
                uint HF = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[7], verts_twisted[5]});

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
            // std::cout << "Split along faces " << faces_to_pad_converted[0] << ", " << faces_to_pad_converted[1] << ", " << faces_to_pad_converted[2] << " and " << faces_to_pad_converted[3] << std::endl;
            std::vector<uint> faces_not_to_pad;

            // verifica che le facce escluse dal padding siano adiacenti
            for (int i = 0; i < 6; i++)
            {
                if (std::find(faces_to_pad_converted.begin(), faces_to_pad_converted.end(), i) == faces_to_pad_converted.end())
                {
                    faces_not_to_pad.push_back(i);
                }
            }

            if (poly_mesh.faces_are_adjacent(faces_not_to_pad[0], faces_not_to_pad[1]))
            {
                // caso in cui le facce da escludere siano adiacenti tra loro
                // padding delle facce 0, 1, 2, 4
                arrange_face_rotation(verts_og, faces_to_pad_converted, verts_rebase, verts_twisted, {0, 1, 2, 4}, vertices_ogface);

                uint AH = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[0], verts_twisted[7]});
                uint BH = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[1], verts_twisted[7]});
                uint CH = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[2], verts_twisted[7]});
                uint DH = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[3], verts_twisted[7]});

                uint ED = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[4], verts_twisted[3]});
                uint FD = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[5], verts_twisted[3]});
                uint GD = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[6], verts_twisted[3]});
                uint HD = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[7], verts_twisted[3]});

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
                arrange_face_rotation(verts_og, faces_to_pad_converted, verts_rebase, verts_twisted, {1, 3, 4, 5}, vertices_ogface);

                uint AC = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[0], verts_twisted[2]});
                uint BD = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[1], verts_twisted[3]});
                uint CA = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[2], verts_twisted[0]});
                uint DB = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[3], verts_twisted[1]});

                uint EG = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[4], verts_twisted[6]});
                uint FH = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[5], verts_twisted[7]});
                uint GE = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[6], verts_twisted[4]});
                uint HF = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[7], verts_twisted[5]});

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
            // std::cout << "Split along faces " << faces_to_pad_converted[0] << ", " << faces_to_pad_converted[1] << ", " << faces_to_pad_converted[2] << ", " << faces_to_pad_converted[3] << " and " << faces_to_pad_converted[4] << std::endl;

            // padding delle facce 0, 1, 2, 3, 4
            arrange_face_rotation(verts_og, faces_to_pad_converted, verts_rebase, verts_twisted, {0, 1, 2, 3, 4}, vertices_ogface);

            uint AG = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[0], verts_twisted[6]});
            uint BH = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[1], verts_twisted[7]});
            uint CH = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[2], verts_twisted[7]});
            uint DG = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[3], verts_twisted[6]});

            uint EC = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[4], verts_twisted[2]});
            uint FD = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[5], verts_twisted[3]});
            uint GD = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[6], verts_twisted[3]});
            uint HC = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[7], verts_twisted[2]});

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
            // std::cout << "Split along  all faces" << std::endl;
            //  padding di tutte le facce, nessuna rotazione necessaria

            uint AG = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[0], verts_twisted[6]});
            uint BH = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[1], verts_twisted[7]});
            uint CE = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[2], verts_twisted[4]});
            uint DF = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[3], verts_twisted[5]});

            uint EC = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[4], verts_twisted[2]});
            uint FD = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[5], verts_twisted[3]});
            uint GA = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[6], verts_twisted[0]});
            uint HB = retrieve_create_vertex_edge(poly_mesh, {verts_twisted[7], verts_twisted[1]});

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

        if (padding_flag)
        {
            for (const auto &poly_vids : new_polys_vids)
            {
                poly_mesh.poly_add(poly_vids);
            }
        }
    }


    poly_mesh.update_bbox();
    poly_mesh.update_quality();
    poly_mesh.update_normals();
    return padding_flag;
}

int main(int argc, char **argv)
{
    DrawableHexmesh<> poly_mesh;
    grid_mesh(3, 3, 3, poly_mesh);

    // rimozione poliedri per testing
    poly_mesh.polys_remove({26, 25, 24});

    std::cout << "creazione struttura dati per il padding..." << std::endl;
    // marko le facce di superficie
    // TODO unire al padding
    std::map<uint, PolyPadStructure> polys_to_pad;
    for (uint fid = 0; fid < poly_mesh.num_faces(); ++fid)
    {
        if (poly_mesh.face_is_on_srf(fid))
        {
            uint pid = poly_mesh.adj_f2p(fid)[0]; // prendo il poliedro adiacente alla faccia di superficie
            polys_to_pad[pid].faces.insert(fid);
            // inoltre scompongo la faccia negli edge e li aggiungo alla mappa per il padding
            for (uint eid : poly_mesh.adj_f2e(fid))
            {
                // ogni edge va aggiunto ai poliedri adiacenti
                std::vector<uint> poly_list = poly_mesh.adj_e2p(eid);
                for (uint pid_edge : poly_list)
                {
                    polys_to_pad[pid_edge].edges.insert(eid);
                }
            }
        }
    }

    // std::cout << "rimozione ripetizioni di edge già contenuti in una faccia..." << std::endl;
    //  rimuovo le ripetizioni di edge già contenuti in una faccia
    //  per ogni poliedro con almeno una faccia da paddare
    for (auto &[pid, elements] : polys_to_pad)
    {
        for (uint fid : polys_to_pad[pid].faces)
        {
            // std::cout << "faccia " << fid;

            for (auto it = polys_to_pad[pid].edges.begin(); it != polys_to_pad[pid].edges.end();)
            {

                if (poly_mesh.face_contains_edge(fid, *it))
                {
                    it = polys_to_pad[pid].edges.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }
    }

    poly_mesh.poly_fix_orientation(); // orientamento coerente dei poliedri

    // eseguo il padding
    for (const auto &[key, pp_struct] : polys_to_pad)
    {
        if (pad_poly(poly_mesh, key, pp_struct))
        {
            padded_polys.push_back(key);
        }
    }

    poly_mesh.polys_remove(padded_polys);
    poly_mesh.updateGL();

    GLcanvas gui;
    VolumeMeshControls<DrawableHexmesh<>> menu(&poly_mesh, &gui, "Hex Mesh Controls");

    gui.push(&poly_mesh);
    gui.push(&menu);

    gui.launch();
    return 0;
}
