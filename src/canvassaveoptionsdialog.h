#ifndef CANVASSAVEOPTIONSDIALOG_H
#define CANVASSAVEOPTIONSDIALOG_H

#include <QDialog>

namespace Ui {
class CanvasSaveOptionsDialog;
}

class CanvasSaveOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CanvasSaveOptionsDialog(QWidget *parent = 0);
    ~CanvasSaveOptionsDialog();

    bool fitToUsedArea() const;
    void setFitToUsedArea(bool on);

    bool transparentBackground() const;
    void setTransparentBackground(bool on);

private:
    Ui::CanvasSaveOptionsDialog *ui;
};

#endif // CANVASSAVEOPTIONSDIALOG_H
