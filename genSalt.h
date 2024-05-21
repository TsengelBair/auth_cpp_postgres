#ifndef SALT_H
#define SALT_H

#include <QtCore/QDateTime>
#include <QtCore/QCryptographicHash>
#include <QString>

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

#endif // SALT_H
