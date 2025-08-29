#include "uci_options_qt.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>

int show_uci_options_dialog_qt(QWidget *parent,
                               const std::vector<UciOption> &opts,
                               const std::function<void(const std::string&)> &send,
                               int &out_depth,
                               int &out_movetime_ms)
{
    QDialog dlg(parent);
    dlg.setWindowTitle("UCI Options");

    QVBoxLayout *vbox = new QVBoxLayout(&dlg);
    QFormLayout *form = new QFormLayout();
    vbox->addLayout(form);

    QSpinBox *depth = new QSpinBox(&dlg);
    depth->setMinimum(0); depth->setMaximum(99); depth->setValue(out_depth);
    form->addRow("Analysis depth (0=ignore)", depth);

    QSpinBox *movetime = new QSpinBox(&dlg);
    movetime->setMinimum(0); movetime->setMaximum(600000); movetime->setValue(out_movetime_ms);
    form->addRow("Analysis movetime (ms, 0=ignore)", movetime);

    struct Ctrl { const UciOption *opt; QWidget *w; };
    std::vector<Ctrl> ctrls;

    for(const auto &o : opts){
        if(o.type=="check"){
            QCheckBox *cb = new QCheckBox(QString::fromStdString(o.name), &dlg);
            cb->setChecked(o.def=="true" || o.def=="1");
            form->addRow(cb);
            ctrls.push_back({&o, cb});
        } else if(o.type=="spin"){
            QSpinBox *sp = new QSpinBox(&dlg);
            sp->setMinimum(o.minv); sp->setMaximum(o.maxv); sp->setValue(QString::fromStdString(o.def).toInt());
            form->addRow(QString::fromStdString(o.name), sp);
            ctrls.push_back({&o, sp});
        } else if(o.type=="combo"){
            QComboBox *cb = new QComboBox(&dlg);
            int defIdx = 0; int idx=0;
            for(const auto &v : o.vars){
                cb->addItem(QString::fromStdString(v));
                if(v==o.def) defIdx = idx;
                idx++;
            }
            cb->setCurrentIndex(defIdx);
            form->addRow(QString::fromStdString(o.name), cb);
            ctrls.push_back({&o, cb});
        } else if(o.type=="string"){
            QLineEdit *ed = new QLineEdit(QString::fromStdString(o.def), &dlg);
            form->addRow(QString::fromStdString(o.name), ed);
            ctrls.push_back({&o, ed});
        }
    }

    QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                                Qt::Horizontal, &dlg);
    vbox->addWidget(bb);
    QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    int ret = dlg.exec();
    if(ret==QDialog::Accepted){
        out_depth = depth->value();
        out_movetime_ms = movetime->value();
        for(auto &c : ctrls){
            const UciOption &o = *c.opt;
            if(o.type=="check"){
                bool on = static_cast<QCheckBox*>(c.w)->isChecked();
                send("setoption name " + o.name + " value " + (on?"true":"false"));
            } else if(o.type=="spin"){
                int val = static_cast<QSpinBox*>(c.w)->value();
                send("setoption name " + o.name + " value " + std::to_string(val));
            } else if(o.type=="combo"){
                QString val = static_cast<QComboBox*>(c.w)->currentText();
                send("setoption name " + o.name + " value " + val.toStdString());
            } else if(o.type=="string"){
                QString val = static_cast<QLineEdit*>(c.w)->text();
                send("setoption name " + o.name + " value " + val.toStdString());
            }
        }
        return 1;
    }
    return 0;
}
