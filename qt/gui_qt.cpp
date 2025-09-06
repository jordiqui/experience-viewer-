#include "gui.h"
#if USE_QT && !defined(_WIN32)
#include <QApplication>
#include <QMainWindow>
#include <QListWidget>
#include <QHBoxLayout>
#include <QString>
#include "qt_board.h"

// Minimal Qt based GUI inspired by jfxchess. Displays a chessboard and a
// placeholder move list to demonstrate cross platform GUI support.
int run_gui(HINSTANCE, const wchar_t* assets_dir) {
    int argc = 0;
    char** argv = nullptr;
    QApplication app(argc, argv);

    QMainWindow win;
    QWidget *central = new QWidget(&win);
    QHBoxLayout *layout = new QHBoxLayout(central);

    QtBoard *board = new QtBoard;
    if (assets_dir) {
        board->setAssetsDir(QString::fromWCharArray(assets_dir));
    }
    QListWidget *moves = new QListWidget;

    layout->addWidget(board, 1);
    layout->addWidget(moves);
    win.setCentralWidget(central);
    win.setWindowTitle("Experience Viewer Qt");
    win.resize(800, 600);
    win.show();

    return app.exec();
}
#endif
