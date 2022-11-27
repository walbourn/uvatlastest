//--------------------------------------------------------------------------------------
// File: ShapesGenerator.h
//
// Code for creating common shapes (based on DirectXTK's GeometricPrimitive code)
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkID=324981
//--------------------------------------------------------------------------------------

#pragma once

#include <algorithm>
#include <vector>

#include <cstdint>

#define _XM_NO_XMVECTOR_OVERLOADS_
#include <DirectXMath.h>
#include <DirectXCollision.h>

template<typename T>
class ShapesGenerator
{
public:
    using index_t = T;

    struct Vertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 normal;
        DirectX::XMFLOAT2 textureCoordinate;

        Vertex() {}
        Vertex( DirectX::XMFLOAT3 const& position, DirectX::XMFLOAT3 const& normal, DirectX::XMFLOAT2 const& textureCoordinate ) :
            position(position),
            normal(normal),
            textureCoordinate(textureCoordinate) {}

        Vertex(DirectX::FXMVECTOR position, DirectX::FXMVECTOR normal, DirectX::FXMVECTOR textureCoordinate)
        {
            DirectX::XMStoreFloat3(&this->position, position);
            DirectX::XMStoreFloat3(&this->normal, normal);
            DirectX::XMStoreFloat2(&this->textureCoordinate, textureCoordinate);
        }
    };

    static void CreateCube( std::vector<index_t>& indices, std::vector<Vertex>& vertices, float size, bool rhcoords )
    {
        using namespace DirectX;

        indices.clear();
        vertices.clear();

        // A cube has six faces, each one pointing in a different direction.
        const int FaceCount = 6;

        static const XMVECTORF32 faceNormals[FaceCount] =
        {
            {  0,  0,  1 },
            {  0,  0, -1 },
            {  1,  0,  0 },
            { -1,  0,  0 },
            {  0,  1,  0 },
            {  0, -1,  0 },
        };

        static const XMVECTORF32 textureCoordinates[4] =
        {
            { 1, 0 },
            { 1, 1 },
            { 0, 1 },
            { 0, 0 },
        };

        XMVECTOR tsize = XMVectorReplicate(size);
        tsize = XMVectorDivide(tsize, g_XMTwo);

        // Create each face in turn.
        for (int i = 0; i < FaceCount; i++)
        {
            XMVECTOR normal = faceNormals[i];

            // Get two vectors perpendicular both to the face normal and to each other.
            XMVECTOR basis = (i >= 4) ? g_XMIdentityR2 : g_XMIdentityR1;

            XMVECTOR side1 = XMVector3Cross(normal, basis);
            XMVECTOR side2 = XMVector3Cross(normal, side1);

            // Six indices (two triangles) per face.
            size_t vbase = vertices.size();
            indices.push_back( index_t(vbase + 0) );
            indices.push_back( index_t(vbase + 1) );
            indices.push_back( index_t(vbase + 2) );

            indices.push_back( index_t(vbase + 0) );
            indices.push_back( index_t(vbase + 2) );
            indices.push_back( index_t(vbase + 3) );

            // Four vertices per face.

            // (normal - side1 - side2) * tsize // normal // t0
            vertices.push_back(Vertex(XMVectorMultiply(XMVectorSubtract(XMVectorSubtract(normal, side1), side2), tsize), normal, textureCoordinates[0]));

            // (normal - side1 + side2) * tsize // normal // t1
            vertices.push_back(Vertex(XMVectorMultiply(XMVectorAdd(XMVectorSubtract(normal, side1), side2), tsize), normal, textureCoordinates[1]));

            // (normal + side1 + side2) * tsize // normal // t2
            vertices.push_back(Vertex(XMVectorMultiply(XMVectorAdd(normal, XMVectorAdd(side1, side2)), tsize), normal, textureCoordinates[2]));

            // (normal + side1 - side2) * tsize // normal // t3
            vertices.push_back(Vertex(XMVectorMultiply(XMVectorSubtract(XMVectorAdd(normal, side1), side2), tsize), normal, textureCoordinates[3]));
        }

        if ( !rhcoords )
            ReverseWinding( indices, vertices );
    }

    static void CreateSphere( std::vector<index_t>& indices, std::vector<Vertex>& vertices, float diameter, size_t tessellation, bool rhcoords )
    {
        using namespace DirectX;

        indices.clear();
        vertices.clear();

        tessellation = std::max<size_t>( 3, tessellation );

        size_t verticalSegments = tessellation;
        size_t horizontalSegments = tessellation * 2;

        float radius = diameter / 2;

        // Create rings of vertices at progressively higher latitudes.
        for (size_t i = 0; i <= verticalSegments; i++)
        {
            float v = 1 - (float)i / verticalSegments;

            float latitude = (i * XM_PI / verticalSegments) - XM_PIDIV2;
            float dy, dxz;

            XMScalarSinCos(&dy, &dxz, latitude);

            // Create a single ring of vertices at this latitude.
            for (size_t j = 0; j <= horizontalSegments; j++)
            {
                float u = (float)j / horizontalSegments;

                float longitude = j * XM_2PI / horizontalSegments;
                float dx, dz;

                XMScalarSinCos(&dx, &dz, longitude);

                dx *= dxz;
                dz *= dxz;

                XMVECTOR normal = XMVectorSet(dx, dy, dz, 0);
                XMVECTOR textureCoordinate = XMVectorSet(u, v, 0, 0);

                vertices.push_back(Vertex(XMVectorScale(normal, radius), normal, textureCoordinate));
            }
        }

        // Fill the index buffer with triangles joining each pair of latitude rings.
        size_t stride = horizontalSegments + 1;

        for (size_t i = 0; i < verticalSegments; i++)
        {
            for (size_t j = 0; j <= horizontalSegments; j++)
            {
                size_t nextI = i + 1;
                size_t nextJ = (j + 1) % stride;

                indices.push_back( index_t(i * stride + j) );
                indices.push_back( index_t(nextI * stride + j) );
                indices.push_back( index_t(i * stride + nextJ) );

                indices.push_back( index_t(i * stride + nextJ) );
                indices.push_back( index_t(nextI * stride + j) );
                indices.push_back( index_t(nextI * stride + nextJ) );
            }
        }

        if ( !rhcoords )
            ReverseWinding( indices, vertices );
    }

    static void CreateCylinder( std::vector<index_t>& indices, std::vector<Vertex>& vertices, float height, float diameter, size_t tessellation, bool rhcoords )
    {
        using namespace DirectX;

        indices.clear();
        vertices.clear();

        tessellation = std::max<size_t>( 3, tessellation );

        height /= 2;

        XMVECTOR topOffset = XMVectorScale(g_XMIdentityR1, height);

        float radius = diameter / 2;
        size_t stride = tessellation + 1;

        // Create a ring of triangles around the outside of the cylinder.
        for (size_t i = 0; i <= tessellation; i++)
        {
            XMVECTOR normal = GetCircleVector(i, tessellation);

            XMVECTOR sideOffset = XMVectorScale(normal, radius);

            float u = (float)i / tessellation;

            XMVECTOR textureCoordinate = XMLoadFloat(&u);

            vertices.push_back(Vertex(XMVectorAdd(sideOffset, topOffset), normal, textureCoordinate));
            vertices.push_back(Vertex(XMVectorSubtract(sideOffset, topOffset), normal, XMVectorAdd(textureCoordinate, g_XMIdentityR1)));

            indices.push_back( index_t(i * 2) );
            indices.push_back( index_t((i * 2 + 2) % (stride * 2)) );
            indices.push_back( index_t(i * 2 + 1) );

            indices.push_back( index_t(i * 2 + 1) );
            indices.push_back( index_t((i * 2 + 2) % (stride * 2)) );
            indices.push_back( index_t((i * 2 + 3) % (stride * 2)) );
        }

        // Create flat triangle fan caps to seal the top and bottom.
        CreateCylinderCap(vertices, indices, tessellation, height, radius, true);
        CreateCylinderCap(vertices, indices, tessellation, height, radius, false);

        if ( !rhcoords )
            ReverseWinding( indices, vertices );
    }

    static void CreateTorus( std::vector<index_t>& indices, std::vector<Vertex>& vertices, float diameter, float thickness, size_t tessellation, bool rhcoords )
    {
        using namespace DirectX;

        indices.clear();
        vertices.clear();

        tessellation = std::max<size_t>( 3, tessellation );

        size_t stride = tessellation + 1;

        // First we loop around the main ring of the torus.
        for (size_t i = 0; i <= tessellation; i++)
        {
            float u = (float)i / tessellation;

            float outerAngle = i * XM_2PI / tessellation - XM_PIDIV2;

            // Create a transform matrix that will align geometry to
            // slice perpendicularly though the current ring position.
            XMMATRIX transform = XMMatrixTranslation(diameter / 2, 0, 0) * XMMatrixRotationY(outerAngle);

            // Now we loop along the other axis, around the side of the tube.
            for (size_t j = 0; j <= tessellation; j++)
            {
                float v = 1 - (float)j / tessellation;

                float innerAngle = j * XM_2PI / tessellation + XM_PI;
                float dx, dy;

                XMScalarSinCos(&dy, &dx, innerAngle);

                // Create a vertex.
                XMVECTOR normal = XMVectorSet(dx, dy, 0, 0);
                XMVECTOR position = XMVectorScale(normal, thickness / 2.f);
                XMVECTOR textureCoordinate = XMVectorSet(u, v, 0, 0);

                position = XMVector3Transform(position, transform);
                normal = XMVector3TransformNormal(normal, transform);

                vertices.push_back(Vertex(position, normal, textureCoordinate));

                // And create indices for two triangles.
                size_t nextI = (i + 1) % stride;
                size_t nextJ = (j + 1) % stride;

                indices.push_back( index_t(i * stride + j) );
                indices.push_back( index_t(i * stride + nextJ) );
                indices.push_back( index_t(nextI * stride + j) );

                indices.push_back( index_t(i * stride + nextJ) );
                indices.push_back( index_t(nextI * stride + nextJ) );
                indices.push_back( index_t(nextI * stride + j) );
            }
        }

        if ( !rhcoords )
            ReverseWinding( indices, vertices );
    }

private:
    static void ReverseWinding( std::vector<index_t>& indices, std::vector<Vertex>& vertices )
    {
        assert( (indices.size() % 3) == 0 );
        for( auto it = indices.begin(); it != indices.end(); it += 3 )
        {
            std::swap( *it, *(it+2) );
        }

        for (auto& it : vertices)
        {
            it.textureCoordinate.x = (1.f - it.textureCoordinate.x);
        }
    }

    static DirectX::XMVECTOR GetCircleVector(size_t i, size_t tessellation)
    {
        using namespace DirectX;

        float angle = i * XM_2PI / tessellation;
        float dx, dz;

        XMScalarSinCos(&dx, &dz, angle);

        XMVECTORF32 v = { { { dx, 0, dz, 0 } } };
        return v;
    }

    static DirectX::XMVECTOR GetCircleTangent(size_t i, size_t tessellation)
    {
        using namespace DirectX;

        float angle = ( i * XM_2PI / tessellation ) + XM_PIDIV2;
        float dx, dz;

        XMScalarSinCos(&dx, &dz, angle);

        XMVECTORF32 v = { dx, 0, dz, 0 };
        return v;
    }

    static void CreateCylinderCap( std::vector<Vertex>& vertices, std::vector<index_t>& indices, size_t tessellation, float height, float radius, bool isTop )
    {
        using namespace DirectX;

        // Create cap indices.
        for (size_t i = 0; i < tessellation - 2; i++)
        {
            size_t i1 = (i + 1) % tessellation;
            size_t i2 = (i + 2) % tessellation;

            if (isTop)
            {
                std::swap(i1, i2);
            }

            size_t vbase = vertices.size();
            indices.push_back( index_t(vbase) );
            indices.push_back( index_t(vbase + i1) );
            indices.push_back( index_t(vbase + i2) );
        }

        // Which end of the cylinder is this?
        XMVECTOR normal = g_XMIdentityR1;
        XMVECTOR textureScale = g_XMNegativeOneHalf;

        if (!isTop)
        {
            normal = XMVectorNegate(normal);
            textureScale = XMVectorMultiply(textureScale, g_XMNegateX);
        }

        // Create cap vertices.
        for (size_t i = 0; i < tessellation; i++)
        {
            XMVECTOR circleVector = GetCircleVector(i, tessellation);

            XMVECTOR position = XMVectorAdd( XMVectorScale(circleVector, radius), XMVectorScale(normal, height) );

            XMVECTOR textureCoordinate = XMVectorMultiplyAdd(XMVectorSwizzle<0, 2, 3, 3>(circleVector), textureScale, g_XMOneHalf);

            vertices.push_back(Vertex(position, normal, textureCoordinate));
        }
    }
};
