//-------------------------------------------------------------------------------------
// imt.cpp
//  
// Copyright (c) Microsoft Corporation. All rights reserved.
//-------------------------------------------------------------------------------------

#include "directxtest.h"
#include "TestHelpers.h"
#include "TestGeometry.h"
#include "ShapesGenerator.h"

#include "uvatlas.h"
#include "directxtex.h"

#include <memory>
#include <new>

using namespace DirectX;
using namespace TestGeometry;

static const XMFLOAT3 g_fmCubeNormals[24] =
{
    XMFLOAT3( 0.f, 1.f, 0.f ),
    XMFLOAT3( 0.f, 1.f, 0.f ),
    XMFLOAT3( 0.f, 1.f, 0.f ),
    XMFLOAT3( 0.f, 1.f, 0.f ),
    XMFLOAT3( 0.f, -1.f, 0.f ),
    XMFLOAT3( 0.f, -1.f, 0.f ),
    XMFLOAT3( 0.f, -1.f, 0.f ),
    XMFLOAT3( 0.f, -1.f, 0.f ),
    XMFLOAT3( -1.f, 0.f, 0.f ),
    XMFLOAT3( -1.f, 0.f, 0.f ),
    XMFLOAT3( -1.f, 0.f, 0.f ),
    XMFLOAT3( -1.f, 0.f, 0.f ),
    XMFLOAT3( 1.f, 0.f, 0.f ),
    XMFLOAT3( 1.f, 0.f, 0.f ),
    XMFLOAT3( 1.f, 0.f, 0.f ),
    XMFLOAT3( 1.f, 0.f, 0.f ),
    XMFLOAT3( 0.f, 0.f, -1.f ),
    XMFLOAT3( 0.f, 0.f, -1.f ),
    XMFLOAT3( 0.f, 0.f, -1.f ),
    XMFLOAT3( 0.f, 0.f, -1.f ),
    XMFLOAT3( 0.f, 0.f, 1.f ),
    XMFLOAT3( 0.f, 0.f, 1.f ),
    XMFLOAT3( 0.f, 0.f, 1.f ),
    XMFLOAT3( 0.f, 0.f, 1.f ),
};


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
static HRESULT LoadTextureF32( const wchar_t* fname, std::unique_ptr<float[]>& tex, size_t& width, size_t& height )
{
    if ( !fname )
        return E_INVALIDARG;

    wchar_t szPath[MAX_PATH] = {};
    DWORD ret = ExpandEnvironmentStringsW( fname, szPath, MAX_PATH );
    if ( !ret || ret > MAX_PATH )  
    {  
        printe( "ERROR: ExpandEnvironmentStrings FAILED\n" );  
        return E_FAIL;  
    } 

    wchar_t ext[_MAX_EXT];
    _wsplitpath_s( szPath, nullptr, 0, nullptr, 0, nullptr, 0, ext, _MAX_EXT );

    ScratchImage image;
    HRESULT hr = E_NOTIMPL;
    if (_wcsicmp(ext, L".dds") == 0)
    {
        TexMetadata metadata;
        hr = LoadFromDDSFile(szPath, DDS_FLAGS_NONE, &metadata, image);

        if ( metadata.arraySize > 1
             || metadata.IsVolumemap() )
        {
            return E_FAIL;
        }

        if ( IsCompressed( metadata.format ) )
        {
            ScratchImage tmp;
            hr = Decompress( image.GetImages(), image.GetImageCount(), metadata, DXGI_FORMAT_UNKNOWN, tmp );
            if ( FAILED(hr) )
                return hr;

            std::swap(image, tmp);
        }
    }
    else if (_wcsicmp(ext, L".tga") == 0)
    {
        hr = LoadFromTGAFile(szPath, nullptr, image);
    }
    else
    {
        hr = LoadFromWICFile(szPath, WIC_FLAGS_NONE, nullptr, image);
    }
    if (FAILED(hr))
        return hr;

    auto img = image.GetImage( 0, 0, 0 );

    tex.reset( new (std::nothrow) float[ img->width * img->height * 4 ] );
    if ( !tex )
        return E_OUTOFMEMORY;

    if ( img->format == DXGI_FORMAT_R32G32B32A32_FLOAT )
    {
        memcpy_s( tex.get(), img->width * img->height * sizeof(float) * 4, img->pixels, img->slicePitch );
    }
    else
    {
        ScratchImage image2;
        hr = Convert( *img, DXGI_FORMAT_R32G32B32A32_FLOAT, TEX_FILTER_DEFAULT, 0.f, image2 );
        if (FAILED(hr))
            return hr;

        auto img2 = image2.GetImage( 0, 0, 0 );

        memcpy_s( tex.get(), img->width * img->height * sizeof(float) * 4, img2->pixels, img2->slicePitch );
    }

    width = img->width;
    height = img->height;

    return S_OK;
}


//-------------------------------------------------------------------------------------
// UVAtlasComputeIMTFromPerVertexSignal
bool Test04()
{
    bool success = true;

    std::unique_ptr<float[]> imtArray( new float[ 12 * 3 ] );

    // invalid args
    #pragma warning(push)
    #pragma warning(disable : 6385 6387)
    memset( imtArray.get(), 0xff, sizeof(float) * 12 * 3 );

    HRESULT hr = UVAtlasComputeIMTFromPerVertexSignal( g_fmCubeVerts, 24,
                                                       g_fmCubeIndices16, DXGI_FORMAT_R8G8B8A8_UNORM, 12,
                                                       &g_fmCubeNormals[0].x, 3, sizeof(XMFLOAT3),
                                                       UVAtlasCallback, imtArray.get() );
    if ( hr != E_INVALIDARG )
    {
        printe( "\nERROR: expected failure for wrong DXGI format\n" );
        success = false;
    }

    hr = UVAtlasComputeIMTFromPerVertexSignal( nullptr, 0,
                                               nullptr, DXGI_FORMAT_R16_UINT, 0,
                                               nullptr, 3, sizeof(XMFLOAT3),
                                               UVAtlasCallback, imtArray.get() );
    if ( hr != E_INVALIDARG )
    {
        printe( "\nERROR: expected failure for missing input\n" );
        success = false;
    }

    hr = UVAtlasComputeIMTFromPerVertexSignal( g_fmCubeVerts, 24,
                                               g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                                               &g_fmCubeNormals[0].x, 3, sizeof(XMFLOAT3),
                                               UVAtlasCallback, nullptr );
    if ( hr != E_INVALIDARG )
    {
        printe( "\nERROR: expected failure for missing output\n" );
        success = false;
    }

    hr = UVAtlasComputeIMTFromPerVertexSignal( g_fmCubeVerts, 0xFFFFFFFF,
                                               g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                                               &g_fmCubeNormals[0].x, 3, sizeof(XMFLOAT3),
                                               UVAtlasCallback, imtArray.get() );
    if ( hr != E_INVALIDARG )
    {
        printe( "\nERROR: expected failure for too many verts\n" );
        success = false;
    }

    hr = UVAtlasComputeIMTFromPerVertexSignal( g_fmCubeVerts, 24,
                                               g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                                               &g_fmCubeNormals[0].x, 4, sizeof(XMFLOAT3),
                                               UVAtlasCallback, imtArray.get() );
    if ( hr != E_INVALIDARG )
    {
        printe( "\nERROR: expected failure for dimension > stride\n" );
        success = false;
    }

    hr = UVAtlasComputeIMTFromPerVertexSignal( g_fmCubeVerts, 24,
                                               g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                                               &g_fmCubeNormals[0].x, 3, 1,
                                               UVAtlasCallback, imtArray.get() );
    if ( hr != E_INVALIDARG )
    {
        printe( "\nERROR: expected failure for bad stride\n" );
        success = false;
    }
    #pragma warning(pop)

    // 16-bit cube
    {
        memset( imtArray.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromPerVertexSignal( g_fmCubeVerts, 24,
                                                   g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                                                   &g_fmCubeNormals[0].x, 3, sizeof(XMFLOAT3),
                                                   nullptr /*one case without callback*/, imtArray.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from normals [fmcube16] failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtArray[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from normals [fmcube16] failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }

        // 2 dimension
        memset( imtArray.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromPerVertexSignal( g_fmCubeVerts, 24,
                                                   g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                                                   &g_fmCubeNormals[0].x, 2, sizeof(XMFLOAT3),
                                                   UVAtlasCallback, imtArray.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from normals (2) [fmcube16] failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtArray[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from normals (2) [fmcube16] failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }

        // 1 dimension
        memset( imtArray.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromPerVertexSignal( g_fmCubeVerts, 24,
                                                   g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                                                   &g_fmCubeNormals[0].x, 1, sizeof(XMFLOAT3),
                                                   UVAtlasCallback, imtArray.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from normals (1) [fmcube16] failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtArray[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from normals (1) [fmcube16] failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }
    }

    // 32-bit cube
    {
        memset( imtArray.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromPerVertexSignal( g_fmCubeVerts, 24,
                                                   g_fmCubeIndices32, DXGI_FORMAT_R32_UINT, 12,
                                                   &g_fmCubeNormals[0].x, 3, sizeof(XMFLOAT3),
                                                   UVAtlasCallback, imtArray.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from normals [fmcube32] failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtArray[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from normals [fmcube32] failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }

        // 2 dimension
        memset( imtArray.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromPerVertexSignal( g_fmCubeVerts, 24,
                                                   g_fmCubeIndices32, DXGI_FORMAT_R32_UINT, 12,
                                                   &g_fmCubeNormals[0].x, 2, sizeof(XMFLOAT3),
                                                   UVAtlasCallback, imtArray.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from normals (2) [fmcube32] failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtArray[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from normals (2) [fmcube32] failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }

        // 1 dimension
        memset( imtArray.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromPerVertexSignal( g_fmCubeVerts, 24,
                                                   g_fmCubeIndices32, DXGI_FORMAT_R32_UINT, 12,
                                                   &g_fmCubeNormals[0].x, 1, sizeof(XMFLOAT3),
                                                   UVAtlasCallback, imtArray.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from normals (1) [fmcube32] failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtArray[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from normals (1) [fmcube32] failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }
    }

    return success;
}


//-------------------------------------------------------------------------------------
// UVAtlasComputeIMTFromSignal

static void *g_expectedUserData = nullptr;

static HRESULT signalFunc(const DirectX::XMFLOAT2 *uv, size_t primitiveID, size_t signalDimension, void* userData, float* signalOut)
{
    if ( !uv || !signalOut )
        return E_INVALIDARG;

    if ( userData != g_expectedUserData )
        return E_ABORT;

    UNREFERENCED_PARAMETER(primitiveID);

    for( size_t j = 0; j < signalDimension; ++j )
    {
        signalOut[j] = (float)(j + 1);
    }

    return S_OK;
}

bool Test05()
{
    bool success = true;

    std::unique_ptr<float[]> imtArray( new float[ 12 * 3 ] );

    // invalid args
    #pragma warning(push)
    #pragma warning(disable : 6385 6387)
    memset( imtArray.get(), 0xff, sizeof(float) * 12 * 3 );

    HRESULT hr = UVAtlasComputeIMTFromSignal( g_fmCubeVerts, g_fmCubeUVs, 24,
                                              g_fmCubeIndices16, DXGI_FORMAT_R8G8B8A8_UNORM, 12,
                                              4, 0.f, signalFunc, nullptr, UVAtlasCallback, imtArray.get() );
    if ( hr != E_INVALIDARG )
    {
        printe( "\nERROR: expected failure for wrong DXGI format\n" );
        success = false;
    }

    hr = UVAtlasComputeIMTFromSignal( g_fmCubeVerts, g_fmCubeUVs, 24,
                                      g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                                      4, 0.f, nullptr, nullptr, UVAtlasCallback, imtArray.get() );
    if ( hr != E_INVALIDARG )
    {
        printe( "\nERROR: expected failure for missing function\n" );
        success = false;
    }

    hr = UVAtlasComputeIMTFromSignal( nullptr, nullptr, 0,
                                      nullptr, DXGI_FORMAT_R16_UINT, 0,
                                      4, 0.f, signalFunc, nullptr, UVAtlasCallback, imtArray.get() );
    if ( hr != E_INVALIDARG )
    {
        printe( "\nERROR: expected failure for missing input\n" );
        success = false;
    }

    hr = UVAtlasComputeIMTFromSignal( g_fmCubeVerts, g_fmCubeUVs, 24,
                                      g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                                      4, 0.f, signalFunc, nullptr, UVAtlasCallback, nullptr );
    if ( hr != E_INVALIDARG )
    {
        printe( "\nERROR: expected failure for missing output\n" );
        success = false;
    }

    hr = UVAtlasComputeIMTFromSignal( g_fmCubeVerts, g_fmCubeUVs, 0xFFFFFFFF,
                                      g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                                      4, 0.f, signalFunc, nullptr, UVAtlasCallback, imtArray.get() );
    if ( hr != E_INVALIDARG )
    {
        printe( "\nERROR: expected failure for too many verts\n" );
        success = false;
    }
    #pragma warning(pop)

    // 16-bit cube
    {
        memset( imtArray.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromSignal( g_fmCubeVerts, g_fmCubeUVs, 24,
                                          g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                                          4, 0.f, signalFunc, nullptr, nullptr /*one case without callback*/, imtArray.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from func [fmcube16] failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtArray[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from func [fmcube16] failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }

        // user data
        memset( imtArray.get(), 0xff, sizeof(float) * 12 * 3 );

        g_expectedUserData = (void*)&g_fmCubeNormals[0];
        hr = UVAtlasComputeIMTFromSignal( g_fmCubeVerts, g_fmCubeUVs, 24,
                                          g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                                          4, 0.f, signalFunc, (void*)&g_fmCubeNormals[0], UVAtlasCallback, imtArray.get() );
        g_expectedUserData = nullptr;
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from func (userdata) [fmcube16] failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtArray[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from func (userdata) [fmcube16] failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }

        // 3 dimension
        memset( imtArray.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromSignal( g_fmCubeVerts, g_fmCubeUVs, 24,
                                          g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                                          3, 0.f, signalFunc, nullptr, UVAtlasCallback, imtArray.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from func (3) [fmcube16] failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtArray[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from func (3) [fmcube16] failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }

        // 2 dimension
        memset( imtArray.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromSignal( g_fmCubeVerts, g_fmCubeUVs, 24,
                                          g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                                          2, 0.f, signalFunc, nullptr, UVAtlasCallback, imtArray.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from func (2) [fmcube16] failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtArray[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from func (2) [fmcube16] failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }

        // 1 dimension
        memset( imtArray.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromSignal( g_fmCubeVerts, g_fmCubeUVs, 24,
                                          g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                                          3, 0.f, signalFunc, nullptr, UVAtlasCallback, imtArray.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from func (1) [fmcube16] failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtArray[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from func (1) [fmcube16] failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }
    }

    // 32-bit cube
    {
        memset( imtArray.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromSignal( g_fmCubeVerts, g_fmCubeUVs, 24,
                                          g_fmCubeIndices32, DXGI_FORMAT_R32_UINT, 12,
                                          4, 0.f, signalFunc, nullptr, UVAtlasCallback, imtArray.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from func [fmcube32] failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtArray[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from func [fmcube32] failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }

        // user data
        memset( imtArray.get(), 0xff, sizeof(float) * 12 * 3 );

        g_expectedUserData = (void*)&g_fmCubeNormals[0];
        hr = UVAtlasComputeIMTFromSignal( g_fmCubeVerts, g_fmCubeUVs, 24,
                                          g_fmCubeIndices32, DXGI_FORMAT_R32_UINT, 12,
                                          4, 0.f, signalFunc, (void*)&g_fmCubeNormals[0], UVAtlasCallback, imtArray.get() );
        g_expectedUserData = nullptr;
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from func (userdata) [fmcube32] failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtArray[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from func (userdata) [fmcube32] failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }

        // 3 dimension
        memset( imtArray.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromSignal( g_fmCubeVerts, g_fmCubeUVs, 24,
                                          g_fmCubeIndices32, DXGI_FORMAT_R32_UINT, 12,
                                          3, 0.f, signalFunc, nullptr, UVAtlasCallback, imtArray.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from func (3) [fmcube32] failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtArray[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from func (3) [fmcube32] failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }

        // 2 dimension
        memset( imtArray.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromSignal( g_fmCubeVerts, g_fmCubeUVs, 24,
                                          g_fmCubeIndices32, DXGI_FORMAT_R32_UINT, 12,
                                          2, 0.f, signalFunc, nullptr, UVAtlasCallback, imtArray.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from func (2) [fmcube32] failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtArray[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from func (2) [fmcube32] failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }

        // 1 dimension
        memset( imtArray.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromSignal( g_fmCubeVerts, g_fmCubeUVs, 24,
                                          g_fmCubeIndices32, DXGI_FORMAT_R32_UINT, 12,
                                          3, 0.f, signalFunc, nullptr, UVAtlasCallback, imtArray.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from func (1) [fmcube32] failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtArray[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from func (1) [fmcube32] failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }
    }

    return success;
}


//-------------------------------------------------------------------------------------
// UVAtlasComputeIMTFromTexture
bool Test06()
{
    bool success = true;

    size_t texDefaultW, texDefaultH;
    std::unique_ptr<float[]> texDefault;
    HRESULT hr = LoadTextureF32( TEX_MEDIA_PATH L"reftexture.dds", texDefault, texDefaultW, texDefaultH );
    if ( FAILED(hr))
    {
        printe( "\nERROR: Failed loading 'reftexture.dds' (%08X)\n", static_cast<unsigned int>(hr) );
        return false;
    }

    std::unique_ptr<float[]> imtArray( new float[ 12 * 3 ] );

    // invalid args
    #pragma warning(push)
    #pragma warning(disable : 6385 6387)
    memset( imtArray.get(), 0xff, sizeof(float) * 12 * 3 );

    hr = UVAtlasComputeIMTFromTexture( g_fmCubeVerts, g_fmCubeUVs, 24,
                                       g_fmCubeIndices16, DXGI_FORMAT_R8G8B8A8_UNORM, 12,
                                       texDefault.get(), texDefaultW, texDefaultH,
                                       UVATLAS_IMT_DEFAULT, UVAtlasCallback, imtArray.get() );
    if ( hr != E_INVALIDARG )
    {
        printe( "\nERROR: expected failure for wrong DXGI format\n" );
        success = false;
    }

    hr = UVAtlasComputeIMTFromTexture( nullptr, nullptr, 0,
                                       nullptr, DXGI_FORMAT_R16_UINT, 0,
                                       nullptr, 0, 0,
                                       UVATLAS_IMT_DEFAULT, UVAtlasCallback, imtArray.get() );
    if ( hr != E_INVALIDARG )
    {
        printe( "\nERROR: expected failure for missing input\n" );
        success = false;
    }

    hr = UVAtlasComputeIMTFromTexture( g_fmCubeVerts, g_fmCubeUVs, 24,
                                       g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                                       texDefault.get(), texDefaultW, texDefaultH,
                                       UVATLAS_IMT_DEFAULT, UVAtlasCallback, nullptr );
    if ( hr != E_INVALIDARG )
    {
        printe( "\nERROR: expected failure for missing output\n" );
        success = false;
    }

    hr = UVAtlasComputeIMTFromTexture( g_fmCubeVerts, g_fmCubeUVs, 0xFFFFFFFF,
                                       g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                                       texDefault.get(), texDefaultW, texDefaultH,
                                       UVATLAS_IMT_DEFAULT, UVAtlasCallback, imtArray.get() );
    if ( hr != E_INVALIDARG )
    {
        printe( "\nERROR: expected failure for too many verts\n" );
        success = false;
    }
    #pragma warning(pop)

    // 16-bit cube
    {
        memset( imtArray.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromTexture( g_fmCubeVerts, g_fmCubeUVs, 24,
                                           g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                                           texDefault.get(), texDefaultW, texDefaultH,
                                           UVATLAS_IMT_DEFAULT, nullptr /*one case without callback*/, imtArray.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from 'refexture.dds' [fmcube16] failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtArray[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from 'refexture.dds' [fmcube16] failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }

        // Wrap U
        std::unique_ptr<float[]> imtWrap( new float[ 12 * 3 ] );

        memset( imtWrap.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromTexture( g_fmCubeVerts, g_fmCubeUVs, 24,
                                           g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                                           texDefault.get(), texDefaultW, texDefaultH,
                                           UVATLAS_IMT_WRAP_U, UVAtlasCallback, imtWrap.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from 'refexture.dds' [fmcube16] wrap u failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtWrap[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from 'refexture.dds' [fmcube16] wrap u failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }

        // Wrap V
        memset( imtWrap.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromTexture( g_fmCubeVerts, g_fmCubeUVs, 24,
                                           g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                                           texDefault.get(), texDefaultW, texDefaultH,
                                           UVATLAS_IMT_WRAP_V, UVAtlasCallback, imtWrap.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from 'refexture.dds' [fmcube16] wrap v failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtWrap[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from 'refexture.dds' [fmcube16] wrap v failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }

        // Wrap UV
        memset( imtWrap.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromTexture( g_fmCubeVerts, g_fmCubeUVs, 24,
                                           g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                                           texDefault.get(), texDefaultW, texDefaultH,
                                           UVATLAS_IMT_WRAP_UV, UVAtlasCallback, imtWrap.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from 'refexture.dds' [fmcube16] wrap uv failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtWrap[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from 'refexture.dds' [fmcube16] wrap uv failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }
    }

    // 32-bit cube
    {
        memset( imtArray.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromTexture( g_fmCubeVerts, g_fmCubeUVs, 24,
                                           g_fmCubeIndices32, DXGI_FORMAT_R32_UINT, 12,
                                           texDefault.get(), texDefaultW, texDefaultH,
                                           UVATLAS_IMT_DEFAULT, UVAtlasCallback, imtArray.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from 'refexture.dds' [fmcube32] failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtArray[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from 'refexture.dds' [fmcube32] failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }

        // Wrap U
        std::unique_ptr<float[]> imtWrap( new float[ 12 * 3 ] );

        memset( imtWrap.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromTexture( g_fmCubeVerts, g_fmCubeUVs, 24,
                                           g_fmCubeIndices32, DXGI_FORMAT_R32_UINT, 12,
                                           texDefault.get(), texDefaultW, texDefaultH,
                                           UVATLAS_IMT_WRAP_U, UVAtlasCallback, imtWrap.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from 'refexture.dds' [fmcube32] wrap u failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtWrap[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from 'refexture.dds' [fmcube32] wrap u failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }

        // Wrap V
        memset( imtWrap.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromTexture( g_fmCubeVerts, g_fmCubeUVs, 24,
                                           g_fmCubeIndices32, DXGI_FORMAT_R32_UINT, 12,
                                           texDefault.get(), texDefaultW, texDefaultH,
                                           UVATLAS_IMT_WRAP_V, UVAtlasCallback, imtWrap.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from 'refexture.dds' [fmcube32] wrap v failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtWrap[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from 'refexture.dds' [fmcube32] wrap v failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }

        // Wrap UV
        memset( imtWrap.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromTexture( g_fmCubeVerts, g_fmCubeUVs, 24,
                                           g_fmCubeIndices32, DXGI_FORMAT_R32_UINT, 12,
                                           texDefault.get(), texDefaultW, texDefaultH,
                                           UVATLAS_IMT_WRAP_UV, UVAtlasCallback, imtWrap.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from 'refexture.dds' [fmcube32] wrap uv failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtWrap[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from 'refexture.dds' [fmcube32] wrap uv failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }
    }

    return success;
}


//-------------------------------------------------------------------------------------
// UVAtlasComputeIMTFromPerTexelSignal
bool Test07()
{
    bool success = true;

    size_t texDefaultW, texDefaultH;
    std::unique_ptr<float[]> texDefault;
    HRESULT hr = LoadTextureF32( TEX_MEDIA_PATH L"reftexture.dds", texDefault, texDefaultW, texDefaultH );
    if ( FAILED(hr))
    {
        printe( "\nERROR: Failed loading 'reftexture.dds' (%08X)\n", static_cast<unsigned int>(hr) );
        return false;
    }

    std::unique_ptr<float[]> imtArray( new float[ 12 * 3 ] );
    
    // invalid args
    #pragma warning(push)
    #pragma warning(disable : 6385 6387)
    memset( imtArray.get(), 0xff, sizeof(float) * 12 * 3 );

    hr = UVAtlasComputeIMTFromPerTexelSignal( g_fmCubeVerts, g_fmCubeUVs, 24,
                                              g_fmCubeIndices16, DXGI_FORMAT_R8G8B8A8_UNORM, 12,
                                              texDefault.get(), texDefaultW, texDefaultH, 4, 4,
                                              UVATLAS_IMT_DEFAULT, UVAtlasCallback, imtArray.get() );
    if ( hr != E_INVALIDARG )
    {
        printe( "\nERROR: expected failure for wrong DXGI format\n" );
        success = false;
    }

    hr = UVAtlasComputeIMTFromPerTexelSignal( g_fmCubeVerts, g_fmCubeUVs, 24,
                                              g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                                              texDefault.get(), texDefaultW, texDefaultH, 5, 4,
                                              UVATLAS_IMT_DEFAULT, UVAtlasCallback, imtArray.get() );
    if ( hr != E_INVALIDARG )
    {
        printe( "\nERROR: expected failure for dimension > component \n" );
        success = false;
    }

    hr = UVAtlasComputeIMTFromPerTexelSignal( nullptr, nullptr, 0,
                                              nullptr, DXGI_FORMAT_R16_UINT, 0,
                                              nullptr, 0, 0, 0, 0,
                                              UVATLAS_IMT_DEFAULT, UVAtlasCallback, imtArray.get() );

    if ( hr != E_INVALIDARG )
    {
        printe( "\nERROR: expected failure for missing input\n" );
        success = false;
    }

    hr = UVAtlasComputeIMTFromPerTexelSignal( g_fmCubeVerts, g_fmCubeUVs, 24,
                                              g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                                              texDefault.get(), texDefaultW, texDefaultH, 4, 4,
                                              UVATLAS_IMT_DEFAULT, UVAtlasCallback, nullptr );
    if ( hr != E_INVALIDARG )
    {
        printe( "\nERROR: expected failure for missing output\n" );
        success = false;
    }

    hr = UVAtlasComputeIMTFromPerTexelSignal( g_fmCubeVerts, g_fmCubeUVs, 0xFFFFFFFF,
                                              g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                                              texDefault.get(), texDefaultW, texDefaultH, 4, 4,
                                              UVATLAS_IMT_DEFAULT, UVAtlasCallback, imtArray.get() );
    if ( hr != E_INVALIDARG )
    {
        printe( "\nERROR: expected failure for too many verts\n" );
        success = false;
    }
    #pragma warning(pop)

    // 16-bit cube
    {
        memset( imtArray.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromPerTexelSignal( g_fmCubeVerts, g_fmCubeUVs, 24,
                                                  g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                                                  texDefault.get(), texDefaultW, texDefaultH, 4, 4,
                                                  UVATLAS_IMT_DEFAULT, nullptr /*one case without callback*/, imtArray.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from 'refexture.dds' [fmcube16] failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtArray[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from 'refexture.dds' [fmcube16] failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }

        // 3 dimension
        memset( imtArray.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromPerTexelSignal( g_fmCubeVerts, g_fmCubeUVs, 24,
                                                  g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                                                  texDefault.get(), texDefaultW, texDefaultH, 3, 4,
                                                  UVATLAS_IMT_DEFAULT, UVAtlasCallback, imtArray.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from 'refexture.dds' (3) [fmcube16] failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtArray[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from 'refexture.dds' (3) [fmcube16] failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }

        // 2 dimension
        memset( imtArray.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromPerTexelSignal( g_fmCubeVerts, g_fmCubeUVs, 24,
                                                  g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                                                  texDefault.get(), texDefaultW, texDefaultH, 2, 4,
                                                  UVATLAS_IMT_DEFAULT, UVAtlasCallback, imtArray.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from 'refexture.dds' (2) [fmcube16] failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtArray[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from 'refexture.dds' (2) [fmcube16] failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }

        // 1 dimension
        memset( imtArray.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromPerTexelSignal( g_fmCubeVerts, g_fmCubeUVs, 24,
                                                  g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                                                  texDefault.get(), texDefaultW, texDefaultH, 1, 4,
                                                  UVATLAS_IMT_DEFAULT, UVAtlasCallback, imtArray.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from 'refexture.dds' (1) [fmcube16] failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtArray[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from 'refexture.dds' (1) [fmcube16] failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }

        // Wrap U
        std::unique_ptr<float[]> imtWrap( new float[ 12 * 3 ] );

        memset( imtWrap.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromPerTexelSignal( g_fmCubeVerts, g_fmCubeUVs, 24,
                                                  g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                                                  texDefault.get(), texDefaultW, texDefaultH, 4, 4,
                                                  UVATLAS_IMT_WRAP_U, UVAtlasCallback, imtWrap.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from 'refexture.dds' [fmcube16] wrap u failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtWrap[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from 'refexture.dds' [fmcube16] wrap u failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }

        // Wrap V
        memset( imtWrap.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromPerTexelSignal( g_fmCubeVerts, g_fmCubeUVs, 24,
                                                  g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                                                  texDefault.get(), texDefaultW, texDefaultH, 4, 4,
                                                  UVATLAS_IMT_WRAP_V, UVAtlasCallback, imtWrap.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from 'refexture.dds' [fmcube16] wrap v failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtWrap[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from 'refexture.dds' [fmcube16] wrap v failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }

        // Wrap UV
        memset( imtWrap.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromPerTexelSignal( g_fmCubeVerts, g_fmCubeUVs, 24,
                                                  g_fmCubeIndices16, DXGI_FORMAT_R16_UINT, 12,
                                                  texDefault.get(), texDefaultW, texDefaultH, 4, 4,
                                                  UVATLAS_IMT_WRAP_UV, UVAtlasCallback, imtWrap.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from 'refexture.dds' [fmcube16] wrap uv failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtWrap[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from 'refexture.dds' [fmcube16] wrap uv failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }
    }

    // 32-bit cube
    {
        memset( imtArray.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromPerTexelSignal( g_fmCubeVerts, g_fmCubeUVs, 24,
                                                  g_fmCubeIndices32, DXGI_FORMAT_R32_UINT, 12,
                                                  texDefault.get(), texDefaultW, texDefaultH, 4, 4,
                                                  UVATLAS_IMT_DEFAULT, UVAtlasCallback, imtArray.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from 'refexture.dds' [fmcube32] failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtArray[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from 'refexture.dds' [fmcube32] failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }

        // 3 dimension
        memset( imtArray.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromPerTexelSignal( g_fmCubeVerts, g_fmCubeUVs, 24,
                                                  g_fmCubeIndices32, DXGI_FORMAT_R32_UINT, 12,
                                                  texDefault.get(), texDefaultW, texDefaultH, 3, 4,
                                                  UVATLAS_IMT_DEFAULT, UVAtlasCallback, imtArray.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from 'refexture.dds' (3) [fmcube32] failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtArray[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from 'refexture.dds' (3) [fmcube32] failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }

        // 2 dimension
        memset( imtArray.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromPerTexelSignal( g_fmCubeVerts, g_fmCubeUVs, 24,
                                                  g_fmCubeIndices32, DXGI_FORMAT_R32_UINT, 12,
                                                  texDefault.get(), texDefaultW, texDefaultH, 2, 4,
                                                  UVATLAS_IMT_DEFAULT, UVAtlasCallback, imtArray.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from 'refexture.dds' (2) [fmcube32] failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtArray[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from 'refexture.dds' (2) [fmcube32] failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }

        // 1 dimension
        memset( imtArray.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromPerTexelSignal( g_fmCubeVerts, g_fmCubeUVs, 24,
                                                  g_fmCubeIndices32, DXGI_FORMAT_R32_UINT, 12,
                                                  texDefault.get(), texDefaultW, texDefaultH, 1, 4,
                                                  UVATLAS_IMT_DEFAULT, UVAtlasCallback, imtArray.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from 'refexture.dds' (1) [fmcube32] failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtArray[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from 'refexture.dds' (1) [fmcube32] failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }

        // Wrap U
        std::unique_ptr<float[]> imtWrap( new float[ 12 * 3 ] );

        memset( imtWrap.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromPerTexelSignal( g_fmCubeVerts, g_fmCubeUVs, 24,
                                                  g_fmCubeIndices32, DXGI_FORMAT_R32_UINT, 12,
                                                  texDefault.get(), texDefaultW, texDefaultH, 4, 4,
                                                  UVATLAS_IMT_WRAP_U, UVAtlasCallback, imtWrap.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from 'refexture.dds' [fmcube32] wrap u failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtWrap[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from 'refexture.dds' [fmcube32] wrap u failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }

        // Wrap V
        memset( imtWrap.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromPerTexelSignal( g_fmCubeVerts, g_fmCubeUVs, 24,
                                                  g_fmCubeIndices32, DXGI_FORMAT_R32_UINT, 12,
                                                  texDefault.get(), texDefaultW, texDefaultH, 4, 4,
                                                  UVATLAS_IMT_WRAP_V, UVAtlasCallback, imtWrap.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from 'refexture.dds' [fmcube32] wrap v failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtWrap[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from 'refexture.dds' [fmcube32] wrap v failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }

        // Wrap UV
        memset( imtWrap.get(), 0xff, sizeof(float) * 12 * 3 );

        hr = UVAtlasComputeIMTFromPerTexelSignal( g_fmCubeVerts, g_fmCubeUVs, 24,
                                                  g_fmCubeIndices32, DXGI_FORMAT_R32_UINT, 12,
                                                  texDefault.get(), texDefaultW, texDefaultH, 4, 4,
                                                  UVATLAS_IMT_WRAP_UV, UVAtlasCallback, imtWrap.get() );
        if ( FAILED(hr) )
        {
            printe( "\nERROR: imt from 'refexture.dds' [fmcube32] wrap uv failed (%08X)\n", static_cast<unsigned int>(hr) );
            success = false;
        }
        else
        {
            bool uninit = false;
            for( size_t j = 0; j < 12 * 3; ++j )
            {
                if ( *reinterpret_cast<const uint32_t*>( &imtWrap[j] ) == 0xffffffff )
                {
                    uninit = true;
                }
            }

            if ( uninit )
            {
                printe( "\nERROR: imt from 'refexture.dds' [fmcube32] wrap uv failed by not writing all output\n" );
                success = false;
            }

            // TODO - validate output beyond just initialization
        }
    }

    return success;
}
