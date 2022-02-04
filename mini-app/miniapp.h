#pragma once

#include <QtWidgets/QMainWindow>
#include <qsettings.h>

#include "ui_miniapp.h"

class miniapp : public QMainWindow
{
    Q_OBJECT

public:
    miniapp(QWidget *parent = Q_NULLPTR);

private:
    void closeEvent(QCloseEvent* event) override;

    std::tuple<
        QLineEdit*,
        QLineEdit*,
        QLineEdit*,
        QLineEdit*,
        QLineEdit*,
        QLineEdit*,
        QLineEdit*,
        QLineEdit*,
        QCheckBox*
    > find_widgets();

    void read_fields_and_calc_q();
    void load_settings();
    void enable_speed(bool);

    QSettings create_settings_obj();

private:
    Ui::miniappClass ui;
};
