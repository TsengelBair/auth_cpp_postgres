#include "mainwindow.h"
#include "dbsettings.h"

#include <QApplication>
#include <QtSql>
#include <QDebug>

bool createConnection(){
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL"); // выбираем драйвер postgres и определяем параметры
    db.setHostName(DBSettings::host());
    db.setDatabaseName(DBSettings::databaseName());
    db.setUserName(DBSettings::username());
    db.setPassword(DBSettings::password());

    if (!db.open()) {
        qDebug() << "Cannot connect to database:" << db.lastError().text();
        return false;
    }
    qDebug() << "Success connection";
    return true;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (!createConnection()) {
        return 1; // Завершаем приложение, если подключение не удалось
    }

    MainWindow w;
    w.show();
    return a.exec();
}
