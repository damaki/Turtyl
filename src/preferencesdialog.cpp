#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

PreferencesDialog::PreferencesDialog(TurtleGraphicsScene* const scene) :
    ui(new Ui::PreferencesDialog),
    m_scene(scene)
{
    ui->setupUi(this);

    m_prefAntialiasing = findChild<QCheckBox*>("antialiasingCheckBox");
    m_prefCanvasSize   = findChild<QSpinBox*>("canvasSizeSpinBox");

    connect(this, SIGNAL(accepted()), this, SLOT(applyPreferences()));
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::loadPreferences()
{
    if (NULL != m_scene)
    {
        if (NULL != m_prefAntialiasing)
        {
            m_prefAntialiasing->setChecked(m_scene->antialiasingEnabled());
        }

        if (NULL != m_prefCanvasSize)
        {
            m_prefCanvasSize->setValue(m_scene->canvasSize());
        }
    }
}

void PreferencesDialog::applyPreferences()
{
    if (NULL != m_scene)
    {
        if (NULL != m_prefAntialiasing)
        {
            m_scene->setAntialiasing(m_prefAntialiasing->checkState() == Qt::Checked);
        }

        if (NULL != m_prefCanvasSize)
        {
            m_scene->setCanvasSize(m_prefCanvasSize->value());
        }

        m_scene->updateScene();
    }
}
