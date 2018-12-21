#ifndef DISPENSER_EDIT_H
#define DISPENSER_EDIT_H

#include <QDialog>
#include "unit.h"

namespace Ui {
class dispenser_edit;
}

class dispenser_edit : public QDialog
{
    Q_OBJECT

public:
    explicit dispenser_edit(QString text, int length, int width, QWidget *parent = 0);
    ~dispenser_edit();
    int length;
    int width;
    QString type;
    QString text;

signals:
    void update_this_label(QString, int, int);

private slots:
    void on_enter_clicked();

private:
    Ui::dispenser_edit *ui;
};

#endif // DISPENSER_EDIT_H
