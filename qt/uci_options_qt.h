#pragma once
#include <QDialog>
#include <functional>
#include "uci_options.h"

// Show a modal dialog using Qt to configure UCI options. Returns 1 if the
// user accepted the dialog, 0 otherwise.
int show_uci_options_dialog_qt(QWidget *parent,
                               const std::vector<UciOption> &opts,
                               const std::function<void(const std::string&)> &send,
                               int &out_depth,
                               int &out_movetime_ms);
