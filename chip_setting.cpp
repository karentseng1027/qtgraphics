#include "chip_setting.h"
#include "ui_chip_setting.h"

chip_setting::chip_setting(QWidget *parent) : QDialog(parent), ui(new Ui::chip_setting)
{
    ui->setupUi(this);
}

chip_setting::~chip_setting()
{
    delete ui;
}

// "open new chip" button clicked
void chip_setting::on_setting_clicked()
{
    if(!ui->chip_length->text().isEmpty()) chip_length_mm = ui->chip_length->text().toFloat();
    if(!ui->chip_width->text().isEmpty()) chip_width_mm = ui->chip_width->text().toFloat();
    if(!ui->chip_border->text().isEmpty()) chip_border_mm = ui->chip_border->text().toFloat();

    if(!ui->cp_length->text().isEmpty()) cp_length_mm = ui->cp_length->text().toFloat();
    if(!ui->cp_width->text().isEmpty()) cp_width_mm = ui->cp_width->text().toFloat();

    if(!ui->de1_length->text().isEmpty()) de1_length_mm = ui->de1_length->text().toFloat();
    if(!ui->de1_width->text().isEmpty()) de1_width_mm = ui->de1_width->text().toFloat();
    if(!ui->de2_length->text().isEmpty()) de2_length_mm = ui->de2_length->text().toFloat();
    if(!ui->de2_width->text().isEmpty()) de2_width_mm = ui->de2_width->text().toFloat();

    if(!ui->de_spacing->text().isEmpty()) de_spacing_mm = ui->de_spacing->text().toFloat();
    if(!ui->cp_spacing->text().isEmpty()) cp_spacing_mm = ui->cp_spacing->text().toFloat();
    if(!ui->line_width->text().isEmpty()) line_width_mm = ui->line_width->text().toFloat();
    emit reset(this);
    this->close();
}

void chip_setting::on_setting_cancel_clicked()
{
    this->close();
}
