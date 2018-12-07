#ifndef merge_edit_H
#define merge_edit_H

#include <QDialog>
#include "unit.h"

namespace Ui {
class merge_edit;
}

class merge_edit : public QDialog
{
    Q_OBJECT

public:
    explicit merge_edit(QWidget *parent = 0);
    ~merge_edit();
    QString temp_text;
    QString type;
signals:
    void update_this_label(QString);

private slots:
    void on_enter_clicked();

private:
    Ui::merge_edit *ui;
};

#endif // merge_edit_H