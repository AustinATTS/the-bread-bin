#include <iostream>
#include <breadbin/LoafFile.hpp>

int main()
{
    // Create a LOAF object using the defined struct
    breadbin::core::LoafFile my_app;
    my_app.app_name = "The Bread Bin Editor";
    my_app.args = {"--verbose", "--no-save"};

    std::cout << "--- Breadbin GUI Starting ---" << std::endl;

    // Call the function defined in out Core library
    std::cout << "Status: " << my_app.get_summary() << std::endl;

    return 0;
}