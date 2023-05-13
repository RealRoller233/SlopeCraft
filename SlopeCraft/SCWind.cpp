#include "SCWind.h"
#include "ui_SCWind.h"
#include <QFileDialog>
#include <QMessageBox>
#include <ranges>
#include <QApplication>

// #include "PoolWidget.h"
SCWind::SCWind(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::SCWind),
      kernel(SlopeCraft::SCL_createKernel()) {
  this->ui->setupUi(this);
  {
    const auto pid = QApplication::applicationPid();

    const QString sys_cache_dir = QDir::tempPath();
    const QString cache_dir =
        QStringLiteral("%1/SlopeCraft/pid=%2").arg(sys_cache_dir).arg(pid);

    this->kernel->setCacheDir(cache_dir.toLocal8Bit());

    this->kernel->setWindPtr(this);
    this->kernel->setProgressRangeSet(
        [](void *_self, int min, int max, int val) {
          SCWind *const self = reinterpret_cast<SCWind *>(_self);
          QProgressBar *const bar = self->current_bar();
          if (bar == nullptr) return;

          bar->setMinimum(min);
          bar->setMaximum(max);
          bar->setValue(val);
        });
    this->kernel->setProgressAdd([](void *_self, int delta) {
      SCWind *const self = reinterpret_cast<SCWind *>(_self);
      QProgressBar *const bar = self->current_bar();
      if (bar == nullptr) return;

      bar->setValue(bar->value() + delta);
    });

    this->kernel->setKeepAwake([](void *) { QApplication::processEvents(); });
  }
  // initialize cvt pool model
  {
    this->cvt_pool_model = new CvtPoolModel{this, &this->tasks};
    this->ui->lview_pool_cvt->setModel(this->cvt_pool_model);
    this->cvt_pool_model->set_listview(this->ui->lview_pool_cvt);
    connect(this->ui->lview_pool_cvt->selectionModel(),
            &QItemSelectionModel::selectionChanged, this,
            &SCWind::when_cvt_pool_selectionChanged);

    this->export_pool_model = new ExportPoolModel{this, &this->tasks};
    this->ui->lview_pool_export->setModel(this->export_pool_model);
    this->export_pool_model->set_listview(this->ui->lview_pool_export);
    connect(this->ui->lview_pool_export->selectionModel(),
            &QItemSelectionModel::selectionChanged, this,
            &SCWind::when_export_pool_selectionChanged);

    connect(this, &SCWind::image_changed, this->cvt_pool_model,
            &CvtPoolModel::refresh);
    connect(this, &SCWind::image_changed, this->export_pool_model,
            &ExportPoolModel::refresh);
    connect(this, &SCWind::image_changed, [this]() {
      this->ui->lview_pool_cvt->doItemsLayout();
      this->ui->lview_pool_export->doItemsLayout();
    });
  }

  // initialize blm
  {
    this->ui->blm->setup_basecolors(this->kernel);
    this->ui->blm->set_version_callback(
        [this]() { return this->selected_version(); });

    this->ui->blm->add_blocklist("./Blocks/FixedBlocks.json",
                                 "./Blocks/FixedBlocks");
    this->ui->blm->add_blocklist("./Blocks/CustomBlocks.json",
                                 "./Blocks/CustomBlocks");

    this->ui->blm->finish_blocklist();

    for (auto btnp : this->version_buttons()) {
      connect(btnp, &QRadioButton::toggled, this,
              &SCWind::when_version_buttons_toggled);
    }

    for (auto btnp : this->type_buttons()) {
      connect(btnp, &QRadioButton::toggled, this,
              &SCWind::when_type_buttons_toggled);
    }

    connect(this->ui->blm, &BlockListManager::changed, this,
            &SCWind::when_blocklist_changed);
  }

  for (QRadioButton *rbp : this->export_type_buttons()) {
    connect(rbp, &QRadioButton::clicked, this,
            &SCWind::when_export_type_toggled);
  }

  for (QRadioButton *rbp : this->preset_buttons_no_custom()) {
    connect(rbp, &QRadioButton::clicked, this, &SCWind::when_preset_clicked);
  }
  {
    for (QRadioButton *rbp : this->algo_buttons()) {
      connect(rbp, &QRadioButton::clicked, this,
              &SCWind::when_algo_btn_clicked);
    }
    connect(this->ui->cb_algo_dither, &QCheckBox::clicked, this,
            &SCWind::when_algo_btn_clicked);
  }
  // setup presets
  {
    try {
      this->default_presets[0] =
          load_preset("./Blocks/Presets/vanilla.sc_preset_json");
      this->default_presets[1] =
          load_preset("./Blocks/Presets/cheap.sc_preset_json");
      this->default_presets[2] =
          load_preset("./Blocks/Presets/elegant.sc_preset_json");
      this->default_presets[3] =
          load_preset("./Blocks/Presets/shiny.sc_preset_json");
    } catch (std::exception &e) {
      QMessageBox::critical(this, tr("加载默认预设失败"),
                            tr("一个或多个内置的预设不能被解析。SlopeCraft "
                               "可能已经损坏，请重新安装。\n具体报错信息：\n%1")
                                .arg(e.what()));
      abort();
    }
  }

  this->when_preset_clicked();
}

SCWind::~SCWind() {
  delete this->ui;

  {
    const char *cd = this->kernel->cacheDir();
    if (cd != nullptr) {
      QDir cache_dir{QString::fromLocal8Bit(cd)};
      if (cache_dir.exists()) {
        cache_dir.removeRecursively();
      }
    }
  }

  SlopeCraft::SCL_destroyKernel(this->kernel);
}

void SCWind::when_cvt_pool_selectionChanged() noexcept {
  const auto selected_idx = this->selected_cvt_task_idx();

  this->refresh_current_cvt_display(selected_idx);
}

#define SC_SLOPECRAFT_PRIVATEMACRO_VERSION_BUTTON_LIST              \
  {                                                                 \
    this->ui->rb_ver12, this->ui->rb_ver13, this->ui->rb_ver14,     \
        this->ui->rb_ver15, this->ui->rb_ver16, this->ui->rb_ver17, \
        this->ui->rb_ver18, this->ui->rb_ver19, this->ui->rb_ver20  \
  }

std::array<QRadioButton *, 20 - 12 + 1> SCWind::version_buttons() noexcept {
  return SC_SLOPECRAFT_PRIVATEMACRO_VERSION_BUTTON_LIST;
}

std::array<const QRadioButton *, 20 - 12 + 1> SCWind::version_buttons()
    const noexcept {
  return SC_SLOPECRAFT_PRIVATEMACRO_VERSION_BUTTON_LIST;
}

#define SC_SLOPECRAFT_PRIVATEMACRO_TYPE_BUTTON_LIST \
  { this->ui->rb_type_3d, this->ui->rb_type_flat, this->ui->rb_type_fileonly }

std::array<QRadioButton *, 3> SCWind::type_buttons() noexcept {
  return SC_SLOPECRAFT_PRIVATEMACRO_TYPE_BUTTON_LIST;
}

std::array<const QRadioButton *, 3> SCWind::type_buttons() const noexcept {
  return SC_SLOPECRAFT_PRIVATEMACRO_TYPE_BUTTON_LIST;
}

SCL_gameVersion SCWind::selected_version() const noexcept {
  auto btns = this->version_buttons();
  for (size_t idx = 0; idx < btns.size(); idx++) {
    if (btns[idx]->isChecked()) {
      return SCL_gameVersion(idx + 12);
    }
  }

  assert(false);

  return SCL_gameVersion::ANCIENT;
}

SCL_mapTypes SCWind::selected_type() const noexcept {
  if (this->ui->rb_type_3d->isChecked()) {
    return SCL_mapTypes::Slope;
  }

  if (this->ui->rb_type_flat->isChecked()) {
    return SCL_mapTypes::Flat;
  }

  if (this->ui->rb_type_fileonly->isChecked()) {
    return SCL_mapTypes::FileOnly;
  }

  assert(false);
  return {};
}

std::vector<int> SCWind::selected_indices() const noexcept {
  std::vector<int> ret;
  auto sel = this->ui->lview_pool_cvt->selectionModel()->selectedIndexes();
  ret.reserve(sel.size());
  for (auto &midx : sel) {
    ret.emplace_back(midx.row());
  }
  return ret;
}

std::optional<int> SCWind::selected_cvt_task_idx() const noexcept {
  auto sel = this->ui->lview_pool_cvt->selectionModel()->selectedIndexes();
  if (sel.size() <= 0) {
    return std::nullopt;
  }

  return sel.front().row();
}

std::vector<cvt_task *> SCWind::selected_export_task_list() const noexcept {
  auto selected_eidx =
      this->ui->lview_pool_export->selectionModel()->selectedIndexes();
  std::vector<cvt_task *> ret;
  ret.reserve(selected_eidx.size());
  for (auto &midx : selected_eidx) {
    ret.emplace_back(
        this->export_pool_model->export_idx_to_task_ptr(midx.row()));
  }
  return ret;
}
std::optional<cvt_task *> SCWind::selected_export_task() const noexcept {
  auto selected = this->selected_export_task_list();
  if (selected.empty()) {
    return std::nullopt;
  }

  return selected.front();
}

SCL_convertAlgo SCWind::selected_algo() const noexcept {
  if (this->ui->rb_algo_RGB->isChecked()) {
    return SCL_convertAlgo::RGB;
  }
  if (this->ui->rb_algo_RGB_plus->isChecked()) {
    return SCL_convertAlgo::RGB_Better;
  }
  if (this->ui->rb_algo_Lab94->isChecked()) {
    return SCL_convertAlgo::Lab94;
  }
  if (this->ui->rb_algo_Lab00->isChecked()) {
    return SCL_convertAlgo::Lab00;
  }
  if (this->ui->rb_algo_XYZ->isChecked()) {
    return SCL_convertAlgo::XYZ;
  }
  if (this->ui->rb_algo_GACvter->isChecked()) {
    return SCL_convertAlgo::gaCvter;
  }

  assert(false);
  return {};
}

bool SCWind::is_dither_selected() const noexcept {
  return this->ui->cb_algo_dither->isChecked();
}

bool SCWind::is_lossless_compression_selected() const noexcept {
  return this->ui->cb_compress_lossless->isChecked();
}
bool SCWind::is_lossy_compression_selected() const noexcept {
  return this->ui->cb_compress_lossy->isChecked();
}
int SCWind::current_max_height() const noexcept {
  return this->ui->sb_max_height->value();
}

SCL_compressSettings SCWind::current_compress_method() const noexcept {
  if (this->is_lossless_compression_selected()) {
    if (this->is_lossy_compression_selected()) {
      return SCL_compressSettings::Both;
    } else {
      return SCL_compressSettings::NaturalOnly;
    }
  } else {
    if (this->is_lossless_compression_selected()) {
      return SCL_compressSettings::ForcedOnly;
    } else {
      return SCL_compressSettings::noCompress;
    }
  }
}

bool SCWind::is_glass_bridge_selected() const noexcept {
  return this->ui->cb_glass_bridge->isChecked();
}
int SCWind::current_glass_brigde_interval() const noexcept {
  return this->ui->sb_glass_bridge_interval->value();
}
SCL_glassBridgeSettings SCWind::current_glass_method() const noexcept {
  if (this->is_glass_bridge_selected()) {
    return SCL_glassBridgeSettings::withBridge;
  }
  return SCL_glassBridgeSettings::noBridge;
}

bool SCWind::is_fire_proof_selected() const noexcept {
  return this->ui->cb_fireproof->isChecked();
}
bool SCWind::is_enderman_proof_selected() const noexcept {
  return this->ui->cb_enderproof->isChecked();
}
bool SCWind::is_connect_mushroom_selected() const noexcept {
  return this->ui->cb_connect_mushroom->isChecked();
}

SlopeCraft::Kernel::build_options SCWind::current_build_option()
    const noexcept {
  return SlopeCraft::Kernel::build_options{
      .maxAllowedHeight = (uint16_t)this->current_max_height(),
      .bridgeInterval = (uint16_t)this->current_glass_brigde_interval(),
      .compressMethod = this->current_compress_method(),
      .glassMethod = this->current_glass_method(),
      .fire_proof = this->is_fire_proof_selected(),
      .enderman_proof = this->is_enderman_proof_selected(),
      .connect_mushrooms = this->is_connect_mushroom_selected()};
}

SCWind::export_type SCWind::selected_export_type() const noexcept {
  auto btns = this->export_type_buttons();
  static_assert(btns.size() == 5);

  for (int i = 0; i < 5; i++) {
    if (btns[i]->isChecked()) {
      return SCWind::export_type{i};
    }
  }
  assert(false);
  return {};
}

void SCWind::when_version_buttons_toggled() noexcept {
  this->ui->blm->when_version_updated();
  this->when_blocklist_changed();
}

void SCWind::when_type_buttons_toggled() noexcept {
  this->when_blocklist_changed();
  this->update_button_states();
}

void SCWind::when_blocklist_changed() noexcept {
  this->kernel_set_type();
  this->ui->rb_preset_custom->setChecked(true);
  // this->ui->rb_preset_
}

void SCWind::kernel_set_type() noexcept {
  std::vector<uint8_t> a;
  std::vector<const SlopeCraft::AbstractBlock *> b;

  this->ui->blm->get_blocklist(a, b);

  assert(a.size() == b.size());
  if (!this->kernel->setType(this->selected_type(), this->selected_version(),
                             reinterpret_cast<const bool *>(a.data()),
                             b.data())) {
    QMessageBox::warning(this, tr("设置方块列表失败"),
                         tr("您设置的方块列表可能存在错误"));
    return;
  }
  int num_colors{0};
  this->kernel->getAvailableColors(nullptr, nullptr, &num_colors);

  this->ui->lb_avaliable_colors->setText(
      tr("可用颜色数量：%1").arg(num_colors));
}

#define SC_SLOPECRAFT_PREIVATEMACRO_EXPORT_TYPE_BUTTONS                        \
  {                                                                            \
    this->ui->rb_export_lite, this->ui->rb_export_nbt, this->ui->rb_export_WE, \
        this->ui->rb_export_flat_diagram, this->ui->rb_export_fileonly         \
  }

std::array<QRadioButton *, 5> SCWind::export_type_buttons() noexcept {
  return SC_SLOPECRAFT_PREIVATEMACRO_EXPORT_TYPE_BUTTONS;
}
std::array<const QRadioButton *, 5> SCWind::export_type_buttons()
    const noexcept {
  return SC_SLOPECRAFT_PREIVATEMACRO_EXPORT_TYPE_BUTTONS;
}

std::array<QRadioButton *, 4> SCWind::preset_buttons_no_custom() noexcept {
  return {this->ui->rb_preset_vanilla, this->ui->rb_preset_cheap,
          this->ui->rb_preset_elegant, this->ui->rb_preset_shiny};
}

#define SC_SLOPECRAFT_PRIVATEMARCO_ALGO_BUTTONS           \
  {                                                       \
    this->ui->rb_algo_RGB, this->ui->rb_algo_RGB_plus,    \
        this->ui->rb_algo_Lab94, this->ui->rb_algo_Lab00, \
        this->ui->rb_algo_XYZ, this->ui->rb_algo_GACvter  \
  }

std::array<const QRadioButton *, 6> SCWind::algo_buttons() const noexcept {
  return SC_SLOPECRAFT_PRIVATEMARCO_ALGO_BUTTONS;
}

std::array<QRadioButton *, 6> SCWind::algo_buttons() noexcept {
  return SC_SLOPECRAFT_PRIVATEMARCO_ALGO_BUTTONS;
}

QProgressBar *SCWind::current_bar() noexcept {
  const int cid = this->ui->tw_main->currentIndex();
  switch (cid) {
    case 1:
      return this->ui->pbar_cvt;
    case 2:
      return this->ui->pbar_export;
      break;
    default:
      return nullptr;
  }
}

void SCWind::update_button_states() noexcept {
  {
    const bool disable_3d = (this->selected_type() == SCL_mapTypes::FileOnly);
    std::array<QRadioButton *, 3> rb_export_3d_types{this->ui->rb_export_lite,
                                                     this->ui->rb_export_nbt,
                                                     this->ui->rb_export_WE};
    for (auto rbp : rb_export_3d_types) {
      rbp->setDisabled(disable_3d);
    }

    std::array<QPushButton *, 4> pb_export{
        this->ui->pb_export_all, this->ui->pb_build3d,
        this->ui->pb_preview_compress_effect, this->ui->pb_preview_materials};
    for (QPushButton *pbp : pb_export) {
      pbp->setDisabled(disable_3d);
    }

    if (disable_3d) {
      this->ui->rb_export_fileonly->setChecked(true);
    }
  }
  {
    const bool enable_flatdiagram =
        (this->selected_type() == SCL_mapTypes::Flat);

    this->ui->rb_export_flat_diagram->setEnabled(enable_flatdiagram);
  }
}

void SCWind::when_preset_clicked() noexcept {
  int final_idx = -1;

  for (int idx = 0; idx < (int)this->preset_buttons_no_custom().size(); idx++) {
    if (this->preset_buttons_no_custom()[idx]->isChecked()) {
      final_idx = idx;
      break;
    }
  }

  assert(final_idx >= 0);

  if (!this->ui->blm->loadPreset(this->default_presets[final_idx])) {
    QMessageBox::warning(this, tr("应用预设失败"), "");
    return;
  }

  this->preset_buttons_no_custom()[final_idx]->setChecked(true);
}

void SCWind::when_export_type_toggled() noexcept {
  const bool page_3d = !this->ui->rb_export_fileonly->isChecked();
  if (page_3d) {
    this->ui->sw_export->setCurrentIndex(0);
  } else {
    this->ui->sw_export->setCurrentIndex(1);
  }

  const auto btns = this->export_type_buttons();

  for (int idx = 0; idx < 4; idx++) {
    if (btns[idx]->isChecked()) {
      this->ui->tw_export_options->setCurrentIndex(idx);
    }
  }

  this->update_button_states();
}

void SCWind::kernel_set_image(int idx) noexcept {
  assert(idx >= 0);
  assert(idx < (int)this->tasks.size());

  if (this->kernel->queryStep() < SCL_step::wait4Image) {
    this->kernel_set_type();
  }

  const QImage &raw = this->tasks[idx].original_image;
  this->kernel->setRawImage((const uint32_t *)raw.scanLine(0), raw.height(),
                            raw.width(), false);
}

void SCWind::kernel_convert_image() noexcept {
  assert(this->kernel->queryStep() >= SCL_step::convertionReady);

  if (!this->kernel->convert(this->selected_algo(),
                             this->is_dither_selected())) {
    QMessageBox::warning(this, tr("转化图像失败"), tr(""));

    return;
  }
}

void SCWind::kernel_make_cvt_cache() noexcept {
  std::string err;
  err.resize(4096);
  SlopeCraft::StringDeliver sd{err.data(), err.size()};

  if (!this->kernel->saveConvertCache(sd)) {
    QString qerr = QString::fromUtf8(sd.data);
    QMessageBox::warning(this, tr("缓存失败"),
                         tr("未能创建缓存文件，错误信息：\n%1").arg(qerr));
  }
}

QImage SCWind::get_converted_image_from_kernel() const noexcept {
  assert(this->kernel->queryStep() >= SCL_step::converted);

  const int rows = this->kernel->getImageRows();
  const int cols = this->kernel->getImageCols();

  QImage img{cols, rows, QImage::Format_ARGB32};

  this->kernel->getConvertedImage(nullptr, nullptr, (uint32_t *)img.scanLine(0),
                                  false);

  return img;
}

void SCWind::refresh_current_cvt_display(
    std::optional<int> selected_idx,
    bool is_image_coneverted_in_kernel) noexcept {
  if (!selected_idx.has_value()) {
    this->ui->lb_raw_image->setPixmap({});
    this->ui->lb_cvted_image->setPixmap({});
    return;
  }

  const int idx = selected_idx.value();

  this->ui->lb_raw_image->setPixmap(
      QPixmap::fromImage(this->tasks[idx].original_image));

  if (is_image_coneverted_in_kernel) {
    assert(this->kernel->queryStep() >= SCL_step::converted);

    this->ui->lb_cvted_image->setPixmap(
        QPixmap::fromImage(this->get_converted_image_from_kernel()));
    return;
  }

  this->ui->lb_cvted_image->setPixmap({});
  if (!this->tasks[idx].is_converted) {
    return;
  }

  /*
    if (!kernel_check_colorset_hash()) {
      this->mark_all_task_unconverted();
      emit this->image_changed();
      return;
    }*/

  this->kernel_set_image(idx);

  if (!this->kernel->loadConvertCache(this->selected_algo(),
                                      this->is_dither_selected())) {
    this->tasks[idx].set_unconverted();
    emit this->image_changed();
    return;
  }

  this->ui->lb_cvted_image->setPixmap(
      QPixmap::fromImage(this->get_converted_image_from_kernel()));
}

void SCWind::mark_all_task_unconverted() noexcept {
  for (auto &task : this->tasks) {
    task.set_unconverted();
  }
}

void SCWind::when_algo_btn_clicked() noexcept {
  this->refresh_current_cvt_display(this->selected_cvt_task_idx());
}

void SCWind::export_current_cvted_image(int idx, QString filename) noexcept {
  assert(idx >= 0);
  assert(idx < (int)this->tasks.size());

  bool have_image_cvted{false};

  this->kernel_set_image(idx);
  if (!this->kernel->loadConvertCache(this->selected_algo(),
                                      this->is_dither_selected())) {
    const auto ret = QMessageBox::warning(
        this, tr("无法保存第%1个转化后图像").arg(idx + 1),
        tr("该图像未被转化，或者转化之后修改了颜色表/转化算法。请重新转化它。"),
        QMessageBox::StandardButtons{QMessageBox::StandardButton::Ok,
                                     QMessageBox::StandardButton::Ignore});
    if (ret == QMessageBox::StandardButton::Ok) {
      this->kernel_convert_image();
      this->kernel_make_cvt_cache();
      this->tasks[idx].set_converted();
      have_image_cvted = true;
    } else {
      return;
    }
  }

  if (have_image_cvted) {
    emit this->image_changed();
  }

  bool ok = this->get_converted_image_from_kernel().save(filename);
  if (!ok) {
    QMessageBox::warning(
        this, tr("保存图像失败"),
        tr("保存%1时失败。可能是因为文件路径错误，或者图片格式不支持。")
            .arg(filename));
    return;
  }
}

void SCWind::kernel_build_3d() noexcept {
  if (!this->kernel->build(this->current_build_option())) {
    QMessageBox::warning(this, tr("构建三维结构失败"),
                         tr("构建三维结构时，出现错误。可能是因为尝试跳步。"));
    return;
  }
}

void SCWind::refresh_current_build_display(
    std::optional<cvt_task *> taskp, bool is_image_built_in_kernel) noexcept {
  this->ui->lb_show_3dsize->setText(tr("大小："));
  this->ui->lb_show_block_count->setText(tr("方块数量："));
  if (!taskp.has_value()) {
    return;
  }

  int x{-1}, y{-1}, z{-1};
  int64_t block_count{-1};
  {
    if (is_image_built_in_kernel) {
      // the caller garentee that the image is built in kernel
    } else {
      if (taskp.value()->is_built) {
        // try to load convert cache
        if (!this->kernel->loadConvertCache(this->selected_algo(),
                                            this->is_dither_selected())) {
          taskp.value()->set_unconverted();
          return;
        }
        // try to load build cache
        if (!this->kernel->loadBuildCache(this->current_build_option())) {
          taskp.value()->set_unbuilt();
          return;
        }
      } else {
        return;
      }
    }
  }
  this->kernel->get3DSize(&x, &y, &z);
  block_count = this->kernel->getBlockCounts();

  this->ui->lb_show_3dsize->setText(
      tr("大小： %1 × %2 × %3").arg(x).arg(y).arg(z));
  this->ui->lb_show_block_count->setText(tr("方块数量：%1").arg(block_count));
}

void SCWind::when_export_pool_selectionChanged() noexcept {
  this->refresh_current_build_display(this->selected_export_task());
}

void SCWind::kernel_make_build_cache() noexcept {
  std::string err;
  err.resize(4096);
  SlopeCraft::StringDeliver sd{err.data(), err.size()};

  if (!this->kernel->saveBuildCache(sd)) {
    QString qerr = QString::fromUtf8(sd.data);
    QMessageBox::warning(this, tr("缓存失败"),
                         tr("未能创建缓存文件，错误信息：\n%1").arg(qerr));
  }
}

QString extension_of_export_type(SCWind::export_type et) noexcept {
  switch (et) {
    case SCWind::export_type::litematica:
      return "litematic";
    case SCWind::export_type::vanilla_structure:
      return "nbt";
    case SCWind::export_type::WE_schem:
      return "schem";
    case SCWind::export_type::flat_diagram:
      return "png";
    case SCWind::export_type::data_file:
      return "dat";
  }

  assert(false);
  return "Invalid_export_type";
}