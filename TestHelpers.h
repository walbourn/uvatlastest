//--------------------------------------------------------------------------------------
// File: TestHelpers.h
//
// Helper code for test content for IBs and VBs
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// http://go.microsoft.com/fwlink/?LinkID=324981
//--------------------------------------------------------------------------------------

#define NOMINMAX
#include <windows.h>
#include <cguid.h>

#include <algorithm>
#include <memory>

#pragma warning(push)
#pragma warning(disable : 4005)
#include <stdint.h>
#pragma warning(pop)

#include <directxmath.h>
#include <directxcollision.h>


//--------------------------------------------------------------------------------------
enum IB_TEST_TYPE
{
    IB_ZERO,
    IB_IDENTITY,
    IB_REVERSE,
};


template <typename T>
inline std::unique_ptr<T[]> CreateIndexBuffer( size_t count, IB_TEST_TYPE fillType )
{
    std::unique_ptr<T[]> ib( new T[ count ] );

    switch( fillType )
    {
    case IB_IDENTITY:
        for( size_t j = 0; j < count; ++j )
        {
            ib[ j ] = T( j );
        }
        break;

    case IB_REVERSE:
        for( size_t j = 0; j < (count / 3); ++j )
        {
            ib[ j * 3 ] = T( count - (j*3) - 3 );
            ib[ j * 3 + 1 ] = T( count - (j*3) - 2 );
            ib[ j * 3 + 2 ] = T( count - (j*3) - 1 );
        }
        break;

    default:
        memset( ib.get(), 0, sizeof(T) * count );
        break;
    }

    return ib;
}


template <typename T>
inline bool IsTestIBCorrect( const T* ib, size_t count, IB_TEST_TYPE fillType )
{
    if ( !ib || !count )
        return false;

    switch( fillType )
    {
    case IB_IDENTITY:
        for( size_t j = 0; j < count; ++j )
        {
            if ( ib[ j ] != T( j ) )
                return false;
        }
        break;

    case IB_REVERSE:
        for( size_t j = 0; j < (count / 3); ++j )
        {
            if ( ib[ j * 3 ] != T( count - (j*3) - 3 )
                 || ib[ j * 3 + 1 ] != T( count - (j*3) - 2 )
                 || ib[ j * 3 + 2 ] != T( count - (j*3) - 1 ) )
                 return false;
        }
        break;

    default:
        for( size_t j = 0; j < count; ++j )
        {
            if ( ib[ j ] != T( 0 ) )
                return false;
        }
        break;
    }

    return true;
}


//--------------------------------------------------------------------------------------
extern const __declspec(selectany) GUID g_VBGuid = { 0xc129d450, 0x922e, 0x4655, { 0xb0, 0x98, 0xf6, 0x1e, 0xfe, 0xd7, 0xb7, 0x6d } };
extern const __declspec(selectany) GUID g_VBGuid2 = { 0x92f9af91, 0xe870, 0x45f2, { 0x98, 0x8c, 0x29, 0xb4, 0x34, 0x82, 0xc6, 0x14 } };

static_assert ( sizeof(GUID) == 16, "Size mismatch" );

//--------------------------------------------------------------------------------------
inline std::unique_ptr<uint8_t[]> CreateVertexBuffer( size_t stride, size_t count )
{
    std::unique_ptr<uint8_t[]> vb( new uint8_t[ stride * count ] );
    memset( vb.get(), 0, stride * count );
    return vb;
}


enum VB_FILL_TYPE
{
    VB_ZERO,
    VB_IDENTITY,
    VB_REVERSE,
};


inline std::unique_ptr<uint8_t[]> CreateVertexBuffer32( size_t count, VB_FILL_TYPE fillType )
{
    static_assert( sizeof(GUID)*2 == 32, "Mismatch size" );
    std::unique_ptr<uint8_t[]> vb( new uint8_t[ sizeof(GUID) * 2 * count ] );

    switch( fillType )
    {
    case VB_IDENTITY:
        {
            auto ptr = reinterpret_cast<GUID*>( vb.get() );
            auto ptr2 = reinterpret_cast<GUID*>( vb.get() + 16 );
            for( size_t j = 0; j < count; ++j )
            {
                memcpy( ptr, &g_VBGuid, sizeof(GUID) );
                memcpy( ptr2, &g_VBGuid2, sizeof(GUID) );

                ptr->Data1 = DWORD( j );
                ptr += 2;

                ptr2->Data1 = DWORD( j );
                ptr2 += 2;
            }
        }
        break;

    case VB_REVERSE:
        {
            auto ptr = reinterpret_cast<GUID*>( vb.get() );
            auto ptr2 = reinterpret_cast<GUID*>( vb.get() + 16 );
            for( size_t j = 0; j < count; ++j )
            {
                memcpy( ptr, &g_VBGuid, sizeof(GUID) );
                memcpy( ptr2, &g_VBGuid2, sizeof(GUID) );

                DWORD sentinal = DWORD( count - j - 1 );
                ptr->Data1 = sentinal;
                ptr += 2;

                ptr2->Data1 = sentinal;
                ptr2 += 2;
            }
        }
        break;

    default:
        memset( vb.get(), 0, sizeof(GUID) * 2 * count );
        break;
    }

    return vb;
}


inline bool IsTestVBCorrect32( const uint8_t* vert, DWORD sentinal )
{
    auto ptr = reinterpret_cast<const GUID*>( vert );

    if ( memcmp( reinterpret_cast<const BYTE*>(ptr) + sizeof(DWORD),
                 reinterpret_cast<const BYTE*>(&g_VBGuid) + sizeof(DWORD), sizeof(GUID) - sizeof(DWORD) ) != 0 )
        return false;

    if ( ptr->Data1 != sentinal )
        return false;

    ++ptr;

    if ( memcmp( reinterpret_cast<const BYTE*>(ptr) + sizeof(DWORD),
                 reinterpret_cast<const BYTE*>(&g_VBGuid2) + sizeof(DWORD), sizeof(GUID) - sizeof(DWORD) ) != 0 )
        return false;

    if ( ptr->Data1 != sentinal )
        return false;

    return true;
}


inline bool IsTestVBCorrect32( const uint8_t* vb, size_t count, VB_FILL_TYPE fillType )
{
    if ( !vb || !count )
        return false;

    switch( fillType )
    {
    case VB_IDENTITY:
        {
            auto ptr = reinterpret_cast<const GUID*>( vb );
            auto ptr2 = reinterpret_cast<const GUID*>( vb + 16 );
            for( size_t j = 0; j < count; ++j )
            {
                if ( memcmp( reinterpret_cast<const BYTE*>(ptr) + sizeof(DWORD),
                             reinterpret_cast<const BYTE*>(&g_VBGuid) + sizeof(DWORD), sizeof(GUID) - sizeof(DWORD) ) != 0 )
                     return false;

                if ( ptr->Data1 != DWORD( j ) )
                    return false;

                ptr += 2;

                if ( memcmp( reinterpret_cast<const BYTE*>(ptr2) + sizeof(DWORD),
                             reinterpret_cast<const BYTE*>(&g_VBGuid2) + sizeof(DWORD), sizeof(GUID) - sizeof(DWORD) ) != 0 )
                     return false;

                if ( ptr2->Data1 != DWORD( j ) )
                    return false;

                ptr2 += 2;
            }
        }
        break;

    case VB_REVERSE:
        {
            auto ptr = reinterpret_cast<const GUID*>( vb );
            auto ptr2 = reinterpret_cast<const GUID*>( vb + 16 );
            for( size_t j = 0; j < count; ++j )
            {
                DWORD sentinal = DWORD( count - j - 1 );

                if ( memcmp( reinterpret_cast<const BYTE*>(ptr) + sizeof(DWORD),
                             reinterpret_cast<const BYTE*>(&g_VBGuid) + sizeof(DWORD), sizeof(GUID) - sizeof(DWORD) ) != 0 )
                    return false;

                if ( ptr->Data1 != sentinal )
                    return false;

                ptr += 2;

                if ( memcmp( reinterpret_cast<const BYTE*>(ptr2) + sizeof(DWORD),
                             reinterpret_cast<const BYTE*>(&g_VBGuid2) + sizeof(DWORD), sizeof(GUID) - sizeof(DWORD) ) != 0 )
                    return false;

                if ( ptr2->Data1 != sentinal )
                    return false;

                ptr2 += 2;
            }
        }
        break;

    default:
        {
            auto ptr = reinterpret_cast<const GUID*>( vb );
            auto ptr2 = reinterpret_cast<const GUID*>( vb + 16 );
            for( size_t j = 0; j < count; ++j )
            {
                if ( memcmp( reinterpret_cast<const BYTE*>(ptr),
                             reinterpret_cast<const BYTE*>(&GUID_NULL), sizeof(GUID) ) != 0 )
                    return false;

                ptr += 2;

                if ( memcmp( reinterpret_cast<const BYTE*>(ptr2),
                             reinterpret_cast<const BYTE*>(&GUID_NULL), sizeof(GUID) ) != 0 )
                    return false;

                ptr2 += 2;
            }
        }
        break;
    }

    return true;
}


inline std::unique_ptr<uint8_t[]> CreateVertexBuffer16( size_t count, VB_FILL_TYPE fillType )
{
    std::unique_ptr<uint8_t[]> vb( new uint8_t[ sizeof(GUID) * count ] );

    switch( fillType )
    {
    case VB_IDENTITY:
        {
            auto ptr = reinterpret_cast<GUID*>( vb.get() );
            for( size_t j = 0; j < count; ++j )
            {
                memcpy( ptr, &g_VBGuid, sizeof(GUID) );
                ptr->Data1 = DWORD( j );
                ++ptr;
            }
        }
        break;

    case VB_REVERSE:
        {
            auto ptr = reinterpret_cast<GUID*>( vb.get() );
            for( size_t j = 0; j < count; ++j )
            {
                memcpy( ptr, &g_VBGuid, sizeof(GUID) );
                ptr->Data1 = DWORD( count - j - 1 );
                ++ptr;
            }
        }
        break;

    default:
        memset( vb.get(), 0, sizeof(GUID) * count );
        break;
    }

    return vb;
}


inline bool IsTestVBCorrect16( const uint8_t* vert, DWORD sentinal )
{
    auto ptr = reinterpret_cast<const GUID*>( vert );

    if ( memcmp( reinterpret_cast<const BYTE*>(ptr) + sizeof(DWORD),
                 reinterpret_cast<const BYTE*>(&g_VBGuid) + sizeof(DWORD), sizeof(GUID) - sizeof(DWORD) ) != 0 )
        return false;

    if ( ptr->Data1 != sentinal )
        return false;

    return true;
}


inline bool IsTestVBCorrect16( const uint8_t* vb, size_t count, VB_FILL_TYPE fillType )
{
    if ( !vb || !count )
        return false;

    switch( fillType )
    {
    case VB_IDENTITY:
        {
            auto ptr = reinterpret_cast<const GUID*>( vb );
            for( size_t j = 0; j < count; ++j )
            {
                if ( memcmp( reinterpret_cast<const BYTE*>(ptr) + sizeof(DWORD),
                             reinterpret_cast<const BYTE*>(&g_VBGuid) + sizeof(DWORD), sizeof(GUID) - sizeof(DWORD) ) != 0 )
                     return false;

                if ( ptr->Data1 != DWORD( j ) )
                    return false;

                ++ptr;
            }
        }
        break;

    case VB_REVERSE:
        {
            auto ptr = reinterpret_cast<const GUID*>( vb );
            for( size_t j = 0; j < count; ++j )
            {
                if ( memcmp( reinterpret_cast<const BYTE*>(ptr) + sizeof(DWORD),
                             reinterpret_cast<const BYTE*>(&g_VBGuid) + sizeof(DWORD), sizeof(GUID) - sizeof(DWORD) ) != 0 )
                    return false;

                if ( ptr->Data1 != DWORD( count - j - 1 ) )
                    return false;

                ++ptr;
            }
        }
        break;

    default:
        {
            auto ptr = reinterpret_cast<const GUID*>( vb );
            for( size_t j = 0; j < count; ++j )
            {
                if ( memcmp( reinterpret_cast<const BYTE*>(ptr),
                             reinterpret_cast<const BYTE*>(&GUID_NULL), sizeof(GUID) ) != 0 )
                    return false;

                ++ptr;
            }
        }
        break;
    }

    return true;
}


//--------------------------------------------------------------------------------------
extern const __declspec(selectany) DirectX::XMVECTORF32 g_MeshEpsilon = { 1.192092896e-6f, 1.192092896e-6f, 1.192092896e-6f, 1.192092896e-6f };

inline bool CompareArray( const DirectX::XMFLOAT3* a, const DirectX::XMFLOAT3* b, size_t count )
{
    using namespace DirectX;

    for( size_t j = 0; j < count; ++j )
    {
        XMVECTOR v1 = XMLoadFloat3( &a[j] );
        XMVECTOR v2 = XMLoadFloat3( &b[j] );

        if ( !XMVector3NearEqual( v1, v2, g_MeshEpsilon ) )
            return false;
    }

    return true;
}


//--------------------------------------------------------------------------------------
inline bool IsValidPointReps( _In_reads_(nVerts) const uint32_t* pointRep, size_t nVerts )
{
    if ( !pointRep || !nVerts )
        return false;

    for( size_t j = 0; j < nVerts; ++j )
    {
        if ( pointRep[ j ] == uint32_t(-1) )
            continue;

        if ( pointRep[ j ] >= nVerts )
            return false;
    }

    return true;
}


//--------------------------------------------------------------------------------------
template<typename index_t>
inline bool IsValidFaceRemap( _In_reads_(nFaces*3) const index_t* indices, _In_reads_(nFaces) const uint32_t* faceRemap, size_t nFaces )
{
    if ( !indices || !faceRemap || !nFaces )
        return false;

    size_t unused = 0;

    for( size_t j = 0; j < nFaces; ++j )
    {
        if ( faceRemap[j] == uint32_t(-1) )
            ++unused;
        else if ( faceRemap[j] >= nFaces )
            return false;
    }

    std::unique_ptr<uint32_t[]> temp( new (std::nothrow) uint32_t[ nFaces ] );
    if ( !temp )
        return false;

    // check that each 'used' face is used and only used once
    memcpy( temp.get(), faceRemap, sizeof(uint32_t) * nFaces );

    std::sort( temp.get(), temp.get() + nFaces );

    size_t curface = 0;
    size_t expectedUnused = 0;

    for( uint32_t j = 0; j < nFaces; ++j )
    {
        index_t i0 = indices[ j*3 ];
        index_t i1 = indices[ j*3 + 1 ];
        index_t i2 = indices[ j*3 + 2 ];

        if ( i0 == index_t(-1)
             || i1 == index_t(-1)
             || i2 == index_t(-1 ) )
        {
             // ignore unused faces
             ++expectedUnused;
             continue;
        }

        if ( temp[ curface ] != j )
            return false;

        ++curface;
    }

    if ( unused > expectedUnused )
        return false;

    return true;
}


//--------------------------------------------------------------------------------------
template<typename index_t>
inline bool IsValidVertexRemap( _In_reads_(nFaces*3) const index_t* indices, size_t nFaces,
                                _In_reads_(nVerts) const uint32_t* vertexRemap, size_t nVerts, bool allowdups = false )
{
    if ( !indices || !nFaces || !vertexRemap || !nVerts )
        return false;

    size_t unused = 0;

    for( size_t j = 0; j < nVerts; ++j )
    {
        if ( vertexRemap[j] == uint32_t(-1) )
            ++unused;
        else if ( vertexRemap[j] >= nVerts )
            return false;
    }

    std::unique_ptr<uint8_t[]> temp( new (std::nothrow) uint8_t[ ( sizeof(bool) + sizeof(uint32_t) ) * nVerts ] );
    if ( !temp )
        return false;

    auto vused = reinterpret_cast<bool*>( temp.get() );
    memset( vused, 0, sizeof(bool) * nVerts );

    for( size_t j = 0; j < nFaces; ++j )
    {
        index_t i0 = indices[ j*3 ];
        index_t i1 = indices[ j*3 + 1 ];
        index_t i2 = indices[ j*3 + 2 ];

        if ( i0 == index_t(-1)
             || i1 == index_t(-1)
             || i2 == index_t(-1 ) )
        {
            // skip unused faces
            continue;
        }

        if ( i0 < nVerts )  vused[ i0 ] = true;
        if ( i1 < nVerts )  vused[ i1 ] = true;
        if ( i2 < nVerts )  vused[ i2 ] = true;
    }

    size_t expectedUnused = 0;

    for( size_t j = 0; j < nVerts; ++j )
    {
        if ( !vused[ j ] )
            ++expectedUnused;
    }

    if ( unused > expectedUnused )
        return false;

    if ( allowdups )
        return true;

    // check that each 'used' vertex is used and only used once
    auto verts = reinterpret_cast<uint32_t*>( temp.get() + sizeof(bool) * nVerts );
    memcpy( verts, vertexRemap, sizeof(uint32_t) * nVerts );

    std::sort( verts, verts + nVerts );

    size_t curvert = 0;
    for( uint32_t j = 0; j < nVerts; ++j )
    {
        if ( !vused[ j ] )
            continue;

        if ( verts[ curvert ] != j )
            return false;

        ++curvert;
    }

    return true;
}
