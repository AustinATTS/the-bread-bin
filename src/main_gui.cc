#include <QApplication>
#include "gui/MainWindow.h"

int main (int argument_count, char *argument_vector[]) {
    QApplication app(argument_count, argument_vector);
    
    QApplication::setApplicationName("The Bread Bin");
    QApplication::setApplicationVersion("0.1");
    QApplication::setOrganizationName("All Things Toasty Software Ltd");
    
    BreadBin::GUI::MainWindow main_window;
    main_window.show();
    
    return app.exec();
}