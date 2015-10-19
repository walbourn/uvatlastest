//-------------------------------------------------------------------------------------
// TestGeometry.h
//
// Simple geometry IBs/VBs
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// http://go.microsoft.com/fwlink/?LinkID=324981
//-------------------------------------------------------------------------------------

namespace TestGeometry
{

//--- Cube ----------------------------------------------------------------------------
extern const __declspec(selectany) DirectX::XMFLOAT3 g_cubeVerts[ 8 ] = 
    {
        DirectX::XMFLOAT3( 0.f, 0.f, 0.f ),
        DirectX::XMFLOAT3( 1.f, 1.f, 0.f ),
        DirectX::XMFLOAT3( 1.f, 0.f, 0.f ),
        DirectX::XMFLOAT3( 0.f, 1.f, 0.f ),
        DirectX::XMFLOAT3( 0.f, 1.f, 1.f ),
        DirectX::XMFLOAT3( 0.f, 0.f, 1.f ),
        DirectX::XMFLOAT3( 1.f, 1.f, 1.f ),
        DirectX::XMFLOAT3( 1.f, 0.f, 1.f ),
    };

extern const __declspec(selectany) uint16_t g_cubeIndices16[ 12*3 ] =
    {
        0, 1, 2,
        0, 3, 1,
        0, 4, 3,
        0, 5, 4,
        3, 6, 1,
        3, 4, 6,
        2, 1, 6,
        2, 6, 7,
        0, 2, 7,
        0, 7, 5,
        5, 7, 6,
        5, 6, 4,
    };

extern const __declspec(selectany) uint32_t g_cubeIndices32[ 12*3 ] =
    {
        0, 1, 2,
        0, 3, 1,
        0, 4, 3,
        0, 5, 4,
        3, 6, 1,
        3, 4, 6,
        2, 1, 6,
        2, 6, 7,
        0, 2, 7,
        0, 7, 5,
        5, 7, 6,
        5, 6, 4,
    };


//--- Face-mapped Cube ----------------------------------------------------------------
extern const __declspec(selectany) DirectX::XMFLOAT3 g_fmCubeVerts[ 24 ] = 
    {
        DirectX::XMFLOAT3( -1.0f, 1.0f, -1.0f ),
        DirectX::XMFLOAT3( 1.0f, 1.0f, -1.0f ),
        DirectX::XMFLOAT3( 1.0f, 1.0f, 1.0f ),
        DirectX::XMFLOAT3( -1.0f, 1.0f, 1.0f ),

        DirectX::XMFLOAT3( -1.0f, -1.0f, -1.0f ),
        DirectX::XMFLOAT3( 1.0f, -1.0f, -1.0f ),
        DirectX::XMFLOAT3( 1.0f, -1.0f, 1.0f ),
        DirectX::XMFLOAT3( -1.0f, -1.0f, 1.0f ),

        DirectX::XMFLOAT3( -1.0f, -1.0f, 1.0f ),
        DirectX::XMFLOAT3( -1.0f, -1.0f, -1.0f ),
        DirectX::XMFLOAT3( -1.0f, 1.0f, -1.0f ),
        DirectX::XMFLOAT3( -1.0f, 1.0f, 1.0f ),

        DirectX::XMFLOAT3( 1.0f, -1.0f, 1.0f ),
        DirectX::XMFLOAT3( 1.0f, -1.0f, -1.0f ),
        DirectX::XMFLOAT3( 1.0f, 1.0f, -1.0f ),
        DirectX::XMFLOAT3( 1.0f, 1.0f, 1.0f ),

        DirectX::XMFLOAT3( -1.0f, -1.0f, -1.0f ),
        DirectX::XMFLOAT3( 1.0f, -1.0f, -1.0f ),
        DirectX::XMFLOAT3( 1.0f, 1.0f, -1.0f ),
        DirectX::XMFLOAT3( -1.0f, 1.0f, -1.0f ),

        DirectX::XMFLOAT3( -1.0f, -1.0f, 1.0f ),
        DirectX::XMFLOAT3( 1.0f, -1.0f, 1.0f ),
        DirectX::XMFLOAT3( 1.0f, 1.0f, 1.0f ),
        DirectX::XMFLOAT3( -1.0f, 1.0f, 1.0f ),
    };

extern const __declspec(selectany) DirectX::XMFLOAT2 g_fmCubeUVs[ 24 ] = 
    {
        DirectX::XMFLOAT2( 1.0f, 0.0f ),
        DirectX::XMFLOAT2( 0.0f, 0.0f ),
        DirectX::XMFLOAT2( 0.0f, 1.0f ),
        DirectX::XMFLOAT2( 1.0f, 1.0f ),

        DirectX::XMFLOAT2( 0.0f, 0.0f ),
        DirectX::XMFLOAT2( 1.0f, 0.0f ),
        DirectX::XMFLOAT2( 1.0f, 1.0f ),
        DirectX::XMFLOAT2( 0.0f, 1.0f ),

        DirectX::XMFLOAT2( 0.0f, 1.0f ),
        DirectX::XMFLOAT2( 1.0f, 1.0f ),
        DirectX::XMFLOAT2( 1.0f, 0.0f ),
        DirectX::XMFLOAT2( 0.0f, 0.0f ),

        DirectX::XMFLOAT2( 1.0f, 1.0f ),
        DirectX::XMFLOAT2( 0.0f, 1.0f ),
        DirectX::XMFLOAT2( 0.0f, 0.0f ),
        DirectX::XMFLOAT2( 1.0f, 0.0f ),

        DirectX::XMFLOAT2( 0.0f, 1.0f ),
        DirectX::XMFLOAT2( 1.0f, 1.0f ),
        DirectX::XMFLOAT2( 1.0f, 0.0f ),
        DirectX::XMFLOAT2( 0.0f, 0.0f ),

        DirectX::XMFLOAT2( 1.0f, 1.0f ),
        DirectX::XMFLOAT2( 0.0f, 1.0f ),
        DirectX::XMFLOAT2( 0.0f, 0.0f ),
        DirectX::XMFLOAT2( 1.0f, 0.0f ),
    };

extern const __declspec(selectany) uint16_t g_fmCubeIndices16[ 12*3 ] =
    {
        3,1,0,
        2,1,3,

        6,4,5,
        7,4,6,

        11,9,8,
        10,9,11,

        14,12,13,
        15,12,14,

        19,17,16,
        18,17,19,

        22,20,21,
        23,20,22
    };

extern const __declspec(selectany) uint32_t g_fmCubeIndices32[ 12*3 ] =
    {
        3,1,0,
        2,1,3,

        6,4,5,
        7,4,6,

        11,9,8,
        10,9,11,

        14,12,13,
        15,12,14,

        19,17,16,
        18,17,19,

        22,20,21,
        23,20,22
    };

extern const __declspec(selectany) uint32_t g_fmCubeAttributes[ 12 ] =
    {
        0, 0,
        1, 1,
        2, 2,
        3, 3,
        4, 4,
        5, 5,
    };


//--- Box (aka cuboid or rectangular parallelepiped) ----------------------------------
extern const __declspec(selectany) DirectX::XMFLOAT3 g_boxVerts[ 8 ] = 
    {
        DirectX::XMFLOAT3( 0.f, 0.f, 0.f ),
        DirectX::XMFLOAT3( 4.f, 2.f, 0.f ),
        DirectX::XMFLOAT3( 4.f, 0.f, 0.f ),
        DirectX::XMFLOAT3( 0.f, 2.f, 0.f ),
        DirectX::XMFLOAT3( 0.f, 2.f, 1.f ),
        DirectX::XMFLOAT3( 0.f, 0.f, 1.f ),
        DirectX::XMFLOAT3( 4.f, 2.f, 1.f ),
        DirectX::XMFLOAT3( 4.f, 0.f, 1.f ),
    };

extern const __declspec(selectany) uint16_t g_boxIndices16[ 12*3 ] =
    {
        0, 1, 2,
        0, 3, 1,
        0, 4, 3,
        0, 5, 4,
        3, 6, 1,
        3, 4, 6,
        2, 1, 6,
        2, 6, 7,
        0, 2, 7,
        0, 7, 5,
        5, 7, 6,
        5, 6, 4,
    };

extern const __declspec(selectany) uint32_t g_boxIndices32[ 12*3 ] =
    {
        0, 1, 2,
        0, 3, 1,
        0, 4, 3,
        0, 5, 4,
        3, 6, 1,
        3, 4, 6,
        2, 1, 6,
        2, 6, 7,
        0, 2, 7,
        0, 7, 5,
        5, 7, 6,
        5, 6, 4,
    };


//--- Tetrahedron ---------------------------------------------------------------------
extern const __declspec(selectany) DirectX::XMFLOAT3 g_tetraVerts[ 4 ] = 
    {
        DirectX::XMFLOAT3( 1.f, 1.f, 1.f ),
        DirectX::XMFLOAT3( 1.f, 2.f, 1.f ),
        DirectX::XMFLOAT3( 2.f, 1.f, 1.f ),
        DirectX::XMFLOAT3( 1.f, 1.f, 2.f ),
    };

extern const __declspec(selectany) uint16_t g_tetraIndices16[ 4*3 ] =
    {
        0, 1, 2,
        1, 2, 0,
        2, 0, 3,
        0, 3, 1,
    };

extern const __declspec(selectany) uint32_t g_tetraIndices32[ 4*3 ] =
    {
        0, 1, 2,
        1, 2, 0,
        2, 0, 3,
        0, 3, 1,
    };


//--- Bowtie --------------------------------------------------------------------------
extern const __declspec(selectany) DirectX::XMFLOAT3 g_bowtieVerts[ 5 ] = 
    {
        DirectX::XMFLOAT3( -1.f, 1.f, 0.f ),
        DirectX::XMFLOAT3( -1.f, -1.f, 0.f ),
        DirectX::XMFLOAT3( 0.f, 0.f, 0.f ),
        DirectX::XMFLOAT3( 1.f, 1.f, 0.f ),
        DirectX::XMFLOAT3( 1.f, -1.f, 0.f ),
    };

extern const __declspec(selectany) uint16_t g_bowtieIndices16[ 2*3 ] =
    {
        0, 1, 2,
        2, 4, 3
    };

extern const __declspec(selectany) uint32_t g_bowtieIndices32[ 2*3 ] =
    {
        0, 1, 2,
        2, 4, 3
    };


//--- Backfacing ----------------------------------------------------------------------
extern const __declspec(selectany) DirectX::XMFLOAT3 g_backfaceVerts[ 3 ] = 
    {
        DirectX::XMFLOAT3( 0.f, 0.f, 0.f ),
        DirectX::XMFLOAT3( 1.f, 0.f, 0.f ),
        DirectX::XMFLOAT3( 1.f, 1.f, 0.f ),
    };

extern const __declspec(selectany) uint16_t g_backfaceIndices16[ 2*3 ] =
    {
        0, 1, 2,
        2, 1, 0,
    };

extern const __declspec(selectany) uint32_t g_backfaceIndices32[ 2*3 ] =
    {
        0, 1, 2,
        2, 1, 0,
    };

};