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

#ifndef SCHEM_SCHEM_H
#define SCHEM_SCHEM_H

#include <MCDataVersion.h>
#include <string>
#include <string_view>
#include <type_traits>
#include <unsupported/Eigen/CXX11/Tensor>
#include <vector>
#include <span>
#include <cereal/access.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <exception>

#include "SC_GlobalEnums.h"

namespace libSchem {
// template <int64_t max_block_count = 256>

struct litematic_info {
  litematic_info();
  std::string litename_utf8{"Litematic generated by SlopeCraft."};
  std::string regionname_utf8{"Software developer : TokiNoBug."};
  std::string author_utf8{"SlopeCraft"};
  std::string destricption_utf8{"This litematic is generated by SlopeCraft."};
  uint64_t time_created;   //< Miliseconds since 1970
  uint64_t time_modified;  //< Miliseconds since 1970
};

struct WorldEditSchem_info {
  WorldEditSchem_info();
  std::array<int, 3> offset{0, 0, 0};
  std::array<int, 3> WE_offset{0, 0, 0};
  std::string schem_name_utf8{
      "WorldEdit schem generated by SlopeCraft, deveploer TokiNoBug."};
  std::string author_utf8{"SlopeCraft"};
  std::vector<std::string> required_mods_utf8{};
  uint64_t date;  //< Miliseconds since 1970
};

class Schem {
 public:
  // using ele_t = std::conditional_t<(max_block_count > 256), uint16_t,
  // uint8_t>;
  using ele_t = uint16_t;

  static constexpr ele_t invalid_ele_t = ~ele_t(0);

 private:
  /// The 3 indices are stored in [x][z][y] col-major, and in minecraft the
  /// best storage is [y][z][x] row-major
  Eigen::Tensor<ele_t, 3> xzy;

  std::vector<std::string> block_id_list;

  ::SCL_gameVersion MC_major_ver;
  MCDataVersion::MCDataVersion_t MC_data_ver;

 public:
  Schem() { xzy.resize(0, 0, 0); }
  Schem(int64_t x, int64_t y, int64_t z) {
    xzy.resize(x, y, z);
    xzy.setZero();
  }

  std::string check_size() const noexcept;
  static std::string check_size(int64_t x, int64_t y, int64_t z) noexcept;

  inline void set_zero() noexcept { xzy.setZero(); }

  inline ele_t *data() noexcept { return xzy.data(); }

  inline const ele_t *data() const noexcept { return xzy.data(); }

  void resize(int64_t x, int64_t y, int64_t z) noexcept;

  inline bool check_version_id() const noexcept {
    return MCDataVersion::is_data_version_suitable(this->MC_major_ver,
                                                   this->MC_data_ver);
  }

  inline const auto &tensor() const noexcept { return this->xzy; }

  inline const auto &palette() const noexcept { return this->block_id_list; }

  inline ele_t &operator()(int64_t x, int64_t y, int64_t z) noexcept {
    assert(x >= 0 && x < this->x_range());
    assert(y >= 0 && y < this->y_range());
    assert(z >= 0 && z < this->z_range());
    return xzy(x, z, y);
  }

  inline const ele_t &operator()(int64_t x, int64_t y,
                                 int64_t z) const noexcept {
    assert(x >= 0 && x < this->x_range());
    assert(y >= 0 && y < this->y_range());
    assert(z >= 0 && z < this->z_range());
    return xzy(x, z, y);
  }

  inline ele_t &operator()(int64_t idx) noexcept {
    assert(idx >= 0 && idx < this->size());
    return xzy(idx);
  }

  inline const ele_t &operator()(int64_t idx) const noexcept {
    assert(idx >= 0 && idx < this->size());
    return xzy(idx);
  }

  inline const char *id_at(int64_t x, int64_t y, int64_t z) const noexcept {
    assert(x >= 0 && x < this->x_range());
    assert(y >= 0 && y < this->y_range());
    assert(z >= 0 && z < this->z_range());
    return block_id_list[xzy(x, z, y)].c_str();
  }

  inline const char *id_at(int64_t idx) const noexcept {
    assert(idx >= 0 && idx < this->size());
    return block_id_list[xzy(idx)].c_str();
  }

  inline ele_t *begin() noexcept { return xzy.data(); }

  inline const ele_t *begin() const noexcept { return xzy.data(); }

  inline ele_t *end() noexcept { return xzy.data() + xzy.size(); }
  inline const ele_t *end() const noexcept { return xzy.data() + xzy.size(); }

  inline void fill(const ele_t _) noexcept {
    for (uint16_t &val : *this) {
      val = _;
    }
  }

  inline int64_t x_range() const noexcept { return xzy.dimension(0); }
  inline int64_t y_range() const noexcept { return xzy.dimension(2); }
  inline int64_t z_range() const noexcept { return xzy.dimension(1); }

  inline int palette_size() const noexcept { return block_id_list.size(); }

  inline int64_t size() const noexcept { return xzy.size(); }

  int64_t non_zero_count() const noexcept;

  /// Schem will copy these strings
  void set_block_id(const char *const *const block_ids, const int num) noexcept;
  void set_block_id(std::span<std::string_view> id) noexcept;

  inline MCDataVersion::MCDataVersion_t MC_version_number() const noexcept {
    return this->MC_data_ver;
  }

  inline void set_MC_version_number(
      const MCDataVersion::MCDataVersion_t _) noexcept {
    this->MC_data_ver = _;
  }

  inline ::SCL_gameVersion MC_major_version_number() const noexcept {
    return this->MC_major_ver;
  }

  inline void set_MC_major_version_number(const ::SCL_gameVersion _) noexcept {
    this->MC_major_ver = _;
  }

  /**
   * \brief Search for invalid block.
   *
   * \note Invalid block are blocks which has an id(uint16_t) greater than or
   * equal to the block type number. For example, if there are only 5 kinds of
   * blocks, then id 5 and greater are invalid.
   *
   * \param first_invalid_block_idx The index of the first invalid block
   * \return true The schem contains invalid block(s)
   * \return false The schem don't contain any invalid block.
   */
  bool have_invalid_block(
      int64_t *first_invalid_block_idx = nullptr) const noexcept;

  /**
   * \brief Search for invalid block.
   *
   * \param first_invalid_block_x_pos The x position of the first invalid block
   * \param first_invalid_block_y_pos The y position of the first invalid block
   * \param first_invalid_block_z_pos The z position of the first invalid block
   * \return true The schem contains invalid block(s)
   * \return false The schem don't contain any invalid block.
   */
  bool have_invalid_block(int64_t *first_invalid_block_x_pos,
                          int64_t *first_invalid_block_y_pos,
                          int64_t *first_invalid_block_z_pos) const noexcept;

  void process_mushroom_states() noexcept;

 public:
  bool export_litematic(std::string_view filename,
                        const litematic_info &info = litematic_info(),
                        SCL_errorFlag *const error_flag = nullptr,
                        std::string *const error_str = nullptr) const noexcept;

  bool export_structure(std::string_view filename,
                        const bool is_air_structure_void = true,
                        SCL_errorFlag *const error_flag = nullptr,
                        std::string *const error_str = nullptr) const noexcept;

  bool export_WESchem(std::string_view filename,
                      const WorldEditSchem_info &info = WorldEditSchem_info(),
                      SCL_errorFlag *const error_flag = nullptr,
                      std::string *const error_str = nullptr) const noexcept;

 private:
  friend class cereal::access;

  template <class archive>
  void save(archive &ar) const {
    ar(this->MC_major_ver);
    ar(this->MC_data_ver);
    ar(this->block_id_list);
    const int64_t x{this->x_range()}, y{this->y_range()}, z{this->z_range()};
    ar(x, y, z);
    ar(cereal::binary_data(this->data(), this->size()));
  }

  template <class archive>
  void load(archive &ar) {
    ar(this->MC_major_ver);
    ar(this->MC_data_ver);
    ar(this->block_id_list);
    int64_t x, y, z;
    ar(x, y, z);
    {
      std::string err = check_size(x, y, z);
      if (!err.empty()) {
        throw std::runtime_error{err};
      }
    }
    this->resize(x, y, z);
    ar(cereal::binary_data(this->data(), this->size()));
  }
};

}  // namespace libSchem

#endif  // SCHEM_SCHEM_H