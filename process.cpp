//-------------------------------------------------------------------------------------
// process.cpp
//  
// Copyright (c) Microsoft Corporation. All rights reserved.
//-------------------------------------------------------------------------------------

#include "directxtest.h"

#include "uvatlas.h"
#include "directxmesh.h"

#include "TestHelpers.h"
#include "WaveFrontReader.h"

using namespace DirectX;

struct TestMedia
{
    const wchar_t * fname;
};

static const TestMedia g_TestMedia16[] =
{
    // filename
    { MESH_MEDIA_PATH L"cup._obj",               },
    { MESH_MEDIA_PATH L"teapot._obj",            },
    { MESH_MEDIA_PATH L"SuperSimpleRunner._obj", },
    { MESH_MEDIA_PATH L"shuttle._obj",           },
    { MESH_MEDIA_PATH L"player_ship_a._obj",     },
    { MESH_MEDIA_PATH L"FSEngineGeo._obj",       },
    { MESH_MEDIA_PATH L"sphere.vbo",             },
    { MESH_MEDIA_PATH L"cylinder.vbo",           },
    { MESH_MEDIA_PATH L"torus.vbo",              },
};

static const TestMedia g_TestMedia32[] =
{
    // filename
    { MESH_MEDIA_PATH L"Head_Big_Ears._obj",     },
    { MESH_MEDIA_PATH L"John40k._obj",           },
};


//-------------------------------------------------------------------------------------
static inline bool IsValidFacePartition( const uint32_t* facePart, size_t nFaces, size_t nCharts )
{
    if ( !facePart )
        return false;

    for( size_t j = 0; j < nFaces; ++j )
    {
        if ( facePart[j] >= nCharts )
            return false;
    }

    return true;
}


//-------------------------------------------------------------------------------------
static inline bool VerifyVertices( const XMFLOAT3* pos, size_t nVerts, const UVAtlasVertex* verts, const uint32_t* remap, size_t nTotalVerts )
{
    if ( !pos || !verts || !remap )
        return false;

    auto vptr = verts;
    for( size_t j = 0; j < nTotalVerts; ++j, ++vptr )
    {
        size_t oldn = remap[j];
        if ( oldn  >= nVerts )
            return false;

        XMVECTOR v1 = XMLoadFloat3( &pos[ oldn ] );
        XMVECTOR v2 = XMLoadFloat3( &vptr->pos );

        if ( !XMVector3NearEqual( v1, v2, g_XMEpsilon ) )
            return false;
    }

    return true;
}


//-------------------------------------------------------------------------------------
static HRESULT __cdecl UVAtlasCallback( float fPercentDone  )
{
    UNREFERENCED_PARAMETER(fPercentDone);

    static ULONGLONG s_lastTick = 0;

    ULONGLONG tick = GetTickCount64();

    if ( ( tick - s_lastTick ) > 1000 )
    {
        print(".");
        s_lastTick = tick;
    }

    return S_OK;
}



//-------------------------------------------------------------------------------------
// MeshProcess (16-bit)
bool Test08()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for( size_t index=0; index < _countof(g_TestMedia16); ++index )
    {
        wchar_t szPath[MAX_PATH] = {};
        DWORD ret = ExpandEnvironmentStringsW( g_TestMedia16[index].fname, szPath, MAX_PATH );
        if ( !ret || ret > MAX_PATH )  
        {  
            printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );  
            return false;  
        } 

#ifdef _DEBUG
        OutputDebugStringW(szPath);
        OutputDebugStringA("\n");
#endif

        wchar_t ext[_MAX_EXT];
        _wsplitpath_s( szPath, nullptr, 0, nullptr, 0, nullptr, 0, ext, _MAX_EXT );

        ++ncount;

        std::unique_ptr<WaveFrontReader<uint16_t>> mesh( new WaveFrontReader<uint16_t>() );

        print( "*" );

        HRESULT hr;
        if ( _wcsicmp( ext, L".vbo" ) == 0 )
        {
            hr = mesh->LoadVBO( szPath );
        }
        else
        {
            hr = mesh->Load( szPath );
        }

        if ( FAILED(hr) )
        {
            success = false;
            printe( "ERROR: Failed loading mesh data (%08X):\n%S\n", hr, szPath );
            continue;
        }

        size_t nFaces = mesh->indices.size() / 3;
        size_t nVerts = mesh->vertices.size();

#ifdef _DEBUG
        char output[ 256 ] = {};
        sprintf_s( output, "INFO: %zu verts, %zu faces\n", nVerts, nFaces );
        OutputDebugStringA( output );
#endif

        std::wstring msgs;
        hr = Validate( mesh->indices.data(), nFaces, nVerts, nullptr, VALIDATE_DEFAULT, &msgs );
        if ( FAILED(hr) )
        {
            success = false;
            printe( "ERROR: Failed Validate mesh data (%08X):\n%S\n%S\n", hr, szPath, msgs.c_str() );
            continue;
        }

#ifdef _DEBUG
        hr = Validate( mesh->indices.data(), nFaces, nVerts, nullptr, VALIDATE_DEGENERATE, &msgs );
        if ( FAILED(hr) )
        {
            OutputDebugStringW( msgs.c_str() );
        }
#endif

        std::unique_ptr<XMFLOAT3[]> pos( new XMFLOAT3[ nVerts ] );
        for( size_t j = 0; j < nVerts; ++j )
            pos[ j ] = mesh->vertices[ j ].position;

        std::unique_ptr<uint32_t[]> adj( new uint32_t[ mesh->indices.size() ] );
        memset( adj.get(), 0xff, sizeof(uint32_t) *  mesh->indices.size() );

        hr = GenerateAdjacencyAndPointReps( mesh->indices.data(), nFaces, pos.get(), nVerts, 0.f, nullptr, adj.get() );
        if ( FAILED(hr) )
        {
            success = false;
            printe("ERROR: failed GenerateAdjacencyAndPointReps (%08X)\n:%S\n", hr, szPath );
            continue;
        }

        std::vector<UVAtlasVertex> vb;
        std::vector<uint8_t> ib;
        std::vector<uint32_t> facePart;
        std::vector<uint32_t> remap;
        float maxStretch = 0.f;
        size_t numCharts = 0;
        hr = UVAtlasCreate( pos.get(), nVerts, mesh->indices.data(), DXGI_FORMAT_R16_UINT, nFaces,
                            0, 0.f, 512, 512, 1.f,
                            adj.get(), nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                            UVATLAS_DEFAULT, vb, ib, &facePart, &remap, &maxStretch, &numCharts );
        if (FAILED(hr))
        {
            printe( "\nERROR: create atlas failed (%08X)\n%S\n", hr, szPath );
            success = false;
        }
        else if ( vb.size() < nVerts
                  || (ib.size() / (sizeof(uint16_t)*3)) != nFaces
                  || facePart.size() != nFaces
                  || remap.size() != vb.size()
                  || !numCharts )
        {
            printe( "\nERROR: Unexpected results from create atlas:\n%S\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tface partitions %zu\n\tremap array %zu\n\tmaxStretch %f\n\tnumCharts %zu\n",
                    szPath, vb.size(), nFaces, ib.size(), facePart.size(), remap.size(), maxStretch, numCharts );
            success = false;
        }
        else if ( !IsValidVertexRemap( reinterpret_cast<const uint16_t*>( ib.data() ), nFaces, remap.data(), vb.size(), true ) )
        {
            printe( "\nERROR: Vertex remap invalid from create atlas:\n%S\n", szPath );
            success = false;
        }
        else if ( !IsValidFacePartition( facePart.data(), nFaces, numCharts ) )
        {
            printe( "\nERROR: Face partition invalid from create atlas:\n%S\n", szPath );
            success = false;
        }
        else if ( !VerifyVertices( pos.get(), nVerts, vb.data(), remap.data(), vb.size() ) )
        {
            printe( "\nERROR: Vertex data doesn't match remap:\n%S\n", szPath );
            success = false;
        }
        else
        {
            std::wstring msgs;
            hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), nFaces, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs );
            if ( FAILED(hr)) 
            {
                printe( "\nERROR: Invalid index buffer from create atlas (%08X):\n%S\n%S\n", hr, szPath, msgs.c_str() );
                success = false;
            }
        }

        ++npass;
    }

    print("\n%zu meshes tested, %zu meshes passed ", ncount, npass );

    return success;
}



//-------------------------------------------------------------------------------------
// MeshProcess (32-bit)
bool Test11()
{
    bool success = true;

    size_t ncount = 0;
    size_t npass = 0;

    for (size_t index = 0; index < _countof(g_TestMedia32); ++index)
    {
        wchar_t szPath[MAX_PATH] = {};
        DWORD ret = ExpandEnvironmentStringsW(g_TestMedia32[index].fname, szPath, MAX_PATH);
        if (!ret || ret > MAX_PATH)
        {
            printe("ERROR: ExpandEnvironmentStrings FAILED\n");
            return false;
        }

#ifdef _DEBUG
        OutputDebugStringW(szPath);
        OutputDebugStringA("\n");
#endif

        wchar_t ext[_MAX_EXT];
        _wsplitpath_s(szPath, nullptr, 0, nullptr, 0, nullptr, 0, ext, _MAX_EXT);

        ++ncount;

        std::unique_ptr<WaveFrontReader<uint32_t>> mesh(new WaveFrontReader<uint32_t>());

        print("*");

        HRESULT hr;
        if (_wcsicmp(ext, L".vbo") == 0)
        {
            hr = mesh->LoadVBO(szPath);
        }
        else
        {
            hr = mesh->Load(szPath);
        }

        if (FAILED(hr))
        {
            success = false;
            printe("ERROR: Failed loading mesh data (%08X):\n%S\n", hr, szPath);
            continue;
        }

        size_t nFaces = mesh->indices.size() / 3;
        size_t nVerts = mesh->vertices.size();

#ifdef _DEBUG
        char output[256] = {};
        sprintf_s(output, "INFO: %zu verts, %zu faces\n", nVerts, nFaces);
        OutputDebugStringA(output);
#endif

        std::wstring msgs;
        hr = Validate(mesh->indices.data(), nFaces, nVerts, nullptr, VALIDATE_DEFAULT, &msgs);
        if (FAILED(hr))
        {
            success = false;
            printe("ERROR: Failed Validate mesh data (%08X):\n%S\n%S\n", hr, szPath, msgs.c_str());
            continue;
        }

#ifdef _DEBUG
        hr = Validate(mesh->indices.data(), nFaces, nVerts, nullptr, VALIDATE_DEGENERATE, &msgs);
        if (FAILED(hr))
        {
            OutputDebugStringW(msgs.c_str());
        }
#endif

        std::unique_ptr<XMFLOAT3[]> pos(new XMFLOAT3[nVerts]);
        for (size_t j = 0; j < nVerts; ++j)
            pos[j] = mesh->vertices[j].position;

        std::unique_ptr<uint32_t[]> adj(new uint32_t[mesh->indices.size()]);
        memset(adj.get(), 0xff, sizeof(uint32_t) * mesh->indices.size());

        hr = GenerateAdjacencyAndPointReps(mesh->indices.data(), nFaces, pos.get(), nVerts, 0.f, nullptr, adj.get());
        if (FAILED(hr))
        {
            success = false;
            printe("ERROR: failed GenerateAdjacencyAndPointReps (%08X)\n:%S\n", hr, szPath);
            continue;
        }

        std::vector<UVAtlasVertex> vb;
        std::vector<uint8_t> ib;
        std::vector<uint32_t> facePart;
        std::vector<uint32_t> remap;
        float maxStretch = 0.f;
        size_t numCharts = 0;
        hr = UVAtlasCreate(pos.get(), nVerts, mesh->indices.data(), DXGI_FORMAT_R32_UINT, nFaces,
            0, 0.f, 512, 512, 1.f,
            adj.get(), nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
            UVATLAS_DEFAULT, vb, ib, &facePart, &remap, &maxStretch, &numCharts);
        if (FAILED(hr))
        {
            printe("\nERROR: create atlas failed (%08X)\n%S\n", hr, szPath);
            success = false;
        }
        else if (vb.size() < nVerts
            || (ib.size() / (sizeof(uint32_t) * 3)) != nFaces
            || facePart.size() != nFaces
            || remap.size() != vb.size()
            || !numCharts)
        {
            printe("\nERROR: Unexpected results from create atlas:\n%S\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tface partitions %zu\n\tremap array %zu\n\tmaxStretch %f\n\tnumCharts %zu\n",
                szPath, vb.size(), nFaces, ib.size(), facePart.size(), remap.size(), maxStretch, numCharts);
            success = false;
        }
        else if (!IsValidVertexRemap(reinterpret_cast<const uint32_t*>(ib.data()), nFaces, remap.data(), vb.size(), true))
        {
            printe("\nERROR: Vertex remap invalid from create atlas:\n%S\n", szPath);
            success = false;
        }
        else if (!IsValidFacePartition(facePart.data(), nFaces, numCharts))
        {
            printe("\nERROR: Face partition invalid from create atlas:\n%S\n", szPath);
            success = false;
        }
        else if (!VerifyVertices(pos.get(), nVerts, vb.data(), remap.data(), vb.size()))
        {
            printe("\nERROR: Vertex data doesn't match remap:\n%S\n", szPath);
            success = false;
        }
        else
        {
            std::wstring msgs;
            hr = Validate(reinterpret_cast<const uint32_t*>(ib.data()), nFaces, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs);
            if (FAILED(hr))
            {
                printe("\nERROR: Invalid index buffer from create atlas (%08X):\n%S\n%S\n", hr, szPath, msgs.c_str());
                success = false;
            }
        }

        ++npass;
    }

    print("\n%zu meshes tested, %zu meshes passed ", ncount, npass);

    return success;
}
