### 1. Для использования postgresql в приложениях qt, необходимо установить модули
Ссылка на гайд (https://www.youtube.com/watch?v=fBgJ9Azm_S0&list=PLt7W7qLhROYC4S3ANrOwdlWAJIgNSSz-5)

Если проблема не решается, стоит попробовать скопировать большую часть dll файлов из postgres и добавить вручную в qt mingw/bin
### 2. Подключение к бд при запуске (параметры для бд вынесены в файл gitignore)
В main.cpp создаем подключение к бд, указывая необходимые параметры 
```cpp
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
```
### 3. Инициализация в заголовочном необходимых виджетов

```cpp
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLoginClicked();
    void onRegisterClicked();

private:
    QLineEdit *loginLineEdit;
    QLineEdit *passwordLineEdit;
    QPushButton *loginButton;
    QPushButton *registerButton;
    QLabel *statusLabel;
};

#endif // MAINWINDOW_H

```
### 4. Создание виджетов и подключение сигналов к слотам в mainwindow.cpp

```cpp
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
```
### 5. Для безопасного хранения паролей будем к каждому паролю генерировать соль и использовать хэширование, обе функции вынесем в отдельный файл

```cpp
// Функция для генерации случайной соли
inline QString generateRandomSalt() {
    const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
    const int saltLength = 16; // Длина соли
    QString salt;
    srand(QDateTime::currentMSecsSinceEpoch() / 1000); // Инициализация генератора случайных чисел
    for (int i = 0; i < saltLength; ++i) {
        int index = rand() % possibleCharacters.length();
        QChar nextChar = possibleCharacters.at(index);
        salt.append(nextChar);
    }
    return salt;
}

// Функция для хэширования строки с использованием SHA-256
inline QString hashFunction(const QString& input) {
    // Выполнение хэширования
    QByteArray hash = QCryptographicHash::hash(input.toUtf8(), QCryptographicHash::Sha256);
    return QString(hash.toHex());
}
```
### 6. Регистрация и авторизация 
Причем в бд будем помимо пароля и логина хранить "соль" для декодирования при авторизации

```cpp
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
```
