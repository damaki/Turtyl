#include "canvassaveoptionsdialog.h"
#include "ui_canvassaveoptionsdialog.h"

CanvasSaveOptionsDialog::CanvasSaveOptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CanvasSaveOptionsDialog)
{
    ui->setupUi(this);
}

CanvasSaveOptionsDialog::~CanvasSaveOptionsDialog()
{
    delete ui;
}

bool CanvasSaveOptionsDialog::fitToUsedArea() const
{
    return ui->fitToDrawingCheckBox->checkState() == Qt::Checked;
}

void CanvasSaveOptionsDialog::setFitToUsedArea(bool on)
{
    ui->fitToDrawingCheckBox->setChecked(on);
}

bool CanvasSaveOptionsDialog::transparentBackground() const
{
    return ui->transparentBackgroundCheckBox->checkState() == Qt::Checked;
}

void CanvasSaveOptionsDialog::setTransparentBackground(bool on)
{
    ui->transparentBackgroundCheckBox->setChecked(on);
}
