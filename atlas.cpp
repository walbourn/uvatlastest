//-------------------------------------------------------------------------------------
// atlas.cpp
//
// Copyright (c) Microsoft Corporation.
//-------------------------------------------------------------------------------------

#include "directxtest.h"
#include "TestHelpers.h"
#include "TestGeometry.h"
#include "ShapesGenerator.h"

#include "UVAtlas.h"
#include "DirectXMesh.h"

using namespace DirectX;
using namespace TestGeometry;

static const uint32_t s_fmCubeAdj[3 * 12] =
{
    1, 9, 5,
    7, 0, 11,
    3, 8, 6,
    4, 2, 10,
    5, 3, 11,
    8, 4, 0,
    7, 2, 9,
    10, 6, 1,
    9, 2, 5,
    6, 8, 0,
    11, 3, 7,
    4, 10, 1
};

static const uint32_t s_feCubeAdj[3 * 12] =
{
    uint32_t(-1), uint32_t(-1), uint32_t(-1),
    uint32_t(-1), uint32_t(-1), uint32_t(-1),
    uint32_t(-1), uint32_t(-1), uint32_t(-1),
    uint32_t(-1), uint32_t(-1), uint32_t(-1),
    uint32_t(-1), uint32_t(-1), uint32_t(-1),
    uint32_t(-1), uint32_t(-1), uint32_t(-1),
    uint32_t(-1), uint32_t(-1), uint32_t(-1),
    uint32_t(-1), uint32_t(-1), uint32_t(-1),
    uint32_t(-1), uint32_t(-1), uint32_t(-1),
    uint32_t(-1), uint32_t(-1), uint32_t(-1),
    uint32_t(-1), uint32_t(-1), uint32_t(-1),
    uint32_t(-1), uint32_t(-1), uint32_t(-1),
};

static const float s_fmCubeIMT[3 * 12] =
{
    1.f, 1.f, 1.f,
    1.f, 1.f, 1.f,
    1.f, 1.f, 1.f,
    1.f, 1.f, 1.f,
    1.f, 1.f, 1.f,
    1.f, 1.f, 1.f,
    1.f, 1.f, 1.f,
    1.f, 1.f, 1.f,
    1.f, 1.f, 1.f,
    1.f, 1.f, 1.f,
    1.f, 1.f, 1.f,
    1.f, 1.f, 1.f,
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
// UVAtlasCreate
bool Test01()
{
    bool success = true;
    HRESULT hr;

    // invalid args
    #pragma warning(push)
    #pragma warning(disable : 6385 6387)
    {
        std::vector<UVAtlasVertex> vb;
        std::vector<uint8_t> ib;
        std::vector<uint32_t> facePart;
        std::vector<uint32_t> remap;
        float maxStretch = 0.f;
        size_t numCharts = 0;
        hr = UVAtlasCreate( g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R8G8B8A8_UNORM, 12,
                            0, 0.f, 512, 512, 1.f,
                            s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                            UVATLAS_DEFAULT, vb, ib, &facePart, &remap, &maxStretch, &numCharts );
        if ( hr != E_INVALIDARG )
        {
            printe( "\nERROR: expected failure for wrong DXGI format\n" );
            success = false;
        }

        hr = UVAtlasCreate( nullptr, 0, nullptr, DXGI_FORMAT_R16_UINT, 0,
                            0, 0.f, 512, 512, 1.f,
                            s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                            UVATLAS_DEFAULT, vb, ib, &facePart, &remap, &maxStretch, &numCharts );
        if ( hr != E_INVALIDARG )
        {
            printe( "\nERROR: expected failure for missing input\n" );
            success = false;
        }

        hr = UVAtlasCreate( g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                            0, 0.f, 512, 512, 1.f,
                            nullptr, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                            UVATLAS_DEFAULT, vb, ib, &facePart, &remap, &maxStretch, &numCharts );
        if ( hr != E_INVALIDARG )
        {
            printe( "\nERROR: expected failure for missing adj\n" );
            success = false;
        }

        hr = UVAtlasCreate( g_fmCubeVerts, 0xFFFFFFFF, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                            0, 0.f, 512, 512, 1.f,
                            s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                            UVATLAS_DEFAULT, vb, ib, &facePart, &remap, &maxStretch, &numCharts );
        if ( hr != E_INVALIDARG )
        {
            printe( "\nERROR: expected failure for too many verts\n" );
            success = false;
        }

        hr = UVAtlasCreate( g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                            0, 0.f, 0, 0, 1.f,
                            s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                            UVATLAS_DEFAULT, vb, ib, &facePart, &remap, &maxStretch, &numCharts );
        if ( hr != E_INVALIDARG )
        {
            printe( "\nERROR: expected failure for bad width/height\n" );
            success = false;
        }
    }
    #pragma warning(pop)

    // 16-bit cube
    {
        std::vector<UVAtlasVertex> vb;
        std::vector<uint8_t> ib;
        std::vector<uint32_t> facePart;
        std::vector<uint32_t> remap;
        float maxStretch = 0.f;
        size_t numCharts = 0;
        hr = UVAtlasCreate( g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                            0, 0.f, 512, 512, 1.f,
                            s_fmCubeAdj, nullptr, nullptr, nullptr /*one case without callback*/, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                            UVATLAS_DEFAULT, vb, ib, &facePart, &remap, &maxStretch, &numCharts );
        if (FAILED(hr))
        {
            printe( "\nERROR: create atlas [fmcube16] failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            size_t nFaces = ib.size() / (sizeof(uint16_t)*3);
            if ( vb.size() < 26 || vb.size() > 32
                 || nFaces != 12
                 || facePart.size() != 12
                 || remap.size() != vb.size()
                 || numCharts < 4 || numCharts > 6)
            {
                printe( "\nERROR: Unexpected results from create atlas [fmcube16]\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tface partitions %zu\n\tremap array %zu\n\tmaxStretch %f\n\tnumCharts %zu\n",
                        vb.size(), nFaces, ib.size(), facePart.size(), remap.size(), maxStretch, numCharts );
                success = false;
            }
            else if ( !IsValidVertexRemap( reinterpret_cast<const uint16_t*>( ib.data() ), 12, remap.data(), vb.size(), true ) )
            {
                printe( "\nERROR: Vertex remap invalid from create atlas [fmcube16]\n" );
                success = false;
            }
            else if ( !IsValidFacePartition( facePart.data(), 12, numCharts ) )
            {
                printe( "\nERROR: Face partition invalid from create atlas [fmcube16]\n" );
                success = false;
            }
            else if ( !VerifyVertices( g_fmCubeVerts, 24, vb.data(), remap.data(), vb.size() ) )
            {
                printe( "\nERROR: Vertex data doesn't match remap [fmcube16]\n" );
                success = false;
            }
            else
            {
                std::wstring msgs;
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid index buffer from create atlas [fmcube16] (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }
            }
        }

        // no clear
        hr = UVAtlasCreate( g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                            0, 0.f, 512, 512, 1.f,
                            s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                            UVATLAS_DEFAULT, vb, ib, &facePart, &remap, &maxStretch, &numCharts );
        if (FAILED(hr))
        {
            printe( "\nERROR: create atlas [fmcube16] (2) failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            size_t nFaces = ib.size() / (sizeof(uint16_t)*3);
            if ( vb.size() < 26 || vb.size() > 32
                 || nFaces != 12
                 || facePart.size() != 12
                 || remap.size() != vb.size()
                 || numCharts < 4 || numCharts > 6)
            {
                printe( "\nERROR: Unexpected results from create atlas [fmcube16] (2)\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tface partitions %zu\n\tremap array %zu\n\tmaxStretch %f\n\tnumCharts %zu\n",
                        vb.size(), nFaces, ib.size(), facePart.size(), remap.size(), maxStretch, numCharts );
                success = false;
            }
            else if ( !IsValidVertexRemap( reinterpret_cast<const uint16_t*>( ib.data() ), 12, remap.data(), vb.size(), true ) )
            {
                printe( "\nERROR: Vertex remap invalid from create atlas [fmcube16] (2)\n" );
                success = false;
            }
            else if ( !IsValidFacePartition( facePart.data(), 12, numCharts ) )
            {
                printe( "\nERROR: Face partition invalid from create atlas [fmcube16] (2)\n" );
                success = false;
            }
            else if ( !VerifyVertices( g_fmCubeVerts, 24, vb.data(), remap.data(), vb.size() ) )
            {
                printe( "\nERROR: Vertex data doesn't match remap [fmcube16] (2)\n" );
                success = false;
            }
            else
            {
                std::wstring msgs;
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid index buffer from create atlas [fmcube16] (2) (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }
            }
        }

        // UVATLAS_GEODESIC_FAST
        hr = UVAtlasCreate( g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                            0, 0.f, 512, 512, 1.f,
                            s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                            UVATLAS_GEODESIC_FAST, vb, ib, &facePart, &remap, &maxStretch, &numCharts );
        if (FAILED(hr))
        {
            printe( "\nERROR: create atlas [fmcube16] FAST failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            size_t nFaces = ib.size() / (sizeof(uint16_t)*3);
            if ( vb.size() != 28
                 || nFaces != 12
                 || facePart.size() != 12
                 || remap.size() != vb.size()
                 || numCharts != 7 )
            {
                printe( "\nERROR: Unexpected results from create atlas [fmcube16] FAST\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tface partitions %zu\n\tremap array %zu\n\tmaxStretch %f\n\tnumCharts %zu\n",
                        vb.size(), nFaces, ib.size(), facePart.size(), remap.size(), maxStretch, numCharts );
                success = false;
            }
            else if ( !IsValidVertexRemap( reinterpret_cast<const uint16_t*>( ib.data() ), 12, remap.data(), vb.size(), true ) )
            {
                printe( "\nERROR: Vertex remap invalid from create atlas [fmcube16] FAST\n" );
                success = false;
            }
            else if ( !IsValidFacePartition( facePart.data(), 12, numCharts ) )
            {
                printe( "\nERROR: Face partition invalid from create atlas [fmcube16] FAST\n" );
                success = false;
            }
            else if ( !VerifyVertices( g_fmCubeVerts, 24, vb.data(), remap.data(), vb.size() ) )
            {
                printe( "\nERROR: Vertex data doesn't match remap [fmcube16] FAST\n" );
                success = false;
            }
            else
            {
                std::wstring msgs;
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid index buffer from create atlas [fmcube16] FAST (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }
            }
        }

        // UVATLAS_GEODESIC_QUALITY
        hr = UVAtlasCreate( g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                            0, 0.f, 512, 512, 1.f,
                            s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                            UVATLAS_GEODESIC_QUALITY, vb, ib, &facePart, &remap, &maxStretch, &numCharts );
        if (FAILED(hr))
        {
            printe( "\nERROR: create atlas [fmcube16] QUALITY failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            size_t nFaces = ib.size() / (sizeof(uint16_t)*3);
            if ( vb.size() < 26 || vb.size() > 32
                 || nFaces != 12
                 || facePart.size() != 12
                 || remap.size() != vb.size()
                 || numCharts < 4 || numCharts > 6)
            {
                printe( "\nERROR: Unexpected results from create atlas [fmcube16] QUALITY\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tface partitions %zu\n\tremap array %zu\n\tmaxStretch %f\n\tnumCharts %zu\n",
                        vb.size(), nFaces, ib.size(), facePart.size(), remap.size(), maxStretch, numCharts );
                success = false;
            }
            else if ( !IsValidVertexRemap( reinterpret_cast<const uint16_t*>( ib.data() ), 12, remap.data(), vb.size(), true ) )
            {
                printe( "\nERROR: Vertex remap invalid from create atlas [fmcube16] QUALITY\n" );
                success = false;
            }
            else if ( !IsValidFacePartition( facePart.data(), 12, numCharts ) )
            {
                printe( "\nERROR: Face partition invalid from create atlas [fmcube16] QUALITY\n" );
                success = false;
            }
            else if ( !VerifyVertices( g_fmCubeVerts, 24, vb.data(), remap.data(), vb.size() ) )
            {
                printe( "\nERROR: Vertex data doesn't match remap [fmcube16] QUALITY\n" );
                success = false;
            }
            else
            {
                std::wstring msgs;
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid index buffer from create atlas [fmcube16] QUALITY (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }
            }
        }

        // UVATLAS_LIMIT_MERGE_STRETCH
        hr = UVAtlasCreate(g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
            0, 0.f, 512, 512, 1.f,
            s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
            UVATLAS_LIMIT_MERGE_STRETCH, vb, ib, &facePart, &remap, &maxStretch, &numCharts);
        if (FAILED(hr))
        {
            printe("\nERROR: create atlas [fmcube16] LIMIT_MERGE_STRETCH failed (%08X)\n", static_cast<unsigned int>(hr));
            success = false;
        }
        else
        {
            size_t nFaces = ib.size() / (sizeof(uint16_t) * 3);
            if (vb.size() < 26 || vb.size() > 32
                || nFaces != 12
                || facePart.size() != 12
                || remap.size() != vb.size()
                || numCharts < 4 || numCharts > 6)
            {
                printe("\nERROR: Unexpected results from create atlas [fmcube16] LIMIT_MERGE_STRETCH\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tface partitions %zu\n\tremap array %zu\n\tmaxStretch %f\n\tnumCharts %zu\n",
                    vb.size(), nFaces, ib.size(), facePart.size(), remap.size(), maxStretch, numCharts);
                success = false;
            }
            else if (!IsValidVertexRemap(reinterpret_cast<const uint16_t*>(ib.data()), 12, remap.data(), vb.size(), true))
            {
                printe("\nERROR: Vertex remap invalid from create atlas [fmcube16] LIMIT_MERGE_STRETCH\n");
                success = false;
            }
            else if (!IsValidFacePartition(facePart.data(), 12, numCharts))
            {
                printe("\nERROR: Face partition invalid from create atlas [fmcube16] LIMIT_MERGE_STRETCH\n");
                success = false;
            }
            else if (!VerifyVertices(g_fmCubeVerts, 24, vb.data(), remap.data(), vb.size()))
            {
                printe("\nERROR: Vertex data doesn't match remap [fmcube16] LIMIT_MERGE_STRETCH\n");
                success = false;
            }
            else
            {
                std::wstring msgs;
                hr = Validate(reinterpret_cast<const uint16_t*>(ib.data()), 12, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs);
                if (FAILED(hr))
                {
                    printe("\nERROR: Invalid index buffer from create atlas [fmcube16] LIMIT_MERGE_STRETCH (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str());
                    success = false;
                }
            }
        }

        // UVATLAS_LIMIT_FACE_STRETCH
        hr = UVAtlasCreate(g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
            0, 0.f, 512, 512, 1.f,
            s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
            UVATLAS_LIMIT_FACE_STRETCH, vb, ib, &facePart, &remap, &maxStretch, &numCharts);
        if (FAILED(hr))
        {
            printe("\nERROR: create atlas [fmcube16] LIMIT_FACE_STRETCH failed (%08X)\n", static_cast<unsigned int>(hr));
            success = false;
        }
        else
        {
            size_t nFaces = ib.size() / (sizeof(uint16_t) * 3);
            if (vb.size() < 26 || vb.size() > 30
                || nFaces != 12
                || facePart.size() != 12
                || remap.size() != vb.size()
                || numCharts < 7 || numCharts > 9)
            {
                printe("\nERROR: Unexpected results from create atlas [fmcube16] LIMIT_FACE_STRETCH\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tface partitions %zu\n\tremap array %zu\n\tmaxStretch %f\n\tnumCharts %zu\n",
                    vb.size(), nFaces, ib.size(), facePart.size(), remap.size(), maxStretch, numCharts);
                success = false;
            }
            else if (!IsValidVertexRemap(reinterpret_cast<const uint16_t*>(ib.data()), 12, remap.data(), vb.size(), true))
            {
                printe("\nERROR: Vertex remap invalid from create atlas [fmcube16] LIMIT_FACE_STRETCH\n");
                success = false;
            }
            else if (!IsValidFacePartition(facePart.data(), 12, numCharts))
            {
                printe("\nERROR: Face partition invalid from create atlas [fmcube16] LIMIT_FACE_STRETCH\n");
                success = false;
            }
            else if (!VerifyVertices(g_fmCubeVerts, 24, vb.data(), remap.data(), vb.size()))
            {
                printe("\nERROR: Vertex data doesn't match remap [fmcube16] LIMIT_FACE_STRETCH\n");
                success = false;
            }
            else
            {
                std::wstring msgs;
                hr = Validate(reinterpret_cast<const uint16_t*>(ib.data()), 12, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs);
                if (FAILED(hr))
                {
                    printe("\nERROR: Invalid index buffer from create atlas [fmcube16] LIMIT_FACE_STRETCH (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str());
                    success = false;
                }
            }
        }
        // maxchartnumber
        hr = UVAtlasCreate( g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                            3, 0.f, 512, 512, 1.f,
                            s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                            UVATLAS_DEFAULT, vb, ib, &facePart, &remap, &maxStretch, &numCharts );
        if (FAILED(hr))
        {
            printe( "\nERROR: create atlas [fmcube16] maxchartnumber failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            size_t nFaces = ib.size() / (sizeof(uint16_t)*3);
            if ( vb.size() < 28 || vb.size() > 32
                 || nFaces != 12
                 || facePart.size() != 12
                 || remap.size() != vb.size()
                 || numCharts != 3 )
            {
                printe( "\nERROR: Unexpected results from create atlas [fmcube16] maxchartnumber\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tface partitions %zu\n\tremap array %zu\n\tmaxStretch %f\n\tnumCharts %zu\n",
                        vb.size(), nFaces, ib.size(), facePart.size(), remap.size(), maxStretch, numCharts );
                success = false;
            }
            else if ( !IsValidVertexRemap( reinterpret_cast<const uint16_t*>( ib.data() ), 12, remap.data(), vb.size(), true ) )
            {
                printe( "\nERROR: Vertex remap invalid from create atlas [fmcube16] maxchartnumber\n" );
                success = false;
            }
            else if ( !IsValidFacePartition( facePart.data(), 12, numCharts ) )
            {
                printe( "\nERROR: Face partition invalid from create atlas [fmcube16] maxchartnumber\n" );
                success = false;
            }
            else if ( !VerifyVertices( g_fmCubeVerts, 24, vb.data(), remap.data(), vb.size() ) )
            {
                printe( "\nERROR: Vertex data doesn't match remap [fmcube16] maxchartnumber\n" );
                success = false;
            }
            else
            {
                std::wstring msgs;
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid index buffer from create atlas [fmcube16] maxchartnumber (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }
            }
        }

        // maxstretch
        hr = UVAtlasCreate( g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                            0, 0.025f, 512, 512, 1.f,
                            s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                            UVATLAS_DEFAULT, vb, ib, &facePart, &remap, &maxStretch, &numCharts );
        if (FAILED(hr))
        {
            printe( "\nERROR: create atlas [fmcube16] maxstretch failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            size_t nFaces = ib.size() / (sizeof(uint16_t)*3);
            if ( vb.size() < 24 || vb.size() > 30
                 || nFaces != 12
                 || facePart.size() != 12
                 || remap.size() != vb.size()
                 || numCharts < 3 || numCharts > 4
                 || maxStretch > 0.025f )
            {
                printe( "\nERROR: Unexpected results from create atlas [fmcube16] maxstretch\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tface partitions %zu\n\tremap array %zu\n\tmaxStretch %f\n\tnumCharts %zu\n",
                        vb.size(), nFaces, ib.size(), facePart.size(), remap.size(), maxStretch, numCharts );
                success = false;
            }
            else if ( !IsValidVertexRemap( reinterpret_cast<const uint16_t*>( ib.data() ), 12, remap.data(), vb.size(), true ) )
            {
                printe( "\nERROR: Vertex remap invalid from create atlas [fmcube16] maxstretch\n" );
                success = false;
            }
            else if ( !IsValidFacePartition( facePart.data(), 12, numCharts ) )
            {
                printe( "\nERROR: Face partition invalid from create atlas [fmcube16] maxstretch\n" );
                success = false;
            }
            else if ( !VerifyVertices( g_fmCubeVerts, 24, vb.data(), remap.data(), vb.size() ) )
            {
                printe( "\nERROR: Vertex data doesn't match remap [fmcube16] maxstretch\n" );
                success = false;
            }
            else
            {
                std::wstring msgs;
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid index buffer from create atlas [fmcube16] maxstretch (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }
            }
        }

        // gutter
        hr = UVAtlasCreate( g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                            0, 0.f, 512, 512, 2.5f,
                            s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                            UVATLAS_DEFAULT, vb, ib, &facePart, &remap, &maxStretch, &numCharts );
        if (FAILED(hr))
        {
            printe( "\nERROR: create atlas [fmcube16] gutter failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            size_t nFaces = ib.size() / (sizeof(uint16_t)*3);
            if ( vb.size() < 26 || vb.size() > 32
                 || nFaces != 12
                 || facePart.size() != 12
                 || remap.size() != vb.size()
                 || numCharts < 4 || numCharts > 6)
            {
                printe( "\nERROR: Unexpected results from create atlas [fmcube16] gutter\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tface partitions %zu\n\tremap array %zu\n\tmaxStretch %f\n\tnumCharts %zu\n",
                        vb.size(), nFaces, ib.size(), facePart.size(), remap.size(), maxStretch, numCharts );
                success = false;
            }
            else if ( !IsValidVertexRemap( reinterpret_cast<const uint16_t*>( ib.data() ), 12, remap.data(), vb.size(), true ) )
            {
                printe( "\nERROR: Vertex remap invalid from create atlas [fmcube16] gutter\n" );
                success = false;
            }
            else if ( !IsValidFacePartition( facePart.data(), 12, numCharts ) )
            {
                printe( "\nERROR: Face partition invalid from create atlas [fmcube16] gutter\n" );
                success = false;
            }
            else if ( !VerifyVertices( g_fmCubeVerts, 24, vb.data(), remap.data(), vb.size() ) )
            {
                printe( "\nERROR: Vertex data doesn't match remap [fmcube16] gutter\n" );
                success = false;
            }
            else
            {
                std::wstring msgs;
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid index buffer from create atlas [fmcube16] gutter (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }
            }
        }

        // false edge adjacency
        hr = UVAtlasCreate( g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                            0, 0.f, 512, 512, 1.f,
                            s_fmCubeAdj, s_feCubeAdj, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                            UVATLAS_DEFAULT, vb, ib, &facePart, &remap, &maxStretch, &numCharts );
        if (FAILED(hr))
        {
            printe( "\nERROR: create atlas [fmcube16] feadj failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            size_t nFaces = ib.size() / (sizeof(uint16_t)*3);
            if ( vb.size() < 26 || vb.size() > 32
                 || nFaces != 12
                 || facePart.size() != 12
                 || remap.size() != vb.size()
                 || numCharts < 4 || numCharts > 6)
            {
                printe( "\nERROR: Unexpected results from create atlas [fmcube16] feadj\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tface partitions %zu\n\tremap array %zu\n\tmaxStretch %f\n\tnumCharts %zu\n",
                        vb.size(), nFaces, ib.size(), facePart.size(), remap.size(), maxStretch, numCharts );
                success = false;
            }
            else if ( !IsValidVertexRemap( reinterpret_cast<const uint16_t*>( ib.data() ), 12, remap.data(), vb.size(), true ) )
            {
                printe( "\nERROR: Vertex remap invalid from create atlas [fmcube16] feadj\n" );
                success = false;
            }
            else if ( !IsValidFacePartition( facePart.data(), 12, numCharts ) )
            {
                printe( "\nERROR: Face partition invalid from create atlas [fmcube16] feadj\n" );
                success = false;
            }
            else if ( !VerifyVertices( g_fmCubeVerts, 24, vb.data(), remap.data(), vb.size() ) )
            {
                printe( "\nERROR: Vertex data doesn't match remap [fmcube16] feadj\n" );
                success = false;
            }
            else
            {
                std::wstring msgs;
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid index buffer from create atlas [fmcube16] feadj (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }
            }
        }

        // IMT array
        hr = UVAtlasCreate( g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                            0, 0.f, 512, 512, 1.f,
                            s_fmCubeAdj, nullptr, s_fmCubeIMT, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                            UVATLAS_DEFAULT, vb, ib, &facePart, &remap, &maxStretch, &numCharts );
        if (FAILED(hr))
        {
            printe( "\nERROR: create atlas [fmcube16] imt failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            size_t nFaces = ib.size() / (sizeof(uint16_t)*3);
            if ( vb.size() != 30
                 || nFaces != 12
                 || facePart.size() != 12
                 || remap.size() != vb.size()
                 || numCharts != 7 )
            {
                printe( "\nERROR: Unexpected results from create atlas [fmcube16] imt\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tface partitions %zu\n\tremap array %zu\n\tmaxStretch %f\n\tnumCharts %zu\n",
                        vb.size(), nFaces, ib.size(), facePart.size(), remap.size(), maxStretch, numCharts );
                success = false;
            }
            else if ( !IsValidVertexRemap( reinterpret_cast<const uint16_t*>( ib.data() ), 12, remap.data(), vb.size(), true ) )
            {
                printe( "\nERROR: Vertex remap invalid from create atlas [fmcube16] imt\n" );
                success = false;
            }
            else if ( !IsValidFacePartition( facePart.data(), 12, numCharts ) )
            {
                printe( "\nERROR: Face partition invalid from create atlas [fmcube16] imt\n" );
                success = false;
            }
            else if ( !VerifyVertices( g_fmCubeVerts, 24, vb.data(), remap.data(), vb.size() ) )
            {
                printe( "\nERROR: Vertex data doesn't match remap [fmcube16] imt\n" );
                success = false;
            }
            else
            {
                std::wstring msgs;
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid index buffer from create atlas [fmcube16] imt (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }
            }
        }

        // facePartitioning nullptr
        hr = UVAtlasCreate( g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                            0, 0.f, 512, 512, 1.f,
                            s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                            UVATLAS_DEFAULT, vb, ib, nullptr, &remap, &maxStretch, &numCharts );
        if (FAILED(hr))
        {
            printe( "\nERROR: create atlas [fmcube16] facePartitioning null failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            size_t nFaces = ib.size() / (sizeof(uint16_t)*3);
            if ( vb.size() < 26 || vb.size() > 32
                 || nFaces != 12
                 || remap.size() != vb.size()
                 || numCharts < 4 || numCharts > 6)
            {
                printe( "\nERROR: Unexpected results from create atlas [fmcube16] facePartitioning null\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tremap array %zu\n\tmaxStretch %f\n\tnumCharts %zu\n",
                        vb.size(), nFaces, ib.size(), remap.size(), maxStretch, numCharts );
                success = false;
            }
            else if ( !IsValidVertexRemap( reinterpret_cast<const uint16_t*>( ib.data() ), 12, remap.data(), vb.size(), true ) )
            {
                printe( "\nERROR: Vertex remap invalid from create atlas [fmcube16] facePartitioning null\n" );
                success = false;
            }
            else if ( !VerifyVertices( g_fmCubeVerts, 24, vb.data(), remap.data(), vb.size() ) )
            {
                printe( "\nERROR: Vertex data doesn't match remap [fmcube16] facePartitioning null\n" );
                success = false;
            }
            else
            {
                std::wstring msgs;
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid index buffer from create atlas [fmcube16] facePartitioning null (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }
            }
        }

        // vertexRemapArray nullptr
        hr = UVAtlasCreate( g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                            0, 0.f, 512, 512, 1.f,
                            s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                            UVATLAS_DEFAULT, vb, ib, &facePart, nullptr, &maxStretch, &numCharts );
        if (FAILED(hr))
        {
            printe( "\nERROR: create atlas [fmcube16] remap null (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            size_t nFaces = ib.size() / (sizeof(uint16_t)*3);
            if ( vb.size() < 26 || vb.size() > 32
                 || nFaces != 12
                 || facePart.size() != 12
                 || numCharts < 4 || numCharts > 6)
            {
                printe( "\nERROR: Unexpected results from create atlas [fmcube16] remap null\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tface partitions %zu\n\tmaxStretch %f\n\tnumCharts %zu\n",
                        vb.size(), nFaces, ib.size(), facePart.size(), maxStretch, numCharts );
                success = false;
            }
            else if ( !IsValidFacePartition( facePart.data(), 12, numCharts ) )
            {
                printe( "\nERROR: Face partition invalid from create atlas [fmcube16] remap null\n" );
                success = false;
            }
            else
            {
                std::wstring msgs;
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid index buffer from create atlas [fmcube16] remap null (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }
            }
        }

        // maxStretchOut nullptr
        hr = UVAtlasCreate( g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                            0, 0.f, 512, 512, 1.f,
                            s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                            UVATLAS_DEFAULT, vb, ib, &facePart, &remap, nullptr, &numCharts );
        if (FAILED(hr))
        {
            printe( "\nERROR: create atlas [fmcube16] maxStretchOut null failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            size_t nFaces = ib.size() / (sizeof(uint16_t)*3);
            if ( vb.size() < 26 || vb.size() > 32
                 || nFaces != 12
                 || facePart.size() != 12
                 || remap.size() != vb.size()
                 || numCharts < 4 || numCharts > 6)
            {
                printe( "\nERROR: Unexpected results from create atlas [fmcube16] maxStretchOut null\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tface partitions %zu\n\tremap array %zu\n\tnumCharts %zu\n",
                        vb.size(), nFaces, ib.size(), facePart.size(), remap.size(), numCharts );
                success = false;
            }
            else if ( !IsValidVertexRemap( reinterpret_cast<const uint16_t*>( ib.data() ), 12, remap.data(), vb.size(), true ) )
            {
                printe( "\nERROR: Vertex remap invalid from create atlas [fmcube16] maxStretchOut null\n" );
                success = false;
            }
            else if ( !VerifyVertices( g_fmCubeVerts, 24, vb.data(), remap.data(), vb.size() ) )
            {
                printe( "\nERROR: Vertex data doesn't match remap [fmcube16] maxStretchOut null\n" );
                success = false;
            }
            else if ( !IsValidFacePartition( facePart.data(), 12, numCharts ) )
            {
                printe( "\nERROR: Face partition invalid from create atlas [fmcube16] maxStretchOut null\n" );
                success = false;
            }
            else
            {
                std::wstring msgs;
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid index buffer from create atlas [fmcube16] maxStretchOut null (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }
            }
        }

        // numChartsOut nullptr
        hr = UVAtlasCreate( g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                            0, 0.f, 512, 512, 1.f,
                            s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                            UVATLAS_DEFAULT, vb, ib, &facePart, &remap, &maxStretch, nullptr );
        if (FAILED(hr))
        {
            printe( "\nERROR: create atlas [fmcube16] numChartsOut null failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            size_t nFaces = ib.size() / (sizeof(uint16_t)*3);
            if ( vb.size() < 26 || vb.size() > 32
                 || nFaces != 12
                 || facePart.size() != 12
                 || remap.size() != vb.size() )
            {
                printe( "\nERROR: Unexpected results from create atlas [fmcube16] numChartsOut null\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tface partitions %zu\n\tremap array %zu\n\tmaxStretch %f\n",
                        vb.size(), nFaces, ib.size(), facePart.size(), remap.size(), maxStretch );
                success = false;
            }
            else if ( !IsValidVertexRemap( reinterpret_cast<const uint16_t*>( ib.data() ), 12, remap.data(), vb.size(), true ) )
            {
                printe( "\nERROR: Vertex remap invalid from create atlas [fmcube16] numChartsOut null\n" );
                success = false;
            }
            else if ( !VerifyVertices( g_fmCubeVerts, 24, vb.data(), remap.data(), vb.size() ) )
            {
                printe( "\nERROR: Vertex data doesn't match remap [fmcube16] numChartsOut\n" );
                success = false;
            }
            else
            {
                std::wstring msgs;
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid index buffer from create atlas [fmcube16] numChartsOut null (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }
            }
        }
    }

    // 32-bit cube
    {
        std::vector<UVAtlasVertex> vb;
        std::vector<uint8_t> ib;
        std::vector<uint32_t> facePart;
        std::vector<uint32_t> remap;
        float maxStretch = 0.f;
        size_t numCharts = 0;
        hr = UVAtlasCreate( g_fmCubeVerts, 24, g_fmCubeIndices32, DXGI_FORMAT_R32_UINT, 12,
                            0, 0.f, 512, 512, 1.f,
                            s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                            UVATLAS_DEFAULT, vb, ib, &facePart, &remap, &maxStretch, &numCharts );
        if (FAILED(hr))
        {
            printe( "\nERROR: create atlas [fmcube32] failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            size_t nFaces = ib.size() / (sizeof(uint32_t)*3);
            if ( vb.size() < 26 || vb.size() > 32
                 || nFaces != 12
                 || facePart.size() != 12
                 || remap.size() != vb.size()
                 || numCharts < 4 || numCharts > 6)
            {
                printe( "\nERROR: Unexpected results from create atlas [fmcube32]\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tface partitions %zu\n\tremap array %zu\n\tmaxStretch %f\n\tnumCharts %zu\n",
                        vb.size(), nFaces, ib.size(), facePart.size(), remap.size(), maxStretch, numCharts );
                success = false;
            }
            else if ( !IsValidVertexRemap( reinterpret_cast<const uint32_t*>( ib.data() ), 12, remap.data(), vb.size(), true ) )
            {
                printe( "\nERROR: Vertex remap invalid from create atlas [fmcube32]\n" );
                success = false;
            }
            else if ( !IsValidFacePartition( facePart.data(), 12, numCharts ) )
            {
                printe( "\nERROR: Face partition invalid from create atlas [fmcube32]\n" );
                success = false;
            }
            else if ( !VerifyVertices( g_fmCubeVerts, 24, vb.data(), remap.data(), vb.size() ) )
            {
                printe( "\nERROR: Vertex data doesn't match remap [fmcube32]\n" );
                success = false;
            }
            else
            {
                std::wstring msgs;
                hr = Validate( reinterpret_cast<const uint32_t*>( ib.data() ), 12, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid index buffer from create atlas [fmcube32] (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }
            }
        }
    }

    return success;
}


//-------------------------------------------------------------------------------------
// UVAtlasPartition
bool Test02()
{
    bool success = true;
    HRESULT hr;

    // invalid args
    #pragma warning(push)
    #pragma warning(disable : 6385 6387)
    {
        std::vector<UVAtlasVertex> vb;
        std::vector<uint8_t> ib;
        std::vector<uint32_t> facePart;
        std::vector<uint32_t> remap;
        std::vector<uint32_t> resultAdj;
        float maxStretch = 0.f;
        size_t numCharts = 0;
        hr = UVAtlasPartition( g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R8G8B8A8_UNORM, 12,
                               0, 0.f,
                               s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                               UVATLAS_DEFAULT, vb, ib, &facePart, &remap, resultAdj, &maxStretch, &numCharts );
        if ( hr != E_INVALIDARG )
        {
            printe( "\nERROR: expected failure for wrong DXGI format\n" );
            success = false;
        }

        hr = UVAtlasPartition( nullptr, 0, nullptr, DXGI_FORMAT_R16_UINT, 0,
                               0, 0.f,
                               s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                               UVATLAS_DEFAULT, vb, ib, &facePart, &remap, resultAdj, &maxStretch, &numCharts );
        if ( hr != E_INVALIDARG )
        {
            printe( "\nERROR: expected failure for missing input\n" );
            success = false;
        }

        hr = UVAtlasPartition( g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                               0, 0.f,
                               nullptr, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                               UVATLAS_DEFAULT, vb, ib, &facePart, &remap, resultAdj, &maxStretch, &numCharts );
        if ( hr != E_INVALIDARG )
        {
            printe( "\nERROR: expected failure for missing adj\n" );
            success = false;
        }

        hr = UVAtlasPartition( g_fmCubeVerts, 0xFFFFFFFF, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                               0, 0.f,
                               s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                               UVATLAS_DEFAULT, vb, ib, &facePart, &remap, resultAdj, &maxStretch, &numCharts );
        if ( hr != E_INVALIDARG )
        {
            printe( "\nERROR: expected failure for too many verts\n" );
            success = false;
        }
    }
    #pragma warning(pop)

    // 16-bit cube
    {
        std::vector<UVAtlasVertex> vb;
        std::vector<uint8_t> ib;
        std::vector<uint32_t> facePart;
        std::vector<uint32_t> remap;
        std::vector<uint32_t> resultAdj;
        float maxStretch = 0.f;
        size_t numCharts = 0;
        hr = UVAtlasPartition( g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                               0, 0.f,
                               s_fmCubeAdj, nullptr, nullptr, nullptr /*one case without callback*/, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                               UVATLAS_DEFAULT, vb, ib, &facePart, &remap, resultAdj, &maxStretch, &numCharts );
        if (FAILED(hr))
        {
            printe( "\nERROR: create partition [fmcube16] failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            size_t nFaces = ib.size() / (sizeof(uint16_t)*3);
            if ( vb.size() < 26 || vb.size() > 32
                 || nFaces != 12
                 || facePart.size() != 12
                 || remap.size() != vb.size()
                 || resultAdj.size() != 3*12
                 || numCharts < 4 || numCharts > 6)
            {
                printe( "\nERROR: Unexpected results from create partition [fmcube16]\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tface partitions %zu\n\tremap array %zu\n\tmaxStretch %f\n\tnumCharts %zu\n",
                        vb.size(), nFaces, ib.size(), facePart.size(), remap.size(), maxStretch, numCharts );
                success = false;
            }
            else if ( !IsValidVertexRemap( reinterpret_cast<const uint16_t*>( ib.data() ), 12, remap.data(), vb.size(), true ) )
            {
                printe( "\nERROR: Vertex remap invalid from create partition [fmcube16]\n" );
                success = false;
            }
            else if ( !IsValidFacePartition( facePart.data(), 12, numCharts ) )
            {
                printe( "\nERROR: Face partition invalid from create partition [fmcube16]\n" );
                success = false;
            }
            else
            {
                std::wstring msgs;
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid index buffer from create partition [fmcube16] (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }

                msgs.clear();
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), resultAdj.data(), VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid result adj from create partition [fmcube16] (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }
            }
        }

        // no clear
        hr = UVAtlasPartition( g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                               0, 0.f,
                               s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                               UVATLAS_DEFAULT, vb, ib, &facePart, &remap, resultAdj, &maxStretch, &numCharts );
        if (FAILED(hr))
        {
            printe( "\nERROR: create partition [fmcube16] (2) failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            size_t nFaces = ib.size() / (sizeof(uint16_t)*3);
            if ( vb.size() < 26 || vb.size() > 32
                 || nFaces != 12
                 || facePart.size() != 12
                 || remap.size() != vb.size()
                 || resultAdj.size() != 3*12
                 || numCharts < 4 || numCharts > 6)
            {
                printe( "\nERROR: Unexpected results from create partition [fmcube16] (2)\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tface partitions %zu\n\tremap array %zu\n\tmaxStretch %f\n\tnumCharts %zu\n",
                        vb.size(), nFaces, ib.size(), facePart.size(), remap.size(), maxStretch, numCharts );
                success = false;
            }
            else if ( !IsValidVertexRemap( reinterpret_cast<const uint16_t*>( ib.data() ), 12, remap.data(), vb.size(), true ) )
            {
                printe( "\nERROR: Vertex remap invalid from create partition [fmcube16] (2)\n" );
                success = false;
            }
            else if ( !IsValidFacePartition( facePart.data(), 12, numCharts ) )
            {
                printe( "\nERROR: Face partition invalid from create partition [fmcube16] (2)\n" );
                success = false;
            }
            else
            {
                std::wstring msgs;
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid index buffer from create partition [fmcube16] (2) (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }
                msgs.clear();
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), resultAdj.data(), VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid result adj from create partition [fmcube16] (2) (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }
            }
        }

        // UVATLAS_GEODESIC_FAST
        hr = UVAtlasPartition( g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                               0, 0.f,
                               s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                               UVATLAS_GEODESIC_FAST, vb, ib, &facePart, &remap, resultAdj, &maxStretch, &numCharts );
        if (FAILED(hr))
        {
            printe( "\nERROR: create partition [fmcube16] FAST failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            size_t nFaces = ib.size() / (sizeof(uint16_t)*3);
            if ( vb.size() != 28
                 || nFaces != 12
                 || facePart.size() != 12
                 || remap.size() != vb.size()
                 || resultAdj.size() != 3*12
                 || numCharts != 7 )
            {
                printe( "\nERROR: Unexpected results from create partition [fmcube16] FAST\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tface partitions %zu\n\tremap array %zu\n\tmaxStretch %f\n\tnumCharts %zu\n",
                        vb.size(), nFaces, ib.size(), facePart.size(), remap.size(), maxStretch, numCharts );
                success = false;
            }
            else if ( !IsValidVertexRemap( reinterpret_cast<const uint16_t*>( ib.data() ), 12, remap.data(), vb.size(), true ) )
            {
                printe( "\nERROR: Vertex remap invalid from create partition [fmcube16] FAST\n" );
                success = false;
            }
            else if ( !IsValidFacePartition( facePart.data(), 12, numCharts ) )
            {
                printe( "\nERROR: Face partition invalid from create partition [fmcube16] FAST\n" );
                success = false;
            }
            else
            {
                std::wstring msgs;
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid index buffer from create partition [fmcube16] FAST (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }

                msgs.clear();
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), resultAdj.data(), VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid result adj from create partition [fmcube16] FAST (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }
            }
        }

        // UVATLAS_GEODESIC_QUALITY
        hr = UVAtlasPartition( g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                               0, 0.f,
                               s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                               UVATLAS_GEODESIC_QUALITY, vb, ib, &facePart, &remap, resultAdj, &maxStretch, &numCharts );
        if (FAILED(hr))
        {
            printe( "\nERROR: create partition [fmcube16] QUALITY failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            size_t nFaces = ib.size() / (sizeof(uint16_t)*3);
            if ( vb.size() < 26 || vb.size() > 32
                 || nFaces != 12
                 || facePart.size() != 12
                 || remap.size() != vb.size()
                 || resultAdj.size() != 3*12
                 || numCharts < 4 || numCharts > 6)
            {
                printe( "\nERROR: Unexpected results from create partition [fmcube16] QUALITY\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tface partitions %zu\n\tremap array %zu\n\tmaxStretch %f\n\tnumCharts %zu\n",
                        vb.size(), nFaces, ib.size(), facePart.size(), remap.size(), maxStretch, numCharts );
                success = false;
            }
            else if ( !IsValidVertexRemap( reinterpret_cast<const uint16_t*>( ib.data() ), 12, remap.data(), vb.size(), true ) )
            {
                printe( "\nERROR: Vertex remap invalid from create partition [fmcube16] QUALITY\n" );
                success = false;
            }
            else if ( !IsValidFacePartition( facePart.data(), 12, numCharts ) )
            {
                printe( "\nERROR: Face partition invalid from create partition [fmcube16] QUALITY\n" );
                success = false;
            }
            else
            {
                std::wstring msgs;
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid index buffer from create partition [fmcube16] QUALITY (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }

                msgs.clear();
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), resultAdj.data(), VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid result adj from create partition [fmcube16] QUALITY (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }
            }
        }

        // UVATLAS_LIMIT_MERGE_STRETCH
        hr = UVAtlasPartition(g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
            0, 0.f,
            s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
            UVATLAS_LIMIT_MERGE_STRETCH, vb, ib, &facePart, &remap, resultAdj, &maxStretch, &numCharts);
        if (FAILED(hr))
        {
            printe("\nERROR: create partition [fmcube16] LIMIT_MERGE_STRETCH failed (%08X)\n", static_cast<unsigned int>(hr));
            success = false;
        }
        else
        {
            size_t nFaces = ib.size() / (sizeof(uint16_t) * 3);
            if (vb.size() < 26 || vb.size() > 32
                || nFaces != 12
                || facePart.size() != 12
                || remap.size() != vb.size()
                || resultAdj.size() != 3 * 12
                || numCharts < 4 || numCharts > 6)
            {
                printe("\nERROR: Unexpected results from create partition [fmcube16] LIMIT_MERGE_STRETCH\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tface partitions %zu\n\tremap array %zu\n\tmaxStretch %f\n\tnumCharts %zu\n",
                    vb.size(), nFaces, ib.size(), facePart.size(), remap.size(), maxStretch, numCharts);
                success = false;
            }
            else if (!IsValidVertexRemap(reinterpret_cast<const uint16_t*>(ib.data()), 12, remap.data(), vb.size(), true))
            {
                printe("\nERROR: Vertex remap invalid from create partition [fmcube16] LIMIT_MERGE_STRETCH\n");
                success = false;
            }
            else if (!IsValidFacePartition(facePart.data(), 12, numCharts))
            {
                printe("\nERROR: Face partition invalid from create partition [fmcube16] LIMIT_MERGE_STRETCH\n");
                success = false;
            }
            else
            {
                std::wstring msgs;
                hr = Validate(reinterpret_cast<const uint16_t*>(ib.data()), 12, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs);
                if (FAILED(hr))
                {
                    printe("\nERROR: Invalid index buffer from create partition [fmcube16] LIMIT_MERGE_STRETCH (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str());
                    success = false;
                }

                msgs.clear();
                hr = Validate(reinterpret_cast<const uint16_t*>(ib.data()), 12, vb.size(), resultAdj.data(), VALIDATE_DEFAULT, &msgs);
                if (FAILED(hr))
                {
                    printe("\nERROR: Invalid result adj from create partition [fmcube16] LIMIT_MERGE_STRETCH (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str());
                    success = false;
                }
            }
        }

        // UVATLAS_LIMIT_FACE_STRETCH
        hr = UVAtlasPartition(g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
            0, 0.f,
            s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
            UVATLAS_LIMIT_FACE_STRETCH, vb, ib, &facePart, &remap, resultAdj, &maxStretch, &numCharts);
        if (FAILED(hr))
        {
            printe("\nERROR: create partition [fmcube16] LIMIT_FACE_STRETCH failed (%08X)\n", static_cast<unsigned int>(hr));
            success = false;
        }
        else
        {
            size_t nFaces = ib.size() / (sizeof(uint16_t) * 3);
            if (vb.size() < 26 || vb.size() > 30
                || nFaces != 12
                || facePart.size() != 12
                || remap.size() != vb.size()
                || resultAdj.size() != 3 * 12
                || numCharts < 7 || numCharts > 9)
            {
                printe("\nERROR: Unexpected results from create partition [fmcube16] LIMIT_FACE_STRETCH\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tface partitions %zu\n\tremap array %zu\n\tmaxStretch %f\n\tnumCharts %zu\n",
                    vb.size(), nFaces, ib.size(), facePart.size(), remap.size(), maxStretch, numCharts);
                success = false;
            }
            else if (!IsValidVertexRemap(reinterpret_cast<const uint16_t*>(ib.data()), 12, remap.data(), vb.size(), true))
            {
                printe("\nERROR: Vertex remap invalid from create partition [fmcube16] LIMIT_FACE_STRETCH\n");
                success = false;
            }
            else if (!IsValidFacePartition(facePart.data(), 12, numCharts))
            {
                printe("\nERROR: Face partition invalid from create partition [fmcube16] LIMIT_FACE_STRETCH\n");
                success = false;
            }
            else
            {
                std::wstring msgs;
                hr = Validate(reinterpret_cast<const uint16_t*>(ib.data()), 12, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs);
                if (FAILED(hr))
                {
                    printe("\nERROR: Invalid index buffer from create partition [fmcube16] LIMIT_FACE_STRETCH (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str());
                    success = false;
                }

                msgs.clear();
                hr = Validate(reinterpret_cast<const uint16_t*>(ib.data()), 12, vb.size(), resultAdj.data(), VALIDATE_DEFAULT, &msgs);
                if (FAILED(hr))
                {
                    printe("\nERROR: Invalid result adj from create partition [fmcube16] LIMIT_FACE_STRETCH (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str());
                    success = false;
                }
            }
        }

        // maxchartnumber
        hr = UVAtlasPartition( g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                               3, 0.f,
                               s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                               UVATLAS_DEFAULT, vb, ib, &facePart, &remap, resultAdj, &maxStretch, &numCharts );
        if (FAILED(hr))
        {
            printe( "\nERROR: create partition [fmcube16] maxchartnumber failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            size_t nFaces = ib.size() / (sizeof(uint16_t)*3);
            if ( vb.size() < 28 || vb.size() > 32
                 || nFaces != 12
                 || facePart.size() != 12
                 || remap.size() != vb.size()
                 || resultAdj.size() != 3*12
                 || numCharts != 3 )
            {
                printe( "\nERROR: Unexpected results from create partition [fmcube16] maxchartnumber\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tface partitions %zu\n\tremap array %zu\n\tmaxStretch %f\n\tnumCharts %zu\n",
                        vb.size(), nFaces, ib.size(), facePart.size(), remap.size(), maxStretch, numCharts );
                success = false;
            }
            else if ( !IsValidVertexRemap( reinterpret_cast<const uint16_t*>( ib.data() ), 12, remap.data(), vb.size(), true ) )
            {
                printe( "\nERROR: Vertex remap invalid from create partition [fmcube16] maxchartnumber\n" );
                success = false;
            }
            else if ( !IsValidFacePartition( facePart.data(), 12, numCharts ) )
            {
                printe( "\nERROR: Face partition invalid from create partition [fmcube16] maxchartnumber\n" );
                success = false;
            }
            else
            {
                std::wstring msgs;
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid index buffer from create partition [fmcube16] maxchartnumber (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }

                msgs.clear();
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), resultAdj.data(), VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid result adj from create partition [fmcube16] maxchartnumber (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }
            }
        }

        // maxstretch
        hr = UVAtlasPartition( g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                               0, 0.025f,
                               s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                               UVATLAS_DEFAULT, vb, ib, &facePart, &remap, resultAdj, &maxStretch, &numCharts );
        if (FAILED(hr))
        {
            printe( "\nERROR: create partition [fmcube16] maxstretch failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            size_t nFaces = ib.size() / (sizeof(uint16_t)*3);
            if ( vb.size() < 24 || vb.size() > 30
                 || nFaces != 12
                 || facePart.size() != 12
                 || remap.size() != vb.size()
                 || resultAdj.size() != 3*12
                 || numCharts < 3 || numCharts > 4
                 || maxStretch > 0.025f )
            {
                printe( "\nERROR: Unexpected results from create partition [fmcube16] maxstretch\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tface partitions %zu\n\tremap array %zu\n\tmaxStretch %f\n\tnumCharts %zu\n",
                        vb.size(), nFaces, ib.size(), facePart.size(), remap.size(), maxStretch, numCharts );
                success = false;
            }
            else if ( !IsValidVertexRemap( reinterpret_cast<const uint16_t*>( ib.data() ), 12, remap.data(), vb.size(), true ) )
            {
                printe( "\nERROR: Vertex remap invalid from create partition [fmcube16] maxstretch\n" );
                success = false;
            }
            else if ( !IsValidFacePartition( facePart.data(), 12, numCharts ) )
            {
                printe( "\nERROR: Face partition invalid from create partition [fmcube16] maxstretch\n" );
                success = false;
            }
            else
            {
                std::wstring msgs;
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid index buffer from create partition [fmcube16] maxstretch (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }

                msgs.clear();
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), resultAdj.data(), VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid result adj from create partition [fmcube16] maxstretch (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }
            }
        }

        // gutter
        hr = UVAtlasPartition( g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                               0, 0.f,
                               s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                               UVATLAS_DEFAULT, vb, ib, &facePart, &remap, resultAdj, &maxStretch, &numCharts );
        if (FAILED(hr))
        {
            printe( "\nERROR: create partition [fmcube16] gutter failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            size_t nFaces = ib.size() / (sizeof(uint16_t)*3);
            if ( vb.size() < 26 || vb.size() > 32
                 || nFaces != 12
                 || facePart.size() != 12
                 || remap.size() != vb.size()
                 || resultAdj.size() != 3*12
                 || numCharts < 4 || numCharts > 6)
            {
                printe( "\nERROR: Unexpected results from create partition [fmcube16] gutter\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tface partitions %zu\n\tremap array %zu\n\tmaxStretch %f\n\tnumCharts %zu\n",
                        vb.size(), nFaces, ib.size(), facePart.size(), remap.size(), maxStretch, numCharts );
                success = false;
            }
            else if ( !IsValidVertexRemap( reinterpret_cast<const uint16_t*>( ib.data() ), 12, remap.data(), vb.size(), true ) )
            {
                printe( "\nERROR: Vertex remap invalid from create partition [fmcube16] gutter\n" );
                success = false;
            }
            else if ( !IsValidFacePartition( facePart.data(), 12, numCharts ) )
            {
                printe( "\nERROR: Face partition invalid from create partition [fmcube16] gutter\n" );
                success = false;
            }
            else
            {
                std::wstring msgs;
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid index buffer from create partition [fmcube16] gutter (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }

                msgs.clear();
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), resultAdj.data(), VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid result adj from create partition [fmcube16] gutter (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }
            }
        }

        // false edge adjacency
        hr = UVAtlasPartition( g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                               0, 0.f,
                               s_fmCubeAdj, s_feCubeAdj, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                               UVATLAS_DEFAULT, vb, ib, &facePart, &remap, resultAdj, &maxStretch, &numCharts );
        if (FAILED(hr))
        {
            printe( "\nERROR: create partition [fmcube16] feadj failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            size_t nFaces = ib.size() / (sizeof(uint16_t)*3);
            if ( vb.size() < 26 || vb.size() > 32
                 || nFaces != 12
                 || facePart.size() != 12
                 || remap.size() != vb.size()
                 || resultAdj.size() != 3*12
                 || numCharts < 4 || numCharts > 6)
            {
                printe( "\nERROR: Unexpected results from create partition [fmcube16] feadj\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tface partitions %zu\n\tremap array %zu\n\tmaxStretch %f\n\tnumCharts %zu\n",
                        vb.size(), nFaces, ib.size(), facePart.size(), remap.size(), maxStretch, numCharts );
                success = false;
            }
            else if ( !IsValidVertexRemap( reinterpret_cast<const uint16_t*>( ib.data() ), 12, remap.data(), vb.size(), true ) )
            {
                printe( "\nERROR: Vertex remap invalid from create partition [fmcube16] feadj\n" );
                success = false;
            }
            else if ( !IsValidFacePartition( facePart.data(), 12, numCharts ) )
            {
                printe( "\nERROR: Face partition invalid from create partition [fmcube16] feadj\n" );
                success = false;
            }
            else
            {
                std::wstring msgs;
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid index buffer from create partition [fmcube16] feadj (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }

                msgs.clear();
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), resultAdj.data(), VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid result adj from create partition [fmcube16] feadj (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }
            }
        }

        // IMT array
        hr = UVAtlasPartition( g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                               0, 0.f,
                               s_fmCubeAdj, nullptr, s_fmCubeIMT, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                               UVATLAS_DEFAULT, vb, ib, &facePart, &remap, resultAdj, &maxStretch, &numCharts );
        if (FAILED(hr))
        {
            printe( "\nERROR: create partition [fmcube16] imt failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            size_t nFaces = ib.size() / (sizeof(uint16_t)*3);
            if ( vb.size() != 30
                 || nFaces != 12
                 || facePart.size() != 12
                 || remap.size() != vb.size()
                 || resultAdj.size() != 3*12
                 || numCharts != 7 )
            {
                printe( "\nERROR: Unexpected results from create partition [fmcube16] imt\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tface partitions %zu\n\tremap array %zu\n\tmaxStretch %f\n\tnumCharts %zu\n",
                        vb.size(), nFaces, ib.size(), facePart.size(), remap.size(), maxStretch, numCharts );
                success = false;
            }
            else if ( !IsValidVertexRemap( reinterpret_cast<const uint16_t*>( ib.data() ), 12, remap.data(), vb.size(), true ) )
            {
                printe( "\nERROR: Vertex remap invalid from create partition [fmcube16] imt\n" );
                success = false;
            }
            else if ( !IsValidFacePartition( facePart.data(), 12, numCharts ) )
            {
                printe( "\nERROR: Face partition invalid from create partition [fmcube16] imt\n" );
                success = false;
            }
            else
            {
                std::wstring msgs;
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid index buffer from create partition [fmcube16] imt (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }

                msgs.clear();
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), resultAdj.data(), VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid result adj from create partition [fmcube16] imt (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }
            }
        }

        // facePartitioning nullptr
        hr = UVAtlasPartition( g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                               0, 0.f,
                               s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                               UVATLAS_DEFAULT, vb, ib, nullptr, &remap, resultAdj, &maxStretch, &numCharts );
        if (FAILED(hr))
        {
            printe( "\nERROR: create partition [fmcube16] facePartitioning null failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            size_t nFaces = ib.size() / (sizeof(uint16_t)*3);
            if ( vb.size() < 26 || vb.size() > 32
                 || nFaces != 12
                 || remap.size() != vb.size()
                 || resultAdj.size() != 3*12
                 || numCharts < 4 || numCharts > 6)
            {
                printe( "\nERROR: Unexpected results from create partition [fmcube16] facePartitioning null\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tremap array %zu\n\tmaxStretch %f\n\tnumCharts %zu\n",
                        vb.size(), nFaces, ib.size(), remap.size(), maxStretch, numCharts );
                success = false;
            }
            else if ( !IsValidVertexRemap( reinterpret_cast<const uint16_t*>( ib.data() ), 12, remap.data(), vb.size(), true ) )
            {
                printe( "\nERROR: Vertex remap invalid from create partition [fmcube16] facePartitioning null\n" );
                success = false;
            }
            else
            {
                std::wstring msgs;
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid index buffer from create partition [fmcube16] facePartitioning null (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }

                msgs.clear();
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), resultAdj.data(), VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid result adj from create partition [fmcube16] facePartitioning null (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }
            }
        }

        // vertexRemapArray nullptr
        hr = UVAtlasPartition( g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                               0, 0.f,
                               s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                               UVATLAS_DEFAULT, vb, ib, &facePart, nullptr, resultAdj, &maxStretch, &numCharts );
        if (FAILED(hr))
        {
            printe( "\nERROR: create partition [fmcube16] remap null (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            size_t nFaces = ib.size() / (sizeof(uint16_t)*3);
            if ( vb.size() < 26 || vb.size() > 32
                 || nFaces != 12
                 || facePart.size() != 12
                 || resultAdj.size() != 3*12
                 || numCharts < 4 || numCharts > 6)
            {
                printe( "\nERROR: Unexpected results from create partition [fmcube16] remap null\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tface partitions %zu\n\tmaxStretch %f\n\tnumCharts %zu\n",
                        vb.size(), nFaces, ib.size(), facePart.size(), maxStretch, numCharts );
                success = false;
            }
            else if ( !IsValidFacePartition( facePart.data(), 12, numCharts ) )
            {
                printe( "\nERROR: Face partition invalid from create partition [fmcube16] remap null\n" );
                success = false;
            }
            else
            {
                std::wstring msgs;
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid index buffer from create partition [fmcube16] remap null (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }

                msgs.clear();
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), resultAdj.data(), VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid result adj from create partition [fmcube16] remap null (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }
            }
        }

        // maxStretchOut nullptr
        hr = UVAtlasPartition( g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                               0, 0.f,
                               s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                               UVATLAS_DEFAULT, vb, ib, &facePart, &remap, resultAdj, nullptr, &numCharts );
        if (FAILED(hr))
        {
            printe( "\nERROR: create partition [fmcube16] maxStretchOut null failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            size_t nFaces = ib.size() / (sizeof(uint16_t)*3);
            if ( vb.size() < 26 || vb.size() > 32
                 || nFaces != 12
                 || facePart.size() != 12
                 || remap.size() != vb.size()
                 || resultAdj.size() != 3*12
                 || numCharts < 4 || numCharts > 6)
            {
                printe( "\nERROR: Unexpected results from create partition [fmcube16] maxStretchOut null\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tface partitions %zu\n\tremap array %zu\n\tnumCharts %zu\n",
                        vb.size(), nFaces, ib.size(), facePart.size(), remap.size(), numCharts );
                success = false;
            }
            else if ( !IsValidVertexRemap( reinterpret_cast<const uint16_t*>( ib.data() ), 12, remap.data(), vb.size(), true ) )
            {
                printe( "\nERROR: Vertex remap invalid from create partition [fmcube16] maxStretchOut null\n" );
                success = false;
            }
            else if ( !IsValidFacePartition( facePart.data(), 12, numCharts ) )
            {
                printe( "\nERROR: Face partition invalid from create partition [fmcube16] maxStretchOut null\n" );
                success = false;
            }
            else
            {
                std::wstring msgs;
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid index buffer from create partition [fmcube16] maxStretchOut null (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }

                msgs.clear();
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), resultAdj.data(), VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid result adj from create partition [fmcube16] maxStretchOut null(%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }
            }
        }

        // numChartsOut nullptr
        hr = UVAtlasPartition( g_fmCubeVerts, 24, g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                               0, 0.f,
                               s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                               UVATLAS_DEFAULT, vb, ib, &facePart, &remap, resultAdj, &maxStretch, nullptr );
        if (FAILED(hr))
        {
            printe( "\nERROR: create partition [fmcube16] numChartsOut null failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            size_t nFaces = ib.size() / (sizeof(uint16_t)*3);
            if ( vb.size() < 26 || vb.size() > 32
                 || nFaces != 12
                 || facePart.size() != 12
                 || remap.size() != vb.size()
                 || resultAdj.size() != 3*12 )
            {
                printe( "\nERROR: Unexpected results from create partition [fmcube16] numChartsOut null\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tface partitions %zu\n\tremap array %zu\n\tmaxStretch %f\n",
                        vb.size(), nFaces, ib.size(), facePart.size(), remap.size(), maxStretch );
                success = false;
            }
            else if ( !IsValidVertexRemap( reinterpret_cast<const uint16_t*>( ib.data() ), 12, remap.data(), vb.size(), true ) )
            {
                printe( "\nERROR: Vertex remap invalid from create partition [fmcube16] numChartsOut null\n" );
                success = false;
            }
            else
            {
                std::wstring msgs;
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid index buffer from create partition [fmcube16] numChartsOut null (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }

                msgs.clear();
                hr = Validate( reinterpret_cast<const uint16_t*>( ib.data() ), 12, vb.size(), resultAdj.data(), VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid result adj from create partition [fmcube16] numChartsOut null (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }
            }
        }
    }

    // 32-bit cube
    {
        std::vector<UVAtlasVertex> vb;
        std::vector<uint8_t> ib;
        std::vector<uint32_t> facePart;
        std::vector<uint32_t> remap;
        std::vector<uint32_t> resultAdj;
        float maxStretch = 0.f;
        size_t numCharts = 0;
        hr = UVAtlasPartition( g_fmCubeVerts, 24, g_fmCubeIndices32, DXGI_FORMAT_R32_UINT, 12,
                               0, 0.f,
                               s_fmCubeAdj, nullptr, nullptr, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY,
                               UVATLAS_DEFAULT, vb, ib, &facePart, &remap, resultAdj, &maxStretch, &numCharts );
        if (FAILED(hr))
        {
            printe( "\nERROR: create partition [fmcube32] failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            size_t nFaces = ib.size() / (sizeof(uint32_t)*3);
            if ( vb.size() < 26 || vb.size() > 32
                 || nFaces != 12
                 || facePart.size() != 12
                 || remap.size() != vb.size()
                 || resultAdj.size() != 3*12
                 || numCharts < 4 || numCharts > 6)
            {
                printe( "\nERROR: Unexpected results from create partition [fmcube32]\n\tverts %zu\n\tfaces %zu (%zu bytes)\n\tface partitions %zu\n\tremap array %zu\n\tmaxStretch %f\n\tnumCharts %zu\n",
                        vb.size(), nFaces, ib.size(), facePart.size(), remap.size(), maxStretch, numCharts );
                success = false;
            }
            else if ( !IsValidVertexRemap( reinterpret_cast<const uint32_t*>( ib.data() ), 12, remap.data(), vb.size(), true ) )
            {
                printe( "\nERROR: Vertex remap invalid from create partition [fmcube32]\n" );
                success = false;
            }
            else if ( !IsValidFacePartition( facePart.data(), 12, numCharts ) )
            {
                printe( "\nERROR: Face partition invalid from create partition [fmcube32]\n" );
                success = false;
            }
            else
            {
                std::wstring msgs;
                hr = Validate( reinterpret_cast<const uint32_t*>( ib.data() ), 12, vb.size(), nullptr, VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid index buffer from create partition [fmcube32] (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }

                msgs.clear();
                hr = Validate( reinterpret_cast<const uint32_t*>( ib.data() ), 12, vb.size(), resultAdj.data(), VALIDATE_DEFAULT, &msgs );
                if ( FAILED(hr))
                {
                    printe( "\nERROR: Invalid result adj from create partition [fmcube32] (%08X):%S\n", static_cast<unsigned int>(hr), msgs.c_str() );
                    success = false;
                }
            }
        }
    }

    return success;
}


//-------------------------------------------------------------------------------------
// UVAtlasPack
bool Test03()
{
    bool success = true;
    HRESULT hr;

    static const UVAtlasVertex s_vb[] =
    {
        { XMFLOAT3(-1.f,  1.f, -1.f ), XMFLOAT2(-258.564087f, 370.440643f) },
        { XMFLOAT3( 1.f,  1.f, -1.f ), XMFLOAT2(441.047699f, 473.121796f) },
        { XMFLOAT3( 1.f,  1.f,  1.f ), XMFLOAT2(543.729126f, -226.489853f) },
        { XMFLOAT3(-1.f,  1.f,  1.f ), XMFLOAT2(-155.882889f, -329.171082f) },
        { XMFLOAT3(-1.f, -1.f, -1.f ), XMFLOAT2(447.213531f, 447.213623f) },
        { XMFLOAT3( 1.f, -1.f, -1.f ), XMFLOAT2(-223.607010f, 223.606918f) },
        { XMFLOAT3( 1.f, -1.f,  1.f ), XMFLOAT2(0.000056f, -447.213531f) },
        { XMFLOAT3(-1.f, -1.f,  1.f ), XMFLOAT2(670.820251f, -223.606720f) },
        { XMFLOAT3(-1.f, -1.f,  1.f ), XMFLOAT2(-855.494629f, -431.852203f) },
        { XMFLOAT3(-1.f, -1.f, -1.f ), XMFLOAT2(-958.176025f, 267.759369f) },
        { XMFLOAT3(-1.f,  1.f, -1.f ), XMFLOAT2(-258.564087f, 370.440643f) },
        { XMFLOAT3(-1.f,  1.f,  1.f ), XMFLOAT2(-155.882889f, -329.171082f) },
        { XMFLOAT3( 1.f, -1.f,  1.f ), XMFLOAT2(0.000056f, -447.213531f) },
        { XMFLOAT3( 1.f, -1.f, -1.f ), XMFLOAT2(-223.607010f, 223.606918f) },
        { XMFLOAT3( 1.f,  1.f, -1.f ), XMFLOAT2(-894.427246f, 0.000029f) },
        { XMFLOAT3( 1.f,  1.f,  1.f ), XMFLOAT2(543.729126f, -226.489853f) },
        { XMFLOAT3(-1.f, -1.f, -1.f ), XMFLOAT2(0.f, 0.f) },
        { XMFLOAT3( 1.f, -1.f, -1.f ), XMFLOAT2(0.f, 707.106812f) },
        { XMFLOAT3( 1.f,  1.f, -1.f ), XMFLOAT2(707.106812f, 707.106812f) },
        { XMFLOAT3(-1.f,  1.f, -1.f ), XMFLOAT2(707.106812f, 0.f) },
        { XMFLOAT3(-1.f, -1.f,  1.f ), XMFLOAT2(0.f, 707.106812f) },
        { XMFLOAT3( 1.f, -1.f,  1.f ), XMFLOAT2(0.f, 0.f) },
        { XMFLOAT3( 1.f,  1.f,  1.f ), XMFLOAT2(707.106812f, 0.f) },
        { XMFLOAT3(-1.f,  1.f,  1.f ), XMFLOAT2(707.106812f, 707.106812f) },
        { XMFLOAT3( 1.f, -1.f,  1.f ), XMFLOAT2(1243.340820f, -123.808609f) },
        { XMFLOAT3( 1.f,  1.f, -1.f ), XMFLOAT2(441.047699f, 473.121796f) },
    };

    static const uint16_t s_ib16[] =
    {
        3, 1, 0,
        2, 1, 3,
        6, 4, 5,
        7, 4, 6,
        11, 9, 8,
        10, 9, 11,
        14, 12, 13,
        15, 24, 25,
        19, 17, 16,
        18, 17, 19,
        22, 20, 21,
        23, 20, 22,
    };

    static const uint32_t s_ib32[] =
    {
        3, 1, 0,
        2, 1, 3,
        6, 4, 5,
        7, 4, 6,
        11, 9, 8,
        10, 9, 11,
        14, 12, 13,
        15, 24, 25,
        19, 17, 16,
        18, 17, 19,
        22, 20, 21,
        23, 20, 22,
    };

    static uint32_t s_resultAdj[] =
    {
        1, uint32_t(-1), 5,
        7, 0, uint32_t(-1),
        3, uint32_t(-1), 6,
        uint32_t(-1), 2, uint32_t(-1),
        5, uint32_t(-1), uint32_t(-1),
        uint32_t(-1), 4, 0,
        uint32_t(-1), 2, uint32_t(-1),
        uint32_t(-1), uint32_t(-1), 1,
        9, uint32_t(-1), uint32_t(-1),
        uint32_t(-1), 8, uint32_t(-1),
        11, uint32_t(-1), uint32_t(-1),
        uint32_t(-1), 10, uint32_t(-1),
    };

    // invalid args
    #pragma warning(push)
    #pragma warning(disable : 6387)
    {
        std::vector<UVAtlasVertex> vb;
        vb.resize( std::size(s_vb ) );
        memcpy( vb.data(), s_vb, sizeof(s_vb) );

        std::vector<uint8_t> ib;
        ib.resize( sizeof(s_ib16) );
        memcpy( ib.data(), s_ib16, sizeof(s_ib16) );

        std::vector<uint32_t> resultAdj;
        resultAdj.resize( 12*3 );
        memcpy( resultAdj.data(), s_resultAdj, sizeof(s_resultAdj) );

        hr = UVAtlasPack( vb, ib, DXGI_FORMAT_R8G8B8A8_UNORM,
                          512, 512, 1.f,
                          resultAdj, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY );
        if ( hr != E_INVALIDARG )
        {
            printe( "\nERROR: expected failure for wrong DXGI format\n" );
            success = false;
        }

        hr = UVAtlasPack( vb, ib, DXGI_FORMAT_R16_UINT,
                          0, 0, 1.f,
                          resultAdj, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY );
        if ( hr != E_INVALIDARG )
        {
            printe( "\nERROR: expected failure for bad width/height\n" );
            success = false;
        }
    }
    #pragma warning(pop)

    // 16-bit cube
    {
        std::vector<UVAtlasVertex> vb;
        vb.resize( std::size(s_vb ) );
        memcpy( vb.data(), s_vb, sizeof(s_vb) );

        std::vector<uint8_t> ib;
        ib.resize( sizeof(s_ib16) );
        memcpy( ib.data(), s_ib16, sizeof(s_ib16) );

        std::vector<uint32_t> resultAdj;
        resultAdj.resize( 12*3 );
        memcpy( resultAdj.data(), s_resultAdj, sizeof(s_resultAdj) );

        hr = UVAtlasPack( vb, ib, DXGI_FORMAT_R16_UINT,
                          512, 512, 1.f,
                          resultAdj, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY );
        if (FAILED(hr))
        {
            printe( "\nERROR: packing [fmcube16] failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
    }

    // 32-bit cube
    {
        std::vector<UVAtlasVertex> vb;
        vb.resize( std::size(s_vb ) );
        memcpy( vb.data(), s_vb, sizeof(s_vb) );

        std::vector<uint8_t> ib;
        ib.resize( sizeof(s_ib32) );
        memcpy( ib.data(), s_ib32, sizeof(s_ib32) );

        std::vector<uint32_t> resultAdj;
        resultAdj.resize( 12*3 );
        memcpy( resultAdj.data(), s_resultAdj, sizeof(s_resultAdj) );

        hr = UVAtlasPack( vb, ib, DXGI_FORMAT_R32_UINT,
                          512, 512, 1.f,
                          resultAdj, UVAtlasCallback, UVATLAS_DEFAULT_CALLBACK_FREQUENCY );
        if (FAILED(hr))
        {
            printe( "\nERROR: packing [fmcube32] failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
    }

    return success;
}
