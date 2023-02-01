#ifndef SLOPECRAFT_VISUALCRAFTL_PARSERESOURCEPACK_H
#define SLOPECRAFT_VISUALCRAFTL_PARSERESOURCEPACK_H

#include <ColorManip/ColorManip.h>

#include <Eigen/Dense>
#include <string>
#include <unordered_map>

#include "Resource_tree.h"

#include "VisualCraftL.h"
#include <utility>

// struct resource_pack;

/// resize image
Eigen::Array<ARGB, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>
resize_image_nearest(const Eigen::Array<ARGB, Eigen::Dynamic, Eigen::Dynamic,
                                        Eigen::RowMajor> &src,
                     int rows, int cols) noexcept;

Eigen::Array<ARGB, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>
resize_image_nearest(
    const decltype(Eigen::Array<ARGB, Eigen::Dynamic, Eigen::Dynamic,
                                Eigen::RowMajor>()
                       .block(0, 0, 1, 1)) src_block,
    int rows, int cols) noexcept;

Eigen::Array<ARGB, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>
process_dynamic_texture(const Eigen::Array<ARGB, Eigen::Dynamic, Eigen::Dynamic,
                                           Eigen::RowMajor> &src) noexcept;

bool parse_png(
    const void *const data, const int64_t length,
    Eigen::Array<ARGB, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> *img);

std::unordered_map<std::string, Eigen::Array<ARGB, Eigen::Dynamic,
                                             Eigen::Dynamic, Eigen::RowMajor>>
folder_to_images(const zipped_folder &src, bool *const error = nullptr,
                 std::string *const error_string = nullptr) noexcept;

namespace block_model {
constexpr int x_idx = 0;
constexpr int y_idx = 1;
constexpr int z_idx = 2;
constexpr int EW_idx = x_idx;
constexpr int NS_idx = z_idx;
constexpr int UD_idx = y_idx;

using EImgRowMajor_t =
    Eigen::Array<ARGB, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

enum class face_rot : uint8_t {
  face_rot_0 = 0,
  face_rot_90 = 9,
  face_rot_180 = 18,
  face_rot_270 = 27
};

inline face_rot int_to_face_rot(int a) {
  switch (a) {
  case 0:
    return face_rot::face_rot_0;
  case 90:
    return face_rot::face_rot_90;
  case 180:
    return face_rot::face_rot_180;
  case 270:
    return face_rot::face_rot_270;

  default:
    printf("\nFunction int_to_face_rot failed to convert int to face_rot : "
           "invalid value : %i\n",
           a);
    exit(1);
    return face_rot::face_rot_0;
  }
}

using face_idx = ::VCL_face_t;

/// assocate xyz with NWSE and up/down
constexpr face_idx face_x_pos = face_idx::face_east;
constexpr face_idx face_x_neg = face_idx::face_west;
constexpr face_idx face_z_pos = face_idx::face_south;
constexpr face_idx face_z_neg = face_idx::face_north;
constexpr face_idx face_y_pos = face_idx::face_up;
constexpr face_idx face_y_neg = face_idx::face_down;

constexpr inline face_idx inverse_face(const face_idx fi) noexcept {
  switch (fi) {
  case face_idx::face_down:
    return face_idx::face_up;
  case face_idx::face_up:
    return face_idx::face_down;

  case face_idx::face_north:
    return face_idx::face_south;
  case face_idx::face_south:
    return face_idx::face_north;

  case face_idx::face_east:
    return face_idx::face_west;
  case face_idx::face_west:
    return face_idx::face_east;
  }

  return face_idx::face_up;
}

constexpr inline bool is_parallel(const face_idx fiA,
                                  const face_idx fiB) noexcept {
  if (fiA == fiB || fiA == inverse_face(fiB)) {
    return true;
  }
  return false;
}

constexpr inline face_idx rotate_x(face_idx original, face_rot x_rot) noexcept {
  if (original == face_x_neg || original == face_x_pos) {
    return original;
  }

  switch (x_rot) {
  case face_rot::face_rot_0:
    return original;

  case face_rot::face_rot_180:
    return inverse_face(original);

  case face_rot::face_rot_90: {
    switch (original) {
    case face_z_neg:
      return face_y_pos;
    case face_y_pos:
      return face_z_pos;
    case face_z_pos:
      return face_y_neg;
    case face_y_neg:
      return face_z_neg;
    default:
      std::unreachable();
      return {};
    }
  }

  case face_rot::face_rot_270: {
    return inverse_face(rotate_x(original, face_rot::face_rot_90));
  }
  }
  std::unreachable();
  return {};
}

constexpr inline face_idx invrotate_x(face_idx rotated,
                                      face_rot x_rot) noexcept {
  switch (x_rot) {
  case face_rot::face_rot_0:
    return rotated;
  case face_rot::face_rot_180:
    return inverse_face(rotated);
  case face_rot::face_rot_90:
    return rotate_x(rotated, face_rot::face_rot_270);
  case face_rot::face_rot_270:
    return rotate_x(rotated, face_rot::face_rot_90);
  }
  std::unreachable();
  return {};
}

constexpr inline face_idx rotate_y(face_idx original, face_rot y_rot) noexcept {
  if (original == face_y_neg || original == face_y_pos) {
    return original;
  }

  switch (y_rot) {
  case face_rot::face_rot_0:
    return original;
  case face_rot::face_rot_180:
    return inverse_face(original);
  case face_rot::face_rot_90: {
    switch (original) {
    case face_x_pos:
      return face_z_neg;
    case face_z_neg:
      return face_x_neg;
    case face_x_neg:
      return face_z_pos;
    case face_z_pos:
      return face_x_pos;
    default:
      std::unreachable();
      return {};
    }
  }

  case face_rot::face_rot_270:
    return inverse_face(rotate_y(original, face_rot::face_rot_90));
  }

  std::unreachable();
  return {};
}

constexpr inline face_idx invrotate_y(face_idx rotated,
                                      face_rot y_rot) noexcept {
  switch (y_rot) {
  case face_rot::face_rot_0:
    return rotated;
  case face_rot::face_rot_180:
    return inverse_face(rotated);
  case face_rot::face_rot_90:
    return rotate_y(rotated, face_rot::face_rot_270);
  case face_rot::face_rot_270:
    return rotate_y(rotated, face_rot::face_rot_90);
  }
  std::unreachable();
  return {};
}

class ray_t {
public:
  ray_t() = delete;
  explicit ray_t(const face_idx f);
  Eigen::Array3f abc;
  Eigen::Array3f x0y0z0;
};

/// Ax+By+Cz+D=0
class plane_t {
public:
  plane_t() = delete;
  explicit plane_t(const Eigen::Array3f &normVec, const Eigen::Array3f &point)
      : ABC(normVec), D(-(normVec * point).sum()) {}

  Eigen::Array3f ABC;
  float D;
};

class face_t {
public:
  const EImgRowMajor_t *texture{nullptr};
  /// It is not pixel index, but [0,1]*16 stored in integer
  std::array<int16_t, 2> uv_start{0, 0};
  /// It is not pixel index, but [0,1]*16 stored in integer
  std::array<int16_t, 2> uv_end{16, 16};
  face_rot rot{face_rot::face_rot_0};
  bool is_hidden{false};

  inline bool is_uv_whole_texture() const noexcept {
    return (uv_start[0] == 0 && uv_start[1] == 0) &&
           (uv_end[0] == 16 && uv_end[1] == 16);
  }
};

inline ARGB color_at_relative_idx(const EImgRowMajor_t &img, const float r_f,
                                  const float c_f) noexcept {
  const int r_i = std::min<int>(std::max(int(std::floor(r_f * img.rows())), 0),
                                img.rows() - 1);
  const int c_i = std::min<int>(std::max(int(std::floor(c_f * img.cols())), 0),
                                img.cols() - 1);
  /*
  printf("\ncolor_at_relative_idx : r_f = %f, c_f = %f, r_i = %i, c_i = %i",
         r_f, c_f, r_i, c_i);
         */

  return img(r_i, c_i);
}

struct intersect_point {
  float distance;
  std::array<float, 2> uv;
  // Eigen::Array3f coordinate;
  const face_t *face_ptr;

  ///(u,v) in range[0,1]. corresponding to (c,r)

  inline ARGB color() const noexcept {
    // compute uv in range [0,1]
    const float u_in_01 =
        (face_ptr->uv_start[0] +
         uv[0] * (face_ptr->uv_end[0] - face_ptr->uv_start[0])) /
        16;
    const float v_in_01 =
        (face_ptr->uv_start[1] +
         uv[1] * (face_ptr->uv_end[1] - face_ptr->uv_start[1])) /
        16;

    // printf("u_in_")
    /*
    printf("\nintersect_point::color : ");
    printf("uv_start = [%i, %i]; uv_end = [%i, %i]; ", face_ptr->uv_start[0],
           face_ptr->uv_start[1], face_ptr->uv_end[0], face_ptr->uv_end[1]);

    printf("uv = [%f, %f]; ", uv[0], uv[1]);
    printf("u_in_01 = %f, v_in_01 = %f; ", u_in_01, v_in_01);*/
    return color_at_relative_idx(*(face_ptr->texture), v_in_01, u_in_01);
  }
};

/// A 3d box to be displayed
class element {
public:
  Eigen::Array3f _from;
  Eigen::Array3f _to;
  std::array<face_t, 6> faces;

  inline face_t &face(face_idx fi) noexcept { return faces[uint8_t(fi)]; }

  inline const face_t &face(face_idx fi) const noexcept {
    return faces[uint8_t(fi)];
  }

  plane_t plane(face_idx fi) const noexcept;

  inline float x_range() const noexcept { return _to[0] - _from[0]; }
  inline float y_range() const noexcept { return _to[1] - _from[1]; }
  inline float z_range() const noexcept { return _to[2] - _from[2]; }

  inline float x_range_abs() const noexcept { return std::abs(x_range()); }
  inline float y_range_abs() const noexcept { return std::abs(y_range()); }
  inline float z_range_abs() const noexcept { return std::abs(z_range()); }

  inline Eigen::Array3f xyz_minpos() const noexcept {
    return this->_from.min(this->_to);
  }

  inline Eigen::Array3f xyz_maxpos() const noexcept {
    return this->_from.max(this->_to);
  }

  inline float volume() const noexcept {
    return std::abs(x_range() * y_range() * z_range());
  }

  inline int shown_face_num() const noexcept {
    int result = 0;
    for (const auto &f : faces) {
      if (!f.is_hidden)
        result++;
    }
    return result;
  }

  inline bool is_not_outside(const Eigen::Array3f &point) const noexcept {
    return (point >= this->xyz_minpos()).all() &&
           (point <= this->xyz_maxpos()).all();
  }

  void
  intersect_points(const face_idx f, const ray_t &ray,
                   std::vector<intersect_point> *const dest) const noexcept;
};

/// a block model
class model {
public:
  std::vector<element> elements;

  inline void
  get_faces(std::vector<const face_t *> *const dest) const noexcept {
    if (dest == nullptr) {
      return;
    }

    dest->reserve(elements.size() * 6);
    dest->clear();

    for (const element &ele : elements) {
      for (const face_t &f : ele.faces) {
        if (!f.is_hidden) {
          dest->emplace_back(&f);
        }
      }
    }
  }

  EImgRowMajor_t projection_image(face_idx fidx) const noexcept;

  void projection_image(face_idx idx,
                        EImgRowMajor_t *const dest) const noexcept;
};

} // namespace block_model

namespace resource_json {

struct state {
  state() = default;
  explicit state(const char *k, const char *v) {
    this->key = k;
    this->value = v;
  }
  std::string key;
  std::string value;
};

struct model_store_t {
  std::string model_name;
  block_model::face_rot x{block_model::face_rot::face_rot_0};
  block_model::face_rot y{block_model::face_rot::face_rot_0};
  bool uvlock{false};
};

struct model_pass_t {
  model_pass_t() = default;
  explicit model_pass_t(const model_store_t &src) {
    this->model_name = src.model_name.data();
    this->x = src.x;
    this->y = src.y;
    this->uvlock = src.uvlock;
  }

  const char *model_name;
  block_model::face_rot x{block_model::face_rot::face_rot_0};
  block_model::face_rot y{block_model::face_rot::face_rot_0};
  bool uvlock{false};
};

using state_list = std::vector<state>;

bool process_full_id(std::string_view full_id, std::string *namespace_name,
                     std::string *pure_id, state_list *states) noexcept;

/// @return true if sla is equal to slb
bool match_state_list(const state_list &sla, const state_list &slb) noexcept;

class block_states_variant {
public:
  model_pass_t block_model_name(const state_list &sl) const noexcept;

  std::vector<std::pair<state_list, model_store_t>> LUT;
};

struct criteria {
public:
  std::string key;
  std::vector<std::string> values;

  inline bool match(const state &state) const noexcept {
    if (state.key != this->key)
      return false;
    return this->match(state.value);
  }

  inline bool match(std::string_view value) const noexcept {
    for (const std::string &v : this->values) {
      if (v == value)
        return true;
    }
    return false;
  }
};

using criteria_list_and = std::vector<criteria>;

/// @return true if sl matches every criteria in cl
bool match_criteria_list(const criteria_list_and &cl,
                         const state_list &sl) noexcept;

struct multipart_pair {
  model_store_t apply_blockmodel;
  criteria when;
  std::vector<criteria_list_and> when_or;

  inline bool match(const state_list &sl) const noexcept {
    if (when_or.size() <= 0) {
      std::string_view key = when.key;
      const char *slvalue = nullptr;
      for (const state &s : sl) {
        if (s.key == key) {
          slvalue = s.value.data();
          break;
        }
      }
      // if sl don't have a value for the key of criteria, it is considered as
      // mismatch
      if (slvalue == nullptr) {
        return false;
      }

      return when.match(slvalue);
    }

    for (const criteria_list_and &cl : when_or) {
      if (match_criteria_list(cl, sl))
        return true;
    }

    return false;
  }
};

class block_state_multipart {
public:
  std::vector<multipart_pair> pairs;

  std::vector<model_pass_t>
  block_model_names(const state_list &sl) const noexcept;
};

bool parse_block_state(const char *const json_str_beg, const char *const end,
                       block_states_variant *const dest_variant,
                       block_state_multipart *const dest_mutlipart = nullptr,
                       bool *const is_dest_variant = nullptr) noexcept;

} // namespace resource_json

block_model::face_idx string_to_face_idx(std::string_view str,
                                         bool *const _ok) noexcept;

/**
 * \note Name of texture = <namespacename>:blocks/<pngfilename without prefix>
 */
class resource_pack {
public:
  using namespace_name_t = std::string;

  bool add_textures(const zipped_folder &resourece_pack_root,
                    const bool on_conflict_replace_old = true) noexcept;

  bool add_block_models(const zipped_folder &resourece_pack_root,
                        const bool on_conflict_replace_old = true) noexcept;

  bool add_block_states(const zipped_folder &resourece_pack_root,
                        const bool on_conflict_replace_old = true) noexcept;

  inline auto &get_textures() const noexcept { return this->textures; }
  inline auto &get_models() const noexcept { return this->block_models; }
  inline auto &get_block_states() const noexcept { return this->block_states; }

  inline void clear_textures() noexcept { this->textures.clear(); }
  inline void clear_models() noexcept { this->block_models.clear(); }
  inline void clear_block_states() noexcept { this->block_states.clear(); }

  struct buffer_t {
    std::string pure_id;
    // std::vector<std::pair<std::string, std::string>> traits;
    resource_json::state_list state_list;
  };

  const block_model::model *
  find_model(const std::string &block_state_str, VCL_face_t face_exposed,
             VCL_face_t *face_invrotated) const noexcept {
    buffer_t b;
    return this->find_model(block_state_str, face_exposed, face_invrotated, b);
  }

  const block_model::model *find_model(const std::string &block_state_str,
                                       VCL_face_t face_exposed,
                                       VCL_face_t *face_invrotated,
                                       buffer_t &) const noexcept;

  bool compute_projection(const std::string &block_state_str,
                          VCL_face_t face_exposed,
                          block_model::EImgRowMajor_t *const img,
                          buffer_t &) const noexcept;

private:
  std::unordered_map<std::string, block_model::model> block_models;
  std::unordered_map<std::string, Eigen::Array<ARGB, Eigen::Dynamic,
                                               Eigen::Dynamic, Eigen::RowMajor>>
      textures;
  std::unordered_map<std::string, resource_json::block_states_variant>
      block_states;

  bool
  add_textures_direct(const std::unordered_map<std::string, zipped_file> &pngs,
                      std::string_view namespace_name,
                      const bool on_conflict_replace_old) noexcept;
};

class VCL_resource_pack : public resource_pack {};
#endif // SLOPECRAFT_VISUALCRAFTL_PARSERESOURCEPACK_H