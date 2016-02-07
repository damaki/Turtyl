#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QCheckBox>
#include <QDialog>
#include <QSpinBox>
#include "turtlegraphicsscene.h"

namespace Ui
{
class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    PreferencesDialog(TurtleGraphicsScene* scene);
    ~PreferencesDialog();

public slots:
    void loadPreferences();
    void applyPreferences();

private:
    Ui::PreferencesDialog* ui;

    TurtleGraphicsScene* m_scene;

    QCheckBox* m_prefAntialiasing;
    QSpinBox* m_prefCanvasSize;
};

#endif // PREFERENCESDIALOG_H
