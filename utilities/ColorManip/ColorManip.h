/*
 Copyright © 2021-2022  TokiNoBug
This file is part of SlopeCraft.

    SlopeCraft is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SlopeCraft is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SlopeCraft.  If not, see <https://www.gnu.org/licenses/>.

    Contact with me:
    github:https://github.com/ToKiNoBug
    bilibili:https://space.bilibili.com/351429231
*/

#ifndef COLORMANIP_COLORMANIP_H
#define COLORMANIP_COLORMANIP_H

#include <stdint.h>
//#include <Eigen/Dense>

using ARGB = uint32_t;
// using Eigen::Dynamic;
// using EImage = Eigen::Array<ARGB, Dynamic, Dynamic>;

// void f(float &) noexcept ;
// void invf(float &) noexcept ;
void RGB2HSV(float, float, float, float &, float &, float &) noexcept;
void HSV2RGB(float, float, float, float &, float &, float &) noexcept;
void RGB2XYZ(float R, float G, float B, float &X, float &Y, float &Z) noexcept;
void XYZ2Lab(float X, float Y, float Z, float &L, float &a, float &b) noexcept;
void Lab2XYZ(float L, float a, float b, float &X, float &Y, float &Z) noexcept;

float Lab00_diff(float, float, float, float, float, float) noexcept;

// float squeeze01(float) noexcept;
ARGB RGB2ARGB(float, float, float) noexcept;
ARGB HSV2ARGB(float, float, float) noexcept;
ARGB XYZ2ARGB(float, float, float) noexcept;
ARGB Lab2ARGB(float, float, float) noexcept;
ARGB ComposeColor(const ARGB front, const ARGB back) noexcept;

inline ARGB ARGB32(uint32_t r, uint32_t g, uint32_t b,
                   uint32_t a = 255) noexcept {
  return ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | ((b & 0xFF)) | (a << 24);
}

inline uint8_t getR(ARGB argb) noexcept { return (argb & 0x00FF0000) >> 16; }
inline uint8_t getG(ARGB argb) noexcept { return (argb & 0x0000FF00) >> 8; }
inline uint8_t getB(ARGB argb) noexcept { return argb & 0xFF; }
inline uint8_t getA(ARGB argb) noexcept { return argb >> 24; }

float color_diff_RGB_plus(const float r1, const float g1, const float b1,
                          const float r2, const float g2,
                          const float b2) noexcept;

#endif