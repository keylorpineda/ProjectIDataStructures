#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Proyecto1EstructurasDeDatos.h"

class Proyecto1EstructurasDeDatos : public QMainWindow
{
    Q_OBJECT

public:
    Proyecto1EstructurasDeDatos(QWidget *parent = nullptr);
    ~Proyecto1EstructurasDeDatos();

private:
    Ui::Proyecto1EstructurasDeDatosClass ui;
};

