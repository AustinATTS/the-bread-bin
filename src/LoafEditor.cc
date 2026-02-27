#include "LoafEditor.h"

#include "LoafItem.h"

namespace BreadBin {
LoafEditor::LoafEditor() : current_loaf_(nullptr), unsaved_changes_(false) {}

LoafEditor::~LoafEditor() { CloseLoaf(); }

void LoafEditor::NewLoaf(const std::string& name, bool mark_unsaved) {
  current_loaf_ = std::make_shared<Loaf>(name);
  unsaved_changes_ = mark_unsaved;
}

bool LoafEditor::OpenLoaf(const std::string& filepath) {
  auto loaf = std::make_shared<Loaf>();
  if (loaf->Load(filepath)) {
    current_loaf_ = loaf;
    unsaved_changes_ = false;
    return true;
  }
  return false;
}

bool LoafEditor::SaveLoaf(const std::string& filepath) {
  if (!current_loaf_) {
    return false;
  }

  if (current_loaf_->Save(filepath)) {
    unsaved_changes_ = false;
    return true;
  }
  return false;
}

bool LoafEditor::CloseLoaf() {
  current_loaf_.reset();
  unsaved_changes_ = false;
  return true;
}

bool LoafEditor::AddApplication(const std::string& id, const std::string& name,
                                const std::string& path) {
  if (!current_loaf_) {
    return false;
  }

  auto item = std::make_shared<ApplicationItem>(id);
  item->SetName(name);
  item->SetPath(path);
  current_loaf_->AddItem(item);
  unsaved_changes_ = true;
  return true;
}

bool LoafEditor::AddFile(const std::string& id, const std::string& name,
                         const std::string& path) {
  if (!current_loaf_) {
    return false;
  }

  auto item = std::make_shared<FileItem>(id);
  item->SetName(name);
  item->SetPath(path);
  current_loaf_->AddItem(item);
  unsaved_changes_ = true;
  return true;
}

bool LoafEditor::AddConfig(const std::string& id, const std::string& name,
                           const std::string& path) {
  if (!current_loaf_) {
    return false;
  }

  auto item = std::make_shared<ConfigItem>(id);
  item->SetName(name);
  item->SetPath(path);
  current_loaf_->AddItem(item);
  unsaved_changes_ = true;
  return true;
}

bool LoafEditor::AddScript(const std::string& id, const std::string& name,
                           const std::string& path) {
  if (!current_loaf_) {
    return false;
  }

  auto item = std::make_shared<ScriptItem>(id);
  item->SetName(name);
  item->SetPath(path);
  current_loaf_->AddItem(item);
  unsaved_changes_ = true;
  return true;
}

bool LoafEditor::AddWebPage(const std::string& id, const std::string& name,
                            const std::string& url) {
  if (!current_loaf_) {
    return false;
  }

  auto item = std::make_shared<WebPageItem>(id);
  item->SetName(name);
  item->SetPath(url);
  current_loaf_->AddItem(item);
  unsaved_changes_ = true;
  return true;
}

bool LoafEditor::RemoveItem(const std::string& itemId) {
  if (!current_loaf_) {
    return false;
  }

  current_loaf_->RemoveItem(itemId);
  unsaved_changes_ = true;
  return true;
}

bool LoafEditor::SetLoafName(const std::string& name) {
  if (!current_loaf_) {
    return false;
  }

  current_loaf_->SetName(name);
  unsaved_changes_ = true;
  return true;
}

bool LoafEditor::SetLoafDescription(const std::string& description) {
  if (!current_loaf_) {
    return false;
  }

  current_loaf_->SetDescription(description);
  unsaved_changes_ = true;
  return true;
}

bool LoafEditor::SetLoafLayout(const std::string& layout) {
  if (!current_loaf_) {
    return false;
  }

  current_loaf_->SetLayout(layout);
  unsaved_changes_ = true;
  return true;
}

bool LoafEditor::SetRuntimeRule(const std::string& key,
                                const std::string& value) {
  if (!current_loaf_) {
    return false;
  }

  current_loaf_->SetRuntimeRule(key, value);
  unsaved_changes_ = true;
  return true;
}

std::shared_ptr<Loaf> LoafEditor::GetCurrentLoaf() const {
  return current_loaf_;
}

bool LoafEditor::HasUnsavedChanges() const { return unsaved_changes_; }
}  // namespace BreadBin