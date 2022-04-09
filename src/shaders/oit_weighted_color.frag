/*
 * Copyright (c) 2020-2021, NVIDIA CORPORATION.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-FileCopyrightText: Copyright (c) 2020-2021 NVIDIA CORPORATION
 * SPDX-License-Identifier: Apache-2.0
 */


// Implements Weighted, Blended Order-Independent Transparency,
// from http://casual-effects.blogspot.de/2014/03/weighted-blended-order-independent.html.
// This is an approximate order-independent transparency method.
// The idea is that we assign each fragment an arbitrarily chosen weight,
// here based on its depth, transparency, and color. Then we compute the
// following quantities, where color0, color1, ... are premultiplied:
// outColor: (weight0 * color0.rgba) + (weight1 * color1.rgba) + ...
//   (i.e. the weighted premultiplied sum, and)
// outReveal: (1-color0.a) * (1-color1.a) * ...
//   (i.e. 1 minus the opacity of the result).
// Then in the resolve pass, get the average weighted RGB color,
// outColor.rgb/outColor.a, and blend it onto the image with the opacity
// of the result. There's one more trick here; assuming it's being blended
// onto an opaque surface, we can use the GL blend mode
// GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA
// using outReveal (instead of 1-outReveal) as the alpha channel to blend
// onto the image.

#version 460
#extension GL_GOOGLE_include_directive : enable

#include "shader_common.h"


layout (set = 0, binding = 0) uniform SimulationBuffer
{
    simulation_data Data;

} Simulation;

layout(location = 0) in interpolants IN;

layout(location = 0) out vec4 outColor;
layout(location = 1) out float outReveal;

// Gooch shading!
// Interpolates between white and a cooler color based on the angle
// between the normal and the light.
vec3 goochLighting(vec3 normal)
{
  // Light direction
  vec3 light = normalize(vec3(-1, 2, 1));
  // cos(theta), remapped into [0,1]
  float warmth = dot(normalize(normal), light) * 0.5 + 0.5;
  // Interpolate between warm and cool colors (alpha here will be ignored)
  return mix(vec3(0, 0.25, 0.75), vec3(1, 1, 1), warmth);
}

// Applies Gooch shading to a surface with color and alpha and returns
// an unpremultiplied RGBA color.
vec4 shading(const interpolants its)
{
  vec3 colorRGB = its.Color.rgb * goochLighting(its.Normal);

  // Calculate transparency in [alphaMin, alphaMin+alphaWidth]
  //float alpha = clamp(scene.alphaMin + its.color.a * scene.alphaWidth, 0, 1);
  float alpha = clamp(0.0f + its.Color.a * 1.0f, 0, 1);

  return vec4(colorRGB, alpha);
}

void main()
{
  vec4 color = shading(IN);
  color.rgb *= color.a;  // Premultiply it

  // Insert your favorite weighting function here. The color-based factor
  // avoids color pollution from the edges of wispy clouds. The z-based
  // factor gives precedence to nearer surfaces.

  // The depth functions in the paper want a camera-space depth of 0.1 < z < 500,
  // but the scene at the moment uses a range of about 0.01 to 50, so multiply
  // by 10 to get an adjusted depth:
  const float depthZ = -IN.Depth * 10.0f;

  const float distWeight = clamp(0.03 / (1e-5 + pow(depthZ / 200, 4.0)), 1e-2, 3e3);

  float alphaWeight = min(1.0, max(max(color.r, color.g), max(color.b, color.a)) * 40.0 + 0.01);
  alphaWeight *= alphaWeight;

  const float weight = alphaWeight * distWeight;

  // GL Blend function: GL_ONE, GL_ONE
  outColor = color * weight;

  // GL blend function: GL_ZERO, GL_ONE_MINUS_SRC_ALPHA
  outReveal = color.a;
}


