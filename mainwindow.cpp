#include "mainwindow.h"
#include "genSalt.h"

#include <QMessageBox>
#include <QtSql>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Создаем центральный виджет и устанавливаем его
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Создаем виджеты формы
    QLabel *loginLabel = new QLabel("Username:", this);
    loginLineEdit = new QLineEdit(this);

    QLabel *passwordLabel = new QLabel("Password:", this);
    passwordLineEdit = new QLineEdit(this);
    passwordLineEdit->setEchoMode(QLineEdit::Password);

    loginButton = new QPushButton("Login", this);
    registerButton = new QPushButton("Register", this);
    statusLabel = new QLabel(this);

    // Создаем компоновку
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(loginLabel);
    layout->addWidget(loginLineEdit);
    layout->addWidget(passwordLabel);
    layout->addWidget(passwordLineEdit);
    layout->addWidget(loginButton);
    layout->addWidget(registerButton);
    layout->addWidget(statusLabel);

    // Устанавливаем компоновку для центрального виджета
    centralWidget->setLayout(layout);

    // Подключаем сигналы к слотам
    connect(loginButton, &QPushButton::clicked, this, &MainWindow::onLoginClicked);
    connect(registerButton, &QPushButton::clicked, this, &MainWindow::onRegisterClicked);
}

MainWindow::~MainWindow()
{
}

void MainWindow::onLoginClicked() {
    QString login = loginLineEdit->text();
    QString password = passwordLineEdit->text();

    if (login.isEmpty() || password.isEmpty()) {
        statusLabel->setText("Please fill in all fields.");
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    query.prepare("SELECT password, salt FROM users WHERE login = :login");
    query.bindValue(":login", login);

    if (query.exec() && query.next()) {
        QString storedHashedPassword = query.value(0).toString();
        QString salt = query.value(1).toString();

        // Применяем тот же процесс к введенному паролю: конкатенация с солью и хэширование
        QString saltedPassword = password + salt;
        QString hashedPassword = hashFunction(saltedPassword);

        // Сравниваем хеши
        if (storedHashedPassword == hashedPassword) {
            statusLabel->setText("Login successful!");
            // Перенаправляем на главное окно приложения
        } else {
            statusLabel->setText("Incorrect password.");
        }
    } else {
        statusLabel->setText("Username not found.");
    }
}



void MainWindow::onRegisterClicked() {
    QString login = loginLineEdit->text();
    QString password = passwordLineEdit->text();

    if (login.isEmpty() || password.isEmpty()) {
        statusLabel->setText("Please fill in all fields.");
        return;
    }

    // Генерация случайной соли
    QString salt = generateRandomSalt();
    // Конкатенация пароля и соли
    QString saltedPassword = password + salt;
    // Хэширование пароля с солью
    QString hashedPassword = hashFunction(saltedPassword);

    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    query.prepare("INSERT INTO users (login, password, salt) VALUES (:login, :hashedPassword, :salt)");
    query.bindValue(":login", login);
    query.bindValue(":hashedPassword", hashedPassword);
    query.bindValue(":salt", salt);

    if (query.exec()) {
        statusLabel->setText("Registration successful!");
    } else {
        statusLabel->setText("Registration failed: " + query.lastError().text());
    }
}


