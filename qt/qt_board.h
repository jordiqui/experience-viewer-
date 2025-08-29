#pragma once
#include <QWidget>
#include "board.h"

// Simple QWidget-based chessboard that draws PNG sprites from an assets
// directory. The widget is intentionally lightweight so it can be embedded
// in tests or standalone tools.
class QtBoard : public QWidget {
public:
    explicit QtBoard(QWidget *parent = nullptr);

    // Set directory containing piece sprites named like w_p.png, b_k.png ...
    void setAssetsDir(const QString &dir);

    // Update the position displayed by the board.
    void setPosition(const BoardState &st);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    BoardState m_state;              // current board position
    QString m_assetsDir;             // directory with images
    QPixmap m_piecePix[12];          // cached piece images
    void loadPieces();
};
