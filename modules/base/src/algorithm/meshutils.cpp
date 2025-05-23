/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2025 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <modules/base/algorithm/meshutils.h>
#include <modules/base/basemoduledefine.h>  // for IVW_MODULE_BASE_API

#include <inviwo/core/datastructures/buffer/buffer.h>                   // for Buffer, IndexBuffer
#include <inviwo/core/datastructures/buffer/bufferram.h>                // for BufferRAMPrecision
#include <inviwo/core/datastructures/camera/camera.h>                   // for mat4, Camera
#include <inviwo/core/datastructures/geometry/geometrytype.h>           // for ConnectivityType
#include <inviwo/core/datastructures/geometry/mesh.h>                   // for Mesh, Mesh::MeshInfo
#include <inviwo/core/datastructures/geometry/typedmesh.h>              // for TypedMesh<>::Vertex
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/util/glmmat.h>                                    // for mat3
#include <inviwo/core/util/glmvec.h>                                    // for vec3, vec4, ivec2

#ifdef WIN32
#define _USE_MATH_DEFINES
#endif
#include <array>          // for array, array<>::v...
#include <cmath>          // for atan, sqrt, sin, cos
#include <cstdint>        // for uint32_t
#include <cstdlib>        // for abs
#include <limits>         // for numeric_limits
#include <memory>         // for shared_ptr, make_...
#include <type_traits>    // for remove_extent_t
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set
#include <utility>        // for move
#include <numbers>
#include <ranges>

#include <glm/ext/matrix_transform.hpp>  // for rotate
#include <glm/fwd.hpp>                   // for vec4, vec3, uint32_t
#include <glm/geometric.hpp>             // for normalize, cross
#include <glm/gtc/constants.hpp>         // for half_pi, quarter_pi
#include <glm/gtx/rotate_vector.hpp>     // for rotate
#include <glm/gtx/transform.hpp>         // for rotate
#include <glm/mat3x3.hpp>                // for operator*, mat
#include <glm/mat4x4.hpp>                // for operator*
#include <glm/matrix.hpp>                // for inverse
#include <glm/trigonometric.hpp>         // for cos, sin
#include <glm/vec2.hpp>                  // for vec<>::(anonymous)
#include <glm/vec3.hpp>                  // for operator*, operator+
#include <glm/vec4.hpp>                  // for operator*, operator+

namespace inviwo {

namespace meshutil {

namespace detail {

vec3 orthvec(const vec3& vec) {
    vec3 u(1.0f, 0.0f, 0.0f);
    vec3 n = glm::normalize(vec);
    float p = glm::dot(u, n);
    if (std::abs(p) != 1.0f) {
        return glm::normalize(u - p * n);
    } else {
        return vec3(0.0f, 1.0f, 0.0f);
    }
}

vec3 tospherical(const vec2& v) {
    return vec3(std::sin(v.x) * std::cos(v.y), std::sin(v.x) * std::sin(v.y), std::cos(v.x));
}

}  // namespace detail

std::shared_ptr<BasicMesh> ellipse(const vec3& center, const vec3& majorAxis, const vec3& minorAxis,
                                   const vec4& color, const float& radius,
                                   const size_t& nsegments) {
    auto mesh = std::make_shared<BasicMesh>();
    vec3 p, p1;
    float angle = static_cast<float>(std::numbers::pi * 2.0 / nsegments);
    float a = glm::length(majorAxis);
    float b = glm::length(minorAxis);
    float eradius, eradius1;
    float x, y;
    vec3 rot, rot1;

    auto segments = static_cast<std::uint32_t>(nsegments);
    for (std::uint32_t i = 0; i < segments; ++i) {
        x = glm::cos(i * angle) * a;
        y = glm::sin(i * angle) * b;
        eradius = glm::length(glm::normalize(majorAxis) * x + glm::normalize(minorAxis) * y);
        rot = glm::normalize(majorAxis) * x + glm::normalize(minorAxis) * y;

        x = glm::cos(((i + 1) % segments) * angle) * a;
        y = glm::sin(((i + 1) % segments) * angle) * b;

        eradius1 = glm::length(glm::normalize(majorAxis) * x + glm::normalize(minorAxis) * y);
        rot1 = glm::normalize(majorAxis) * x + glm::normalize(minorAxis) * y;

        p = center + eradius * glm::normalize(rot);
        p1 = center + eradius1 * glm::normalize(rot1);

        vec3 dir = glm::normalize(p1 - p);
        vec3 odir = detail::orthvec(dir);

        float k;
        vec3 o;
        auto tube = std::make_unique<BasicMesh>();
        auto inds = tube->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);
        for (std::uint32_t j = 0; j < segments; ++j) {
            k = static_cast<float>(j);
            o = glm::rotate(odir, static_cast<float>(j * angle), dir);
            tube->addVertex(p + radius * o, o, vec3(k / segments, 0.0f, 0.0f), color);
            tube->addVertex(p1 + radius * o, o, vec3(k / segments, 1.0f, 0.0f), color);

            inds->add(j * 2 + 1);
            inds->add(j * 2 + 0);
            inds->add(((j + 1) * 2) % (2 * segments) + 0);

            inds->add(j * 2 + 1);
            inds->add(((j + 1) * 2) % (2 * segments) + 0);
            inds->add(((j + 1) * 2) % (2 * segments) + 1);
        }

        mesh->append(tube.get());
    }
    return mesh;
}

std::shared_ptr<BasicMesh> disk(const vec3& center, const vec3& normal, const vec4& color,
                                const float& radius, const size_t& segments) {
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1.f));
    auto inds = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);
    vec3 orth = detail::orthvec(normal);

    mesh->addVertex(center, normal, vec3(0.5f, 0.5f, 0.0f), color);

    vec3 tc = vec3(0.5f, 0.5f, 0.0f);
    vec3 tn = vec3(0.0f, 0.0f, 1.0f);
    vec3 to = vec3(0.5f, 0.0f, 0.0f);
    vec3 p;
    vec3 t;
    double angle = 2.0 * std::numbers::pi / segments;
    const auto ns = static_cast<std::uint32_t>(segments);
    for (std::uint32_t i = 0; i < ns; ++i) {
        p = center + radius * glm::rotate(orth, static_cast<float>(i * angle), normal);
        t = tc + glm::rotate(to, static_cast<float>(i * angle), tn);
        mesh->addVertex(p, normal, t, color);
        inds->add({0, 1 + i, 1 + ((i + 1) % ns)});
    }
    return mesh;
}

std::shared_ptr<BasicMesh> cone(const vec3& start, const vec3& stop, const vec4& color,
                                const float& radius, const size_t& segments) {
    const vec3 tc(0.5f, 0.5f, 0.0f);

    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1.f));

    auto inds = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);
    vec3 e1 = glm::normalize(stop - start);
    vec3 e2 = detail::orthvec(e1);
    vec3 e3 = glm::cross(e1, e2);
    mat3 basis(e1, e2, e3);

    float height = glm::length(stop - start);
    float ratio = radius / height;

    double angle = 2.0 * std::numbers::pi / segments;
    const auto ns = static_cast<std::uint32_t>(segments);
    for (std::uint32_t i = 0; i < ns; ++i) {
        // first vertex at base
        double x = cos(i * angle);
        double y = sin(i * angle);
        vec3 normal = basis * glm::normalize(vec3(ratio, x, y));
        vec3 pos = start + basis * vec3(0.0, x * radius, y * radius);
        vec3 texCoord = tc + vec3(0.5 * x, 0.5 * y, 0.0);
        mesh->addVertex(pos, normal, texCoord, color);

        // second vertex at base
        x = cos((i + 1) * angle);
        y = sin((i + 1) * angle);
        normal = basis * glm::normalize(vec3(ratio, x, y));
        pos = start + basis * vec3(0.0, x * radius, y * radius);
        texCoord = tc + vec3(0.5 * x, 0.5 * y, 0.0);
        mesh->addVertex(pos, normal, texCoord, color);

        // third vertex at tip
        x = cos((i + 0.5) * angle);
        y = sin((i + 0.5) * angle);
        normal = basis * glm::normalize(vec3(ratio, x, y));
        mesh->addVertex(stop, normal, tc, color);

        // add indices
        inds->add({i * 3 + 0, i * 3 + 1, i * 3 + 2});
    }

    return mesh;
}

std::shared_ptr<BasicMesh> cylinder(const vec3& start, const vec3& stop, const vec4& color,
                                    const float& radius, const size_t& segments, bool caps,
                                    std::shared_ptr<BasicMesh> mesh) {
    std::uint32_t globalIndexOffset = 0;
    if (!mesh) {
        mesh = std::make_shared<BasicMesh>();
        mesh->setModelMatrix(mat4(1.f));
    } else {
        globalIndexOffset = static_cast<std::uint32_t>(mesh->getVertices()->getSize());
    }
    std::vector<BasicMesh::Vertex> vertices;
    vertices.reserve(segments * 2);

    auto inds = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);
    inds->getDataContainer().reserve(6 * segments);
    vec3 e1 = glm::normalize(stop - start);
    vec3 e2 = detail::orthvec(e1);
    vec3 e3 = glm::cross(e1, e2);
    mat3 basis(e1, e2, e3);

    double angle = 2.0 * std::numbers::pi / segments;
    const auto ns = static_cast<std::uint32_t>(segments);
    for (std::uint32_t i = 0; i < ns; ++i) {
        // first vertex at base
        double x = cos(i * angle);
        double y = sin(i * angle);
        vec3 normal = basis * vec3(0.0, x, y);
        vec3 dir = basis * vec3(0.0, x * radius, y * radius);
        vec3 texCoord = vec3(static_cast<float>(i) / segments, 0.0, 0.0);

        // vertex at base
        vertices.push_back({start + dir, normal, texCoord, color});
        // vertex at top
        vertices.push_back({stop + dir, normal, texCoord + vec3(0.0, 1.0, 0.0), color});

        // indices for two triangles filling the strip
        inds->add(i * 2 + 1 + globalIndexOffset);
        inds->add(i * 2 + 0 + globalIndexOffset);
        inds->add(((i + 1) * 2) % (2 * ns) + 0 + globalIndexOffset);

        inds->add(i * 2 + 1 + globalIndexOffset);
        inds->add(((i + 1) * 2) % (2 * ns) + 0 + globalIndexOffset);
        inds->add(((i + 1) * 2) % (2 * ns) + 1 + globalIndexOffset);
    }

    mesh->addVertices(vertices);

    // add end caps
    if (caps) {
        auto startcap = disk(start, -e1, color, radius, segments);
        auto endcap = disk(stop, e1, color, radius, segments);

        mesh->append(startcap.get());
        mesh->append(endcap.get());
    }

    return mesh;
}

std::shared_ptr<BasicMesh> line(const vec3& start, const vec3& stop, const vec3& normal,
                                const vec4& color /*= vec4(1.0f, 0.0f, 0.0f, 1.0f)*/,
                                const float& width /*= 1.0f*/, const ivec2& inres /*= ivec2(1)*/) {
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1.f));
    auto inds = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);

    vec3 direction = stop - start;
    vec3 up = glm::cross(glm::normalize(direction), normal);

    vec3 startCornerPoint = start - 0.5f * width * up;
    ivec2 res = inres + ivec2(1);

    for (int j = 0; j < res.y; j++) {
        for (int i = 0; i < res.x; i++) {
            mesh->addVertex(startCornerPoint +
                                static_cast<float>(i) / static_cast<float>(inres.x) * direction +
                                static_cast<float>(j) / static_cast<float>(inres.y) * width * up,
                            normal,
                            vec3(static_cast<float>(i) / static_cast<float>(inres.x),
                                 static_cast<float>(j) / static_cast<float>(inres.y), 0.0f),
                            color);

            if (i != inres.x && j != inres.y) {
                inds->add(i + res.x * j);
                inds->add(i + 1 + res.x * j);
                inds->add(i + res.x * (j + 1));

                inds->add(i + 1 + res.x * j);
                inds->add(i + 1 + res.x * (j + 1));
                inds->add(i + res.x * (j + 1));
            }
        }
    }

    return mesh;
}

std::shared_ptr<BasicMesh> arrow(const vec3& start, const vec3& stop, const vec4& color,
                                 const float& radius, const float& arrowfraction,
                                 const float& arrowRadius, const size_t& segments) {
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1.f));

    vec3 mid = start + (1 - arrowfraction) * (stop - start);
    auto cylinderpart = cylinder(start, mid, color, radius, segments);
    mesh->append(cylinderpart.get());

    auto diskpart = disk(mid, start - mid, color, arrowRadius, segments);
    mesh->append(diskpart.get());

    auto conepart = cone(mid, stop, color, arrowRadius, segments);
    mesh->append(conepart.get());

    return mesh;
}

std::shared_ptr<BasicMesh> colorsphere(const vec3& center, const float& radius,
                                       std::shared_ptr<BasicMesh> mesh) {
    const static std::vector<vec2> spheremesh = {
        {glm::half_pi<float>(), 0.0f},
        {glm::half_pi<float>(), glm::half_pi<float>()},
        {0.0f, 0.0f},
        {0.0f, 0.0f},
        {std::atan(6.0f), 0.0f},
        {std::atan(5.0f / 2.0f), 0.0f},
        {std::atan(4.0f / 3.0f), 0.0f},
        {std::atan(3.0f / 4.0f), 0.0f},
        {std::atan(2.0f / 5.0f), 0.0f},
        {std::atan(1.0f / 6.0f), 0.0f},
        {glm::half_pi<float>(), std::atan(1.0f / 6.0f)},
        {std::atan(std::sqrt(26.0f)), std::atan(1.0f / 5.0f)},
        {std::atan(std::sqrt(17.0f) / 2.0f), std::atan(1.0f / 4.0f)},
        {std::atan(std::sqrt(10.0f) / 3.0f), std::atan(1.0f / 3.0f)},
        {std::atan(std::sqrt(5.0f) / 4.0f), std::atan(1.0f / 2.0f)},
        {std::atan(std::sqrt(2.0f) / 5.0f), glm::quarter_pi<float>()},
        {std::atan(1.0f / 6.0f), glm::half_pi<float>()},
        {glm::half_pi<float>(), std::atan(2.0f / 5.0f)},
        {std::atan(2.0f * std::sqrt(5.0f)), std::atan(1.0f / 2.0f)},
        {std::atan(std::sqrt(13.0f) / 2.0f), std::atan(2.0f / 3.0f)},
        {std::atan((2.0f * std::sqrt(2.0f)) / 3.0f), glm::quarter_pi<float>()},
        {std::atan(std::sqrt(5.0f) / 4.0f), std::atan(2.0f)},
        {std::atan(2.0f / 5.0f), glm::half_pi<float>()},
        {glm::half_pi<float>(), std::atan(3.0f / 4.0f)},
        {std::atan(3.0f * std::sqrt(2.0f)), glm::quarter_pi<float>()},
        {std::atan(std::sqrt(13.0f) / 2.0f), std::atan(3.0f / 2.0f)},
        {std::atan(std::sqrt(10.0f) / 3.0f), std::atan(3.0f)},
        {std::atan(3.0f / 4.0f), glm::half_pi<float>()},
        {glm::half_pi<float>(), std::atan(4.0f / 3.0f)},
        {std::atan(2.0f * std::sqrt(5.0f)), std::atan(2.0f)},
        {std::atan(std::sqrt(17.0f) / 2.0f), std::atan(4.0f)},
        {std::atan(4.0f / 3.0f), glm::half_pi<float>()},
        {glm::half_pi<float>(), std::atan(5.0f / 2.0f)},
        {std::atan(std::sqrt(26.0f)), std::atan(5.0f)},
        {std::atan(5.0f / 2.0f), glm::half_pi<float>()},
        {glm::half_pi<float>(), std::atan(6.0f)},
        {std::atan(6.0f), glm::half_pi<float>()}};

    const static std::vector<uvec3> sphereind = {
        {0, 10, 4},   {4, 11, 5},   {5, 12, 6},   {6, 13, 7},   {7, 14, 8},   {8, 15, 9},
        {9, 16, 2},   {10, 17, 11}, {11, 18, 12}, {12, 19, 13}, {13, 20, 14}, {14, 21, 15},
        {15, 22, 16}, {17, 23, 18}, {18, 24, 19}, {19, 25, 20}, {20, 26, 21}, {21, 27, 22},
        {23, 28, 24}, {24, 29, 25}, {25, 30, 26}, {26, 31, 27}, {28, 32, 29}, {29, 33, 30},
        {30, 34, 31}, {32, 35, 33}, {33, 36, 34}, {35, 1, 36},  {11, 4, 10},  {12, 5, 11},
        {13, 6, 12},  {14, 7, 13},  {15, 8, 14},  {16, 9, 15},  {18, 11, 17}, {19, 12, 18},
        {20, 13, 19}, {21, 14, 20}, {22, 15, 21}, {24, 18, 23}, {25, 19, 24}, {26, 20, 25},
        {27, 21, 26}, {29, 24, 28}, {30, 25, 29}, {31, 26, 30}, {33, 29, 32}, {34, 30, 33},
        {36, 33, 35}};

    std::uint32_t globalIndexOffset = 0;
    if (!mesh) {
        mesh = std::make_shared<BasicMesh>();
        mesh->setModelMatrix(mat4(1.f));
    } else {
        globalIndexOffset = static_cast<std::uint32_t>(mesh->getVertices()->getSize());
    }

    std::vector<BasicMesh::Vertex> vertices;
    vertices.reserve(spheremesh.size() * 8);
    auto inds = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);
    inds->getDataContainer().reserve(sphereind.size() * 8 * 3);

    vec3 quad(0);
    for (quad.x = -1.0f; quad.x <= 1.0f; quad.x += 2.0f) {
        for (quad.y = -1.0f; quad.y <= 1.0f; quad.y += 2.0f) {
            for (quad.z = -1.0f; quad.z <= 1.0f; quad.z += 2.0f) {
                glm::uint32_t idxOffset =
                    static_cast<std::uint32_t>(vertices.size()) + globalIndexOffset;

                vec3 normal;
                vec3 vertex;
                vec3 tcoord;
                vec4 color((quad + 1.0f) / 2.0f, 1.0f);
                for (auto& elem : spheremesh) {
                    normal = quad * detail::tospherical(elem);
                    vertex = center + radius * normal;
                    tcoord = vec3(quad.x * (elem).x, quad.y * (elem).y, 0.0f);
                    vertices.push_back({vertex, normal, tcoord, color});
                }

                if (quad.x * quad.y * quad.z > 0) {
                    for (auto& elem : sphereind) {
                        inds->add({idxOffset + elem.x, idxOffset + elem.y, idxOffset + elem.z});
                    }
                } else {
                    for (auto& elem : sphereind) {
                        inds->add({idxOffset + elem.z, idxOffset + elem.y, idxOffset + elem.x});
                    }
                }
            }
        }
    }
    mesh->addVertices(vertices);
    return mesh;
}

std::shared_ptr<BasicMesh> sphere(const vec3& center, const float& radius, const vec4& color,
                                  std::shared_ptr<BasicMesh> mesh) {
    std::vector<vec2> spheremesh = {
        {glm::half_pi<float>(), 0.0f},
        {glm::half_pi<float>(), glm::half_pi<float>()},
        {0.0f, 0.0f},
        {0.0f, 0.0f},
        {std::atan(6.0f), 0.0f},
        {std::atan(5.0f / 2.0f), 0.0f},
        {std::atan(4.0f / 3.0f), 0.0f},
        {std::atan(3.0f / 4.0f), 0.0f},
        {std::atan(2.0f / 5.0f), 0.0f},
        {std::atan(1.0f / 6.0f), 0.0f},
        {glm::half_pi<float>(), std::atan(1.0f / 6.0f)},
        {std::atan(std::sqrt(26.0f)), std::atan(1.0f / 5.0f)},
        {std::atan(std::sqrt(17.0f) / 2.0f), std::atan(1.0f / 4.0f)},
        {std::atan(std::sqrt(10.0f) / 3.0f), std::atan(1.0f / 3.0f)},
        {std::atan(std::sqrt(5.0f) / 4.0f), std::atan(1.0f / 2.0f)},
        {std::atan(std::sqrt(2.0f) / 5.0f), glm::quarter_pi<float>()},
        {std::atan(1.0f / 6.0f), glm::half_pi<float>()},
        {glm::half_pi<float>(), std::atan(2.0f / 5.0f)},
        {std::atan(2.0f * std::sqrt(5.0f)), std::atan(1.0f / 2.0f)},
        {std::atan(std::sqrt(13.0f) / 2.0f), std::atan(2.0f / 3.0f)},
        {std::atan((2.0f * std::sqrt(2.0f)) / 3.0f), glm::quarter_pi<float>()},
        {std::atan(std::sqrt(5.0f) / 4.0f), std::atan(2.0f)},
        {std::atan(2.0f / 5.0f), glm::half_pi<float>()},
        {glm::half_pi<float>(), std::atan(3.0f / 4.0f)},
        {std::atan(3.0f * std::sqrt(2.0f)), glm::quarter_pi<float>()},
        {std::atan(std::sqrt(13.0f) / 2.0f), std::atan(3.0f / 2.0f)},
        {std::atan(std::sqrt(10.0f) / 3.0f), std::atan(3.0f)},
        {std::atan(3.0f / 4.0f), glm::half_pi<float>()},
        {glm::half_pi<float>(), std::atan(4.0f / 3.0f)},
        {std::atan(2.0f * std::sqrt(5.0f)), std::atan(2.0f)},
        {std::atan(std::sqrt(17.0f) / 2.0f), std::atan(4.0f)},
        {std::atan(4.0f / 3.0f), glm::half_pi<float>()},
        {glm::half_pi<float>(), std::atan(5.0f / 2.0f)},
        {std::atan(std::sqrt(26.0f)), std::atan(5.0f)},
        {std::atan(5.0f / 2.0f), glm::half_pi<float>()},
        {glm::half_pi<float>(), std::atan(6.0f)},
        {std::atan(6.0f), glm::half_pi<float>()}};

    std::vector<uvec3> sphereind = {
        {0, 10, 4},   {4, 11, 5},   {5, 12, 6},   {6, 13, 7},   {7, 14, 8},   {8, 15, 9},
        {9, 16, 2},   {10, 17, 11}, {11, 18, 12}, {12, 19, 13}, {13, 20, 14}, {14, 21, 15},
        {15, 22, 16}, {17, 23, 18}, {18, 24, 19}, {19, 25, 20}, {20, 26, 21}, {21, 27, 22},
        {23, 28, 24}, {24, 29, 25}, {25, 30, 26}, {26, 31, 27}, {28, 32, 29}, {29, 33, 30},
        {30, 34, 31}, {32, 35, 33}, {33, 36, 34}, {35, 1, 36},  {11, 4, 10},  {12, 5, 11},
        {13, 6, 12},  {14, 7, 13},  {15, 8, 14},  {16, 9, 15},  {18, 11, 17}, {19, 12, 18},
        {20, 13, 19}, {21, 14, 20}, {22, 15, 21}, {24, 18, 23}, {25, 19, 24}, {26, 20, 25},
        {27, 21, 26}, {29, 24, 28}, {30, 25, 29}, {31, 26, 30}, {33, 29, 32}, {34, 30, 33},
        {36, 33, 35}};

    std::uint32_t globalIndexOffset = 0;
    if (!mesh) {
        mesh = std::make_shared<BasicMesh>();
        mesh->setModelMatrix(mat4(1.f));
    } else {
        globalIndexOffset = static_cast<std::uint32_t>(mesh->getVertices()->getSize());
    }

    std::vector<BasicMesh::Vertex> vertices;
    vertices.reserve(spheremesh.size() * 8);
    auto inds = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);
    inds->getDataContainer().reserve(sphereind.size() * 8 * 3);

    vec3 quad(0);
    for (quad.x = -1.0f; quad.x <= 1.0f; quad.x += 2.0f) {
        for (quad.y = -1.0f; quad.y <= 1.0f; quad.y += 2.0f) {
            for (quad.z = -1.0f; quad.z <= 1.0f; quad.z += 2.0f) {
                glm::uint32_t idxOffset =
                    static_cast<std::uint32_t>(vertices.size()) + globalIndexOffset;
                vec3 normal;
                vec3 vertex;
                vec3 tcoord;
                for (auto& elem : spheremesh) {
                    normal = quad * detail::tospherical(elem);
                    vertex = center + radius * normal;
                    tcoord = vec3(quad.x * (elem).x, quad.y * (elem).y, 0.0f);
                    vertices.push_back({vertex, normal, tcoord, color});
                }

                if (quad.x * quad.y * quad.z > 0) {
                    for (auto& elem : sphereind) {
                        inds->add({idxOffset + elem.x, idxOffset + elem.y, idxOffset + elem.z});
                    }
                } else {
                    for (auto& elem : sphereind) {
                        inds->add({idxOffset + elem.z, idxOffset + elem.y, idxOffset + elem.x});
                    }
                }
            }
        }
    }
    mesh->addVertices(vertices);
    return mesh;
}

static vec3 V(const mat4& m, const vec3 v) {
    vec4 V = m * vec4(v, 1);
    return vec3(V) / V.w;
}

static vec3 N(const mat4& m, const vec3 v) {
    vec4 V = m * vec4(v, 0);
    return vec3(V);
}

std::shared_ptr<BasicMesh> cube(const mat4& m, const vec4& color) {
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1));

    auto indices = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);

    // Front back
    mesh->addVertices({{V(m, vec3(0, 0, 0)), N(m, vec3(0, 0, -1)), vec3(0, 0, 0), color},
                       {V(m, vec3(1, 0, 0)), N(m, vec3(0, 0, -1)), vec3(1, 0, 0), color},
                       {V(m, vec3(1, 1, 0)), N(m, vec3(0, 0, -1)), vec3(1, 1, 0), color},
                       {V(m, vec3(0, 1, 0)), N(m, vec3(0, 0, -1)), vec3(0, 1, 0), color}});

    std::uint32_t o = 0;
    indices->add({0 + o, 1 + o, 2 + o, 0 + o, 2 + o, 3 + o});

    mesh->addVertices({{V(m, vec3(0, 0, 1)), N(m, vec3(0, 0, 1)), vec3(0, 0, 1), color},
                       {V(m, vec3(1, 0, 1)), N(m, vec3(0, 0, 1)), vec3(1, 0, 1), color},
                       {V(m, vec3(1, 1, 1)), N(m, vec3(0, 0, 1)), vec3(1, 1, 1), color},
                       {V(m, vec3(0, 1, 1)), N(m, vec3(0, 0, 1)), vec3(0, 1, 1), color}});

    o += 4;
    indices->add({0 + o, 2 + o, 1 + o, 0 + o, 3 + o, 2 + o});

    // Right left
    mesh->addVertices({{V(m, vec3(0, 0, 0)), N(m, vec3(-1, 0, 0)), vec3(0, 0, 0), color},
                       {V(m, vec3(0, 1, 0)), N(m, vec3(-1, 0, 0)), vec3(0, 1, 0), color},
                       {V(m, vec3(0, 1, 1)), N(m, vec3(-1, 0, 0)), vec3(0, 1, 1), color},
                       {V(m, vec3(0, 0, 1)), N(m, vec3(-1, 0, 0)), vec3(0, 0, 1), color}});
    o += 4;
    indices->add({0 + o, 1 + o, 2 + o, 0 + o, 2 + o, 3 + o});

    mesh->addVertices({{V(m, vec3(1, 0, 0)), N(m, vec3(1, 0, 0)), vec3(1, 0, 0), color},
                       {V(m, vec3(1, 1, 0)), N(m, vec3(1, 0, 0)), vec3(1, 1, 0), color},
                       {V(m, vec3(1, 1, 1)), N(m, vec3(1, 0, 0)), vec3(1, 1, 1), color},
                       {V(m, vec3(1, 0, 1)), N(m, vec3(1, 0, 0)), vec3(1, 0, 1), color}});
    o += 4;
    indices->add({0 + o, 2 + o, 1 + o, 0 + o, 3 + o, 2 + o});

    // top bottom
    mesh->addVertices({{V(m, vec3(0, 1, 0)), N(m, vec3(0, 1, 0)), vec3(0, 1, 0), color},
                       {V(m, vec3(1, 1, 0)), N(m, vec3(0, 1, 0)), vec3(1, 1, 0), color},
                       {V(m, vec3(1, 1, 1)), N(m, vec3(0, 1, 0)), vec3(1, 1, 1), color},
                       {V(m, vec3(0, 1, 1)), N(m, vec3(0, 1, 0)), vec3(0, 1, 1), color}});
    o += 4;
    indices->add({0 + o, 1 + o, 2 + o, 0 + o, 2 + o, 3 + o});

    mesh->addVertices({{V(m, vec3(0, 0, 0)), N(m, vec3(0, -1, 0)), vec3(0, -1, 0), color},
                       {V(m, vec3(1, 0, 0)), N(m, vec3(0, -1, 0)), vec3(1, -1, 0), color},
                       {V(m, vec3(1, 0, 1)), N(m, vec3(0, -1, 0)), vec3(1, -1, 1), color},
                       {V(m, vec3(0, 0, 1)), N(m, vec3(0, -1, 0)), vec3(0, -1, 1), color}});
    o += 4;
    indices->add({0 + o, 2 + o, 1 + o, 0 + o, 3 + o, 2 + o});

    return mesh;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
std::shared_ptr<Mesh> cubeIndicator(const mat4& basisAndOffset) {
    constexpr float offset = 0.2f;
    constexpr std::array<float, 4> p{{0.0f, 0.0f + offset, 1.0f - offset, 1.0f}};
    constexpr size_t pointsPerFace = (p.size() - 1) * (p.size() - 1) * 4;
    constexpr size_t indicesPerFace = (p.size() - 1) * (p.size() - 1) * 6;
    constexpr size_t facesPerCube = 6;

    std::vector<vec3> positions;
    positions.reserve(facesPerCube * pointsPerFace);
    std::vector<vec3> normals;
    normals.reserve(positions.capacity());
    std::vector<vec4> colors;
    colors.reserve(positions.capacity());
    std::vector<std::uint32_t> pickIds;
    pickIds.reserve(positions.capacity());

    std::vector<std::uint32_t> indices;
    indices.reserve(facesPerCube * indicesPerFace);

    constexpr auto index = [](float x) -> std::uint32_t {
        if (x < 1.0f / 3.0f) {
            return 0;
        } else if (x > 2.0f / 3.0f) {
            return 2;
        } else {
            return 1;
        }
    };

    constexpr auto xyz = +[](const vec3& v) { return vec3{v.x, v.y, v.z}; };
    constexpr auto zxy = +[](const vec3& v) { return vec3{v.z, v.x, v.y}; };
    constexpr auto yzx = +[](const vec3& v) { return vec3{v.y, v.z, v.x}; };

    // generate a mesh of 3x3 quads on each side of the cube
    for (const auto& swizzle : {xyz, zxy, yzx}) {
        for (const auto x : {0.0f, 1.0f}) {
            for (const auto [y1, y2] : std::views::zip(p, p | std::views::drop(1))) {
                for (const auto [z1, z2] : std::views::zip(p, p | std::views::drop(1))) {
                    const auto c1 = vec3{x, y1, z1};
                    const auto c2 = vec3{x, y2, z1};
                    const auto c3 = vec3{x, y2, z2};
                    const auto c4 = vec3{x, y1, z2};

                    for (const uint32_t i : {0, 1, 2, 0, 2, 3}) {
                        indices.emplace_back(static_cast<uint32_t>(positions.size()) + i);
                    }

                    const auto cs1 = swizzle(c1);
                    const auto cs2 = swizzle(c2);
                    const auto cs3 = swizzle(c3);
                    const auto cs4 = swizzle(c4);

                    positions.emplace_back(cs1);
                    positions.emplace_back(cs2);
                    positions.emplace_back(cs3);
                    positions.emplace_back(cs4);

                    const auto center = (cs1 + cs2 + cs3 + cs4) / 4.0f;
                    const auto pickPos =
                        glm::u32vec3{index(center.x), index(center.y), index(center.z)};

                    const std::uint32_t pickId = pickPos.x + 3 * pickPos.y + 9 * pickPos.z;
                    pickIds.emplace_back(pickId);
                    pickIds.emplace_back(pickId);
                    pickIds.emplace_back(pickId);
                    pickIds.emplace_back(pickId);
                }
            }
        }
    }

    // generate normals for each point
    for (const auto& normal : {vec3{1.f, 0.f, 0.f}, vec3{0.f, 1.f, 0.f}, vec3{0.f, 0.f, 1.f}}) {
        for (const auto side : {1.f, -1.f}) {
            for ([[maybe_unused]] const auto _ : std::views::iota(size_t{0}, pointsPerFace)) {
                normals.emplace_back(normal * side);
            }
        }
    }

    // generate colors for each point
    for (const auto& color : {vec3{1.f, 0.f, 0.f}, vec3{0.f, 1.f, 0.f}, vec3{0.f, 0.f, 1.f}}) {
        for (const auto side : {-0.2f, 0.2f}) {
            for (const auto& xCorner : {vec3{0.1f}, vec3{0.0f}, vec3{0.1f}}) {
                for (const auto& yCorner : {vec3{0.1f}, vec3{0.0f}, vec3{0.1f}}) {
                    for ([[maybe_unused]] const auto _ : std::views::iota(0, 4)) {
                        const auto c = 0.5f * color + side + xCorner + yCorner;
                        colors.emplace_back(c, 1.0);
                    }
                }
            }
        }
    }

    const auto pos = std::make_shared<Buffer<vec3>>(
        std::make_shared<BufferRAMPrecision<vec3>>(std::move(positions)));
    const auto norm = std::make_shared<Buffer<vec3>>(
        std::make_shared<BufferRAMPrecision<vec3>>(std::move(normals)));
    const auto col = std::make_shared<Buffer<vec4>>(
        std::make_shared<BufferRAMPrecision<vec4>>(std::move(colors)));
    const auto pick = std::make_shared<Buffer<std::uint32_t>>(
        std::make_shared<BufferRAMPrecision<std::uint32_t>>(std::move(pickIds)));

    const auto inds =
        std::make_shared<IndexBuffer>(std::make_shared<IndexBufferRAM>(std::move(indices)));

    auto mesh = std::make_shared<Mesh>(
        Mesh::BufferVector{{BufferType::PositionAttrib, pos},
                           {BufferType::NormalAttrib, norm},
                           {BufferType::ColorAttrib, col},
                           {BufferType::PickingAttrib, pick}},
        Mesh::IndexVector{{Mesh::MeshInfo{DrawType::Triangles, ConnectivityType::None}, inds}});

    mesh->setModelMatrix(basisAndOffset);

    return mesh;
}

std::shared_ptr<BasicMesh> coordindicator(const vec3& center, const float& size) {
    size_t segments = 16;
    float bsize = size * 1.0f;
    float fsize = size * 1.2f;
    float radius = size * 0.08f;
    float arrowpart = 0.12f;
    float arrowRadius = 2.0f * radius;
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1.f));

    auto xarrow = arrow(center - vec3(bsize, 0.0f, 0.0f), center + vec3(fsize, 0.0f, 0.0f),
                        vec4(1.0f, 0.0f, 0.0f, 1.0f), radius, arrowpart, arrowRadius, segments);
    auto yarrow = arrow(center - vec3(0.0f, bsize, 0.0f), center + vec3(0.0f, fsize, 0.0f),
                        vec4(0.0f, 1.0f, 0.0f, 1.0f), radius, arrowpart, arrowRadius, segments);
    auto zarrow = arrow(center - vec3(0.0f, 0.0f, bsize), center + vec3(0.0f, 0.0f, fsize),
                        vec4(0.0f, 0.0f, 1.0f, 1.0f), radius, arrowpart, arrowRadius, segments);

    auto sphere = colorsphere(center, 0.7f * size);

    mesh->append(sphere.get());
    mesh->append(xarrow.get());
    mesh->append(yarrow.get());
    mesh->append(zarrow.get());

    return mesh;
}

std::shared_ptr<BasicMesh> boundingbox(const mat4& basisAndOffset, const vec4& color) {
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(basisAndOffset);

    mesh->addVertices({{vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), vec3(0.0, 0.0, 0.0), color},
                       {vec3(1.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), vec3(1.0, 0.0, 0.0), color},
                       {vec3(0.0, 1.0, 0.0), vec3(1.0, 1.0, 1.0), vec3(0.0, 1.0, 0.0), color},
                       {vec3(0.0, 0.0, 1.0), vec3(1.0, 1.0, 1.0), vec3(0.0, 0.0, 1.0), color},
                       {vec3(1.0, 1.0, 0.0), vec3(1.0, 1.0, 1.0), vec3(1.0, 1.0, 0.0), color},
                       {vec3(0.0, 1.0, 1.0), vec3(1.0, 1.0, 1.0), vec3(0.0, 1.0, 1.0), color},
                       {vec3(1.0, 0.0, 1.0), vec3(1.0, 1.0, 1.0), vec3(1.0, 0.0, 1.0), color},
                       {vec3(1.0, 1.0, 1.0), vec3(1.0, 1.0, 1.0), vec3(1.0, 1.0, 1.0), color}});

    auto inds = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::None);
    inds->add({0, 1, 0, 2, 0, 3, 1, 6, 1, 4, 2, 5, 2, 4, 3, 5, 3, 6, 5, 7, 6, 7, 4, 7});

    return mesh;
}

//! [Using PosTexColorMesh]
std::shared_ptr<PosTexColorMesh> boundingBoxAdjacency(const mat4& basisAndOffset,
                                                      const vec4& color) {
    auto mesh = std::make_shared<PosTexColorMesh>();
    mesh->setModelMatrix(basisAndOffset);

    mesh->addVertices({{vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0), color},
                       {vec3(1.0, 0.0, 0.0), vec3(1.0, 0.0, 0.0), color},
                       {vec3(1.0, 1.0, 0.0), vec3(1.0, 1.0, 0.0), color},
                       {vec3(0.0, 1.0, 0.0), vec3(0.0, 1.0, 0.0), color},
                       {vec3(0.0, 0.0, 1.0), vec3(0.0, 0.0, 1.0), color},
                       {vec3(1.0, 0.0, 1.0), vec3(1.0, 0.0, 1.0), color},
                       {vec3(1.0, 1.0, 1.0), vec3(1.0, 1.0, 1.0), color},
                       {vec3(0.0, 1.0, 1.0), vec3(0.0, 1.0, 1.0), color}});

    auto inds1 = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::StripAdjacency);
    inds1->add({3, 0, 1, 2, 3, 0, 1});

    auto inds2 = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::StripAdjacency);
    inds2->add({7, 4, 5, 6, 7, 4, 5});

    auto inds3 = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::StripAdjacency);
    inds3->add({3, 0, 4, 7, 3, 0, 4});

    auto inds4 = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::StripAdjacency);
    inds4->add({2, 1, 5, 6, 2, 1, 2});

    return mesh;
}
//! [Using PosTexColorMesh]

std::shared_ptr<BasicMesh> torus(const vec3& center, const vec3& up_, float r1, float r2,
                                 const ivec2& subdivisions, vec4 color) {
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1.f));
    auto inds = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);

    vec3 side;
    auto up = glm::normalize(up_);
    if (std::abs(std::abs(glm::dot(up, vec3(0, 1, 0))) - 1) <
        std::numeric_limits<float>::epsilon()) {
        side = vec3(1, 0, 0);
    } else {
        side = glm::normalize(glm::cross(up, vec3(0, 1, 0)));
    }

    auto numVertex = subdivisions.x * subdivisions.y;

    for (int i = 0; i < subdivisions.x; i++) {
        auto axis =
            glm::rotate(side, static_cast<float>(i * 2 * std::numbers::pi / subdivisions.x), up);
        auto centerP = center + axis * r1;

        auto N = glm::normalize(glm::cross(axis, up));

        int startI = i * subdivisions.y;
        int startJ = ((i + 1) % subdivisions.x) * subdivisions.y;

        for (int j = 0; j < subdivisions.y; j++) {
            auto axis2 =
                glm::rotate(axis, static_cast<float>(j * 2 * std::numbers::pi / subdivisions.y), N);
            auto VP = centerP + axis2 * r2;

            mesh->addVertex(VP, axis2, N, color);

            int i0 = startI + j;
            int i1 = startI + j + 1;
            int j0 = startJ + j;
            int j1 = startJ + j + 1;

            inds->add(i0 % numVertex);
            inds->add(j0 % numVertex);
            inds->add(i1 % numVertex);

            inds->add(j0 % numVertex);
            inds->add(j1 % numVertex);
            inds->add(i1 % numVertex);
        }
    }

    return mesh;
}

std::shared_ptr<BasicMesh> square(const vec3& center, const vec3& normal, const vec2& extent,
                                  const vec4& color /*= vec4(1,1,1,1)*/,
                                  const ivec2& segments /*= ivec2(1)*/) {
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1.f));
    auto inds = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);

    vec3 right = detail::orthvec(normal);
    vec3 up = glm::cross(right, normal);

    vec3 start = center - 0.5f * extent.x * right - 0.5f * extent.y * up;
    ivec2 res = segments + ivec2(1);

    for (int j = 0; j < res.y; j++) {
        for (int i = 0; i < res.x; i++) {
            mesh->addVertex(
                start + static_cast<float>(i) / static_cast<float>(segments.x) * extent.x * right +
                    static_cast<float>(j) / static_cast<float>(segments.y) * extent.y * up,
                normal,
                vec3(static_cast<float>(i) / static_cast<float>(segments.x),
                     static_cast<float>(j) / static_cast<float>(segments.y), 0.0f),
                color);

            if (i != segments.x && j != segments.y) {
                inds->add(i + res.x * j);
                inds->add(i + 1 + res.x * j);
                inds->add(i + res.x * (j + 1));

                inds->add(i + 1 + res.x * j);
                inds->add(i + 1 + res.x * (j + 1));
                inds->add(i + res.x * (j + 1));
            }
        }
    }

    return mesh;
}

//! [Using Colored Mesh]
std::shared_ptr<ColoredMesh> cameraFrustum(const Camera& camera, vec4 color,
                                           std::shared_ptr<ColoredMesh> mesh) {
    const static std::vector<vec3> vertices{vec3(-1, -1, -1), vec3(-1, 1, -1), vec3(1, -1, -1),
                                            vec3(1, 1, -1),   vec3(-1, -1, 1), vec3(-1, 1, 1),
                                            vec3(1, -1, 1),   vec3(1, 1, 1)};

    auto& vertVector = mesh->getTypedDataContainer<buffertraits::PositionsBuffer>();
    auto& colorVector = mesh->getTypedDataContainer<buffertraits::ColorsBuffer>();

    auto off = static_cast<unsigned int>(vertVector.size());
    vertVector.insert(vertVector.end(), vertices.begin(), vertices.end());
    colorVector.insert(colorVector.end(), 8, color);

    mesh->setModelMatrix(glm::inverse(camera.getProjectionMatrix() * camera.getViewMatrix()));

    auto ib = std::make_shared<IndexBufferRAM>();
    auto indices = std::make_shared<IndexBuffer>(ib);
    ib->add({off + 0, off + 1, off + 1, off + 3, off + 3, off + 2, off + 2, off + 0});  // front
    ib->add({off + 4, off + 5, off + 5, off + 7, off + 7, off + 6, off + 6, off + 4});  // back
    ib->add({off + 0, off + 4, off + 1, off + 5, off + 2, off + 6, off + 3, off + 7});  // sides

    mesh->addIndices(Mesh::MeshInfo(DrawType::Lines, ConnectivityType::None), indices);

    return mesh;
}
//! [Using Colored Mesh]

std::shared_ptr<Mesh> parallelepiped(glm::vec3 origin, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3,
                                     glm::vec4 color, glm::vec4 c1, glm::vec4 c2, glm::vec4 c3,
                                     IncludeNormals includeNormals) {

    // create a mesh with positions (vec3), normals (vec3), textures(vec3),
    auto mesh = std::make_shared<Mesh>();

    // Set identity matrix
    mesh->setModelMatrix(mat4(1.f));

    auto corners = [](const auto& origin, const auto& v1, const auto& v2, const auto& v3) {
        return std::array{
            origin,      origin + v1,      origin + v2,      origin + v1 + v2,
            origin + v3, origin + v1 + v3, origin + v2 + v3, origin + v1 + v2 + v3,
        };
    };

    const auto positions = corners(origin, p1, p2, p3);
    const auto colors = corners(color, c1, c2, c3);

    constexpr std::array<std::array<size_t, 4>, 6> faces = {
        {{0, 2, 3, 1}, {4, 5, 7, 6}, {0, 1, 5, 4}, {1, 3, 7, 5}, {3, 2, 6, 7}, {0, 4, 6, 2}}};

    auto calcNnormal = [&](const std::array<size_t, 4>& inds) {
        const auto a = positions[inds[1]] - positions[inds[0]];
        const auto b = positions[inds[3]] - positions[inds[0]];
        return glm::normalize(glm::cross(a, b));
    };

    auto indexBuffer = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);
    auto& indices = indexBuffer->getDataContainer();

    std::vector<vec3> positionVertices;
    std::vector<vec4> colorVertices;
    std::vector<vec3> normalVertices;

    positionVertices.reserve(faces.size() * faces[0].size());
    colorVertices.reserve(faces.size() * faces[0].size());
    normalVertices.reserve(faces.size() * faces[0].size());

    for (auto& face : faces) {
        const auto normal = calcNnormal(face);
        const auto ind = static_cast<uint32_t>(positionVertices.size());
        for (auto i : face) {
            positionVertices.emplace_back(positions[i]);
            colorVertices.emplace_back(colors[i]);
            normalVertices.emplace_back(normal);
        }

        indices.push_back(ind + 0);
        indices.push_back(ind + 1);
        indices.push_back(ind + 2);

        indices.push_back(ind + 0);
        indices.push_back(ind + 2);
        indices.push_back(ind + 3);
    }

    mesh->addBuffer(BufferType::PositionAttrib,
                    std::make_shared<Buffer<vec3>>(
                        std::make_shared<BufferRAMPrecision<vec3>>(std::move(positionVertices))));

    mesh->addBuffer(BufferType::ColorAttrib,
                    std::make_shared<Buffer<vec4>>(
                        std::make_shared<BufferRAMPrecision<vec4>>(std::move(colorVertices))));

    if (includeNormals == IncludeNormals::Yes) {
        mesh->addBuffer(BufferType::NormalAttrib,
                        std::make_shared<Buffer<vec3>>(
                            std::make_shared<BufferRAMPrecision<vec3>>(std::move(normalVertices))));
    }

    return mesh;
}

}  // namespace meshutil

}  // namespace inviwo
