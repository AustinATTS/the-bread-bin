#ifndef LOAF_EDITOR_H
#define LOAF_EDITOR_H

#include <string>
#include <memory>
#include "Loaf.h"

namespace BreadBin {

    class LoafEditor {
        public:
            LoafEditor ( );
            ~LoafEditor ( );

            void NewLoaf (const std::string& name);
            bool OpenLoaf (const std::string& filepath);
            bool SaveLoaf (const std::string& filepath);
            bool CloseLoaf ( );

            bool AddApplication (const std::string& id, const std::string& name, const std::string& path);
            bool AddFile (const std::string& id, const std::string& name, const std::string& path);
            bool AddConfig (const std::string& id, const std::string& name, const std::string& path);
            bool AddScript (const std::string& id, const std::string& name, const std::string& path);
            bool AddWebPage (const std::string& id, const std::string& name, const std::string& url);
            bool RemoveItem (const std::string& itemId);

            bool SetLoafName (const std::string& name);
            bool SetLoafDescription (const std::string& description);
            bool SetLoafLayout (const std::string& layout);
            bool SetRuntimeRule (const std::string& key, const std::string& value);

            std::shared_ptr<Loaf> GetCurrentLoaf ( ) const;
            bool HasUnsavedChanges ( ) const;

        private:
            std::shared_ptr<Loaf> current_loaf_;
            bool unsaved_changes_;
        };

} // namespace BreadBin

#endif // LOAF_EDITOR_H