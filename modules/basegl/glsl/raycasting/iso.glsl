/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2025 Inviwo Foundation
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

#if !defined MAX_ISOVALUE_COUNT
#define MAX_ISOVALUE_COUNT 1
#endif

#if !defined INC_ISOVALUEPARAMETERS
#define INC_ISOVALUEPARAMETERS
struct IsovalueParameters {
    float values[MAX_ISOVALUE_COUNT];
    vec4 colors[MAX_ISOVALUE_COUNT];
    int size;
};
#endif

/**
 * Draws an isosurface if the given isovalue is found along the ray in between the
 * current and the previous volume sample. On return, tIncr refers to the distance
 * between the last valid isosurface and the current sampling position. That is
 * if no isosurface was found, tIncr is not modified.
 *
 * @param result           color accumulated so far during raycasting
 * @param isovalue         isovalue of isosurface to be drawn
 * @param isocolor         color of the isosurface used for blending
 * @param value            scalar values of current sampling position
 * @param previousValue    scalar values of previous sample
 * @param gradient
 * @param previousGradient
 * @param textureToWorld
 * @param lighting
 * @param rayPosition
 * @param rayDirection
 * @param toCameraDir
 * @param t
 * @param tIncr
 * @param tDepth
 * @return in case of an isosurface, result is blended with the color of the isosurface.
 *       Otherwise result is returned
 */
vec4 drawISO(in vec4 result, in float isovalue, in vec4 isocolor, in float value,
             in float previousValue, in vec3 gradient, in vec3 previousGradient,
             in mat4 textureToWorld, in LightParameters lighting, in vec3 rayPosition,
             in vec3 rayDirection, in vec3 toCameraDir, in float t, in float tIncr,
             inout float tDepth) {

    // check if the isovalue is lying in between current and previous sample
    // found isosurface if differences between current/prev sample and isovalue have different signs
    if ((isovalue - value) * (isovalue - previousValue) <= 0) {
        // apply linear interpolation between current and previous sample to obtain location of
        // isosurface
        float a = (value - isovalue) / (value - previousValue);
        // if a == 1, isosurface was already computed for previous sampling position
        if (a >= 1.0) return result;

        vec3 isopos = rayPosition - tIncr * a * rayDirection;

        ShadingParameters shadingParams = shading(isocolor.rgb);
        
        #if defined(SHADING_ENABLED) && defined(GRADIENTS_ENABLED)
        vec3 isoGradient = mix(gradient, previousGradient, a);
        shadingParams.normal = -isoGradient;

#if defined(SHADING_NORMAL) && (SHADING_NORMAL == 1)
        // backside shading only
        shadingParams.normal = -shadingParams.normal;
#elif defined(SHADING_NORMAL) && (SHADING_NORMAL == 2)
        // two-sided shading
        if (dot(shadingParams.normal, rayDirection) > 0.0) {
            shadingParams.normal = -shadingParams.normal;
        }
#endif

        shadingParams.worldPosition = (textureToWorld * vec4(isopos, 1.0)).xyz;
        #endif

        isocolor.rgb = applyLighting(lighting, shadingParams, toCameraDir);

        if (tDepth < 0.0) {  // blend isosurface color and adjust first-hit depth if necessary
            tDepth = t - a * tIncr;  // store depth of first hit, i.e. voxel with non-zero alpha
        }
        isocolor.rgb *= isocolor.a;  // use pre-multiplied alpha
        
        // blend isosurface color with result accumulated so far
        result += (1.0 - result.a) * isocolor;
    }

    return result;
}

vec4 drawISO(in vec4 result, in IsovalueParameters isoparams, in float value,
             in float previousValue, in vec3 gradient, in vec3 previousGradient,
             in mat4 textureToWorld, in LightParameters lighting, in vec3 rayPosition,
             in vec3 rayDirection, in vec3 toCameraDir, in float t, in float tIncr,
             inout float tDepth) {

    #if MAX_ISOVALUE_COUNT == 1
    if (isoparams.size > 0 ) {
        result = drawISO(result, isoparams.values[0], isoparams.colors[0], value, previousValue,
                         gradient, previousGradient, textureToWorld, lighting, rayPosition,
                         rayDirection, toCameraDir, t, tIncr, tDepth);
    }
    #else
    // multiple isosurfaces, need to determine order of traversal
    if (value - previousValue > 0) {
        for (int i = 0; i < isoparams.size; ++i) {
            result = drawISO(result, isoparams.values[i], isoparams.colors[i], value, previousValue,
                             gradient, previousGradient, textureToWorld, lighting, rayPosition,
                             rayDirection, toCameraDir, t, tIncr, tDepth);
        }
    } else {
        for (int i = isoparams.size; i > 0; --i) {
            result = drawISO(result, isoparams.values[i - 1], isoparams.colors[i - 1], value,
                             previousValue, gradient, previousGradient, textureToWorld, lighting,
                             rayPosition, rayDirection, toCameraDir, t, tIncr, tDepth);
        }
    }
    #endif

    return result;
}
