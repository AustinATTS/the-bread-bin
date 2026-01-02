#include <breadbin/LoafFile.hpp>
#include <yaml-cpp/yaml.h>
#include <sstream>

namespace breadbin::core {
    std::string LoafFile::get_summary() const {
        std::stringstream ss;
        ss << "App: " << app_name << " (" << args.size() << " arguments) ";
        return ss.str();
    }
} // namespace breadbin::core