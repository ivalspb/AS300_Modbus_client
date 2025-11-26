#pragma once
#include <QWidget>

class QLineEdit;
class QSpinBox;
class QPushButton;
class QLabel;

class ConnectionWidget : public QWidget {
    Q_OBJECT
public:
    explicit ConnectionWidget(QWidget* parent = nullptr);

    QString address() const;
    quint16 serverPort() const;
    quint16 clientPort() const;

    void setConnected(bool connected);
    void setStatus(const QString& status);

signals:
    void connectRequested(const QString& address, quint16 serverPort, quint16 clientPort);
    void disconnectRequested();

private:
    void setupUI();

    QLineEdit* m_addressEdit;
    QSpinBox* m_serverPortSpinBox;
    QSpinBox* m_clientPortSpinBox;
    QPushButton* m_connectButton;
    QPushButton* m_disconnectButton;
    QLabel* m_statusLabel;
};

