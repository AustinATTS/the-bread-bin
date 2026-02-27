#include "LoafItem.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <utility>

namespace BreadBin {

LoafItem::LoafItem(std::string id, Type type)
    : id_(std::move(id)), type_(type), name_(""), path_("") {}

LoafItem::~LoafItem() = default;

std::string LoafItem::GetId() const { return id_; }

LoafItem::Type LoafItem::GetType() const { return type_; }

void LoafItem::SetName(const std::string& name) { name_ = name; }

std::string LoafItem::GetName() const { return name_; }

void LoafItem::SetPath(const std::string& path) { path_ = path; }

std::string LoafItem::GetPath() const { return path_; }

void LoafItem::SetMetadata(const std::string& key, const std::string& value) {
  metadata_[key] = value;
}

std::string LoafItem::GetMetadata(const std::string& key) const {
  auto it = metadata_.find(key);
  return (it != metadata_.end()) ? it->second : "";
}

std::string LoafItem::ToString() const {
  std::ostringstream oss;
  oss << "LoafItem[id=" << id_ << ", name=" << name_ << ", path=" << path_
      << "]";
  return oss.str();
}

ApplicationItem::ApplicationItem(const std::string& id)
    : LoafItem(id, Type::APPLICATION) {}

bool ApplicationItem::Execute() {
  if (path_.empty()) {
    return false;
  }

  const std::string arguments = GetMetadata("args");
  const std::string working_dir = GetMetadata("working_dir");

#ifdef _WIN32
  std::string command = "start \"\" \"" + path_ + "\"";
  if (!arguments.empty()) {
    command += " " + arguments;
  }
  return std::system(command.c_str()) == 0;
#elif __APPLE__
  std::string command;
  if (!working_dir.empty()) {
    command = "cd \"" + working_dir + "\" && ";
  }
  command += "open \"" + path_ + "\"";
  if (!arguments.empty()) {
    command += " --args " + arguments;
  }
  return std::system(cmd.c_str()) == 0;
#else
  std::string command;
  if (!working_dir.empty()) {
    command = "cd \"" + working_dir + "\" && ";
  }
  command += "\"" + path_ + "\"";
  if (!arguments.empty()) {
    command += " " + arguments;
  }
  command += " >/dev/null 2>&1 &";
  return std::system(command.c_str()) == 0;
#endif
}

bool ApplicationItem::Validate() const {
  std::ifstream file(path_);
  return file.good();
}

FileItem::FileItem(const std::string& id) : LoafItem(id, Type::FILE) {}

bool FileItem::Execute() {
  if (path_.empty()) {
    return false;
  }
#ifdef _WIN32
  std::string command = "start \"\" \"" + path_ + "\"";
  return std::system(command.c_str()) == 0;
#elif __APPLE__
  std::string command = "open \"" + path_ + "\"";
  return std::system(command.c_str()) == 0;
#else
  std::string command = "xdg-open \"" + path_ + "\"";
  return std::system(command.c_str()) == 0;
#endif
}

bool FileItem::Validate() const {
  std::ifstream file(path_);
  return file.good();
}

ConfigItem::ConfigItem(const std::string& id) : LoafItem(id, Type::CONFIG) {}

bool ConfigItem::Execute() { return Validate(); }

bool ConfigItem::Validate() const {
  std::ifstream file(path_);
  return file.good();
}

ScriptItem::ScriptItem(const std::string& id) : LoafItem(id, Type::SCRIPT) {}

bool ScriptItem::Execute() {
  if (path_.empty()) {
    return false;
  }
  std::string command = "\"" + path_ + "\"";
  return std::system(command.c_str()) == 0;
}

bool ScriptItem::Validate() const {
  std::ifstream file(path_);
  return file.good();
}

WebPageItem::WebPageItem(const std::string& id) : LoafItem(id, Type::WEBPAGE) {}

bool WebPageItem::Execute() {
  if (path_.empty()) {
    return false;
  }

  if (!Validate()) {
    return false;
  }

  for (char c : path_) {
    if (!isalnum(c) && c != ':' && c != '/' && c != '.' && c != '-' &&
        c != '_' && c != '?' && c != '=' && c != '&' && c != '#' && c != '%' &&
        c != '+') {
      return false;
    }
  }

#ifdef _WIN32
  std::string command = "rundll32 url.dll,FileProtocolHandler " + path_;
  return std::system(command.c_str()) == 0;
#elif __APPLE__
  std::string command = "open " + path_;
  return std::system(command.c_str()) == 0;
#else
  std::string command = "xdg-open " + path_;
  return std::system(command.c_str()) == 0;
#endif
}

bool WebPageItem::Validate() const {
  return path_.find("http://") == 0 || path_.find("https://") == 0;
}

}  // namespace BreadBin