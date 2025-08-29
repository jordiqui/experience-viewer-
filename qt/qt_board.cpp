#include "qt_board.h"
#include <QPainter>
#include <QPaintEvent>

namespace {
// Map piece character to index in m_piecePix.
int pieceIndex(char p){
    switch(p){
    case 'P': return 0; case 'N': return 1; case 'B': return 2;
    case 'R': return 3; case 'Q': return 4; case 'K': return 5;
    case 'p': return 6; case 'n': return 7; case 'b': return 8;
    case 'r': return 9; case 'q': return 10; case 'k': return 11;
    default: return -1;
    }
}

QString pieceName(char p){
    switch(p){
    case 'P': return "w_p.png"; case 'N': return "w_n.png"; case 'B': return "w_b.png";
    case 'R': return "w_r.png"; case 'Q': return "w_q.png"; case 'K': return "w_k.png";
    case 'p': return "b_p.png"; case 'n': return "b_n.png"; case 'b': return "b_b.png";
    case 'r': return "b_r.png"; case 'q': return "b_q.png"; case 'k': return "b_k.png";
    default: return QString();
    }
}
}

QtBoard::QtBoard(QWidget *parent) : QWidget(parent) {
    setMinimumSize(200,200);
    m_state.set_startpos();
}

void QtBoard::setAssetsDir(const QString &dir){
    m_assetsDir = dir;
    loadPieces();
    update();
}

void QtBoard::setPosition(const BoardState &st){
    m_state = st;
    update();
}

void QtBoard::loadPieces(){
    for(int i=0;i<12;++i) m_piecePix[i] = QPixmap();
    for(int r=0;r<64;++r){
        char c = m_state.squares[r];
        int idx = pieceIndex(c);
        if(idx>=0 && m_piecePix[idx].isNull()){
            m_piecePix[idx].load(m_assetsDir + "/" + pieceName(c));
        }
    }
}

void QtBoard::paintEvent(QPaintEvent *ev){
    Q_UNUSED(ev);
    QPainter p(this);
    int sz = qMin(width(), height());
    int cell = sz/8;
    int offx = (width()-sz)/2;
    int offy = (height()-sz)/2;

    QColor light(240,217,181);
    QColor dark(181,136,99);
    for(int r=0;r<8;++r){
        for(int f=0;f<8;++f){
            bool darksq = ((r+f)&1);
            QRect rect(offx+f*cell, offy+(7-r)*cell, cell, cell);
            p.fillRect(rect, darksq?dark:light);
        }
    }

    for(int r=0;r<8;++r){
        for(int f=0;f<8;++f){
            char c = m_state.squares[r*8+f];
            int idx = pieceIndex(c);
            if(idx<0) continue;
            const QPixmap &px = m_piecePix[idx];
            if(!px.isNull()){
                QRect rect(offx+f*cell+2, offy+(7-r)*cell+2, cell-4, cell-4);
                p.drawPixmap(rect, px);
            }
        }
    }
}
