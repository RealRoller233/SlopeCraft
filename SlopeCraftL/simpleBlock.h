/*
 Copyright © 2021-2023  TokiNoBug
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
    along with SlopeCraft. If not, see <https://www.gnu.org/licenses/>.

    Contact with me:
    github:https://github.com/SlopeCraft/SlopeCraft
    bilibili:https://space.bilibili.com/351429231
*/

#ifndef SIMPLEBLOCK_H
#define SIMPLEBLOCK_H

#include "SCLDefines.h"
using namespace SlopeCraft;
#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <set>

typedef unsigned char uchar;

typedef std::vector<std::string> stringList;

class simpleBlock : public ::SlopeCraft::AbstractBlock {
 public:
  simpleBlock();
  ~simpleBlock(){};
  std::string id;
  uchar version;
  std::string idOld;
  std::string nameZH;
  std::string nameEN;
  Eigen::ArrayXX<uint32_t> image;
  bool needGlass;
  bool doGlow;
  bool endermanPickable;
  bool burnable;

  unsigned long long size() const noexcept override {
    return sizeof(simpleBlock);
  }

  const char *getId() const noexcept override { return id.data(); };
  uint8_t getVersion() const noexcept override { return version; };
  const char *getIdOld() const noexcept override { return idOld.data(); };
  bool getNeedGlass() const noexcept override { return needGlass; };
  bool getDoGlow() const noexcept override { return doGlow; };
  bool getEndermanPickable() const noexcept override {
    return endermanPickable;
  };
  bool getBurnable() const noexcept override { return burnable; };
  const char *getNameZH() const noexcept override {
    return this->nameZH.c_str();
  }
  const char *getNameEN() const noexcept override {
    return this->nameEN.c_str();
  }
  void getImage(uint32_t *dest, bool is_row_major) const noexcept override {
    if (is_row_major) {
      Eigen::Map<Eigen::ArrayXX<uint32_t>> map(dest, 16, 16);
      map = this->image;
    } else {
      memcpy(dest, this->image.data(), this->image.size() * sizeof(uint32_t));
    }
  }

  void setId(const char *_id) noexcept override { id = _id; };
  void setVersion(unsigned char _ver) noexcept override { version = _ver; };
  void setIdOld(const char *_idOld) noexcept override { idOld = _idOld; };
  void setNeedGlass(bool _needGlass) noexcept override {
    needGlass = _needGlass;
  };
  void setDoGlow(bool _doGlow) noexcept override { doGlow = _doGlow; };
  void setEndermanPickable(bool _enderman) noexcept override {
    endermanPickable = _enderman;
  };
  void setBurnable(bool _burn) noexcept override { burnable = _burn; };

  void setNameZH(const char *__nzh) noexcept override { this->nameZH = __nzh; }
  void setNameEN(const char *__nen) noexcept override { this->nameEN = __nen; }
  void setImage(const uint32_t *src, bool is_row_major) noexcept override {
    if (is_row_major) {
      Eigen::Map<const Eigen::ArrayXX<uint32_t>> map(src, 16, 16);
      this->image = map;
    } else {
      this->image.resize(16, 16);
      memcpy(this->image.data(), src, 16 * 16 * sizeof(uint32_t));
    }
  }

  void copyTo(AbstractBlock *dst) const noexcept override {
    *static_cast<simpleBlock *>(dst) = *this;
  }

  static bool dealBlockId(const std::string &id, std::string &netBlockId,
                          stringList *proName, stringList *proVal);
  // simpleBlock& operator =(const simpleBlock &);
};

#warning define the impl class of blockclass here

#endif  // SIMPLEBLOCK_H
