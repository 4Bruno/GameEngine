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

#define IMG_WEIGHTED_COLOR 7
#define IMG_WEIGHTED_REVEAL 8

#if 0//OIT_MSAA != 1
layout(input_attachment_index = 0, binding = IMG_WEIGHTED_COLOR) uniform subpassInputMS texColor;
layout(input_attachment_index = 1, binding = IMG_WEIGHTED_REVEAL) uniform subpassInputMS texWeights;
#else
layout(input_attachment_index = 0, binding = IMG_WEIGHTED_COLOR) uniform subpassInput texColor;
layout(input_attachment_index = 1, binding = IMG_WEIGHTED_REVEAL) uniform subpassInput texWeights;
#endif

layout(location = 0) out vec4 outColor;

void main()
{
#if 0//OIT_MSAA != 1
  vec4  accum  = subpassLoad(texColor, gl_SampleID);
  float reveal = subpassLoad(texWeights, gl_SampleID).r;
#else
  vec4 accum = subpassLoad(texColor);
  float reveal = subpassLoad(texWeights).r;
#endif
  // GL blend function: GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA
  outColor = vec4(accum.rgb / max(accum.a, 1e-5), reveal);
}

