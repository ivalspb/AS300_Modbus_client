#pragma once
#include <QObject>
#include <QMap>

class IModbusClient;
class QComboBox;
class QLineEdit;
class QPushButton;

class ParameterController : public QObject {
    Q_OBJECT
public:
    explicit ParameterController(IModbusClient* client, QObject* parent = nullptr);

    void setParameterComboBox(QComboBox* comboBox);
    void setAddressEdit(QLineEdit* edit);
    void setAddButton(QPushButton* button);

    QMap<QString, quint16> parameterMap() const { return m_parameterMap; }

signals:
    void parameterAdded(const QString& name, quint16 address);
    void parameterSelected(const QString& name);
    void logMessage(const QString& message);

private slots:
    void onAddParameter();
    void onParameterSelected(int index);

private:
    IModbusClient* m_client;
    QComboBox* m_parameterCombo;
    QLineEdit* m_addressEdit;
    QPushButton* m_addButton;
    QMap<QString, quint16> m_parameterMap;
};

