//-------------------------------------------------------------------------------------
// DirectXTest.cpp
//  
// Copyright (c) Microsoft Corporation. All rights reserved.
//-------------------------------------------------------------------------------------

#include "directxtest.h"
#include "ShapesGenerator.h"

#include <objbase.h>

#include "uvatlas.h"

using namespace DirectX;

//-------------------------------------------------------------------------------------
// Types and globals

typedef bool (*TestFN)();

struct TestInfo
{
    const char *name;
    TestFN func;
};

extern bool Test01();
extern bool Test02();
extern bool Test03();
extern bool Test04();
extern bool Test05();
extern bool Test06();
extern bool Test07();
extern bool Test08();
extern bool Test09();
extern bool Test10();
extern bool Test11();

TestInfo g_Tests[] =
{
    { "UVAtlasCreate", Test01 },
    { "UVAtlasPartition", Test02 },
    { "UVAtlasPack", Test03 },
    { "UVAtlasApplyRemap (no duplicates)", Test09 },
    { "UVAtlasApplyRemap (with duplicates)", Test10 },
    { "UVAtlasComputeIMTFromPerVertexSignal", Test04 },
    { "UVAtlasComputeIMTFromSignal", Test05 },
    { "UVAtlasComputeIMTFromTexture", Test06 },
    { "UVAtlasComputeIMTFromPerTexelSignal", Test07 },
    { "MeshProcess(16)", Test08 },
    { "MeshProcess(32)", Test11 },
};


//-------------------------------------------------------------------------------------
bool RunTests()
{
    UINT nPass = 0;
    UINT nFail = 0;

    for( UINT i=0; i < ( sizeof(g_Tests) / sizeof(TestInfo) ); ++i)
    {
        print("%s: ", g_Tests[i].name );

        if ( g_Tests[i].func() )
        {
            ++nPass;
            print("PASS\n");
        }
        else
        {
            ++nFail;
            print("FAIL\n");
        }
    }

    print("Ran %d tests, %d pass, %d fail\n", nPass+nFail, nPass, nFail);

    return (nFail == 0);
}


//-------------------------------------------------------------------------------------
int __cdecl main()
{
    print("**************************************************************\n");
    print("*** " _DIRECTX_TEST_NAME_ " test\n" );
    print("*** Library Version %03d\n", UVATLAS_VERSION  );
    print("**************************************************************\n");

    if ( !XMVerifyCPUSupport() )
    {
        printe("ERROR: XMVerifyCPUSupport fails on this system, not a supported platform\n");
        return -1;
    }

    HRESULT hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );
    if ( FAILED(hr) )
    {
        printe("ERROR: CoInitializeEx fails (%08X)\n", hr); 
        return -1;
    }

    if ( !RunTests() )
        return -1;
       
    return 0;
}
