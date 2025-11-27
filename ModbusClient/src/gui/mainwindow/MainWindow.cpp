#include "MainWindow.h"
#include "../connection/ConnectionViewController.h"
#include "../mode_selection/ModeSelectionViewController.h"
#include "../monitoring/MonitoringViewController.h"
#include "../database/DatabaseViewController.h"
#include "../widgets/ChartWidget.h"
#include "data/interfaces/IDataRepository.h"
#include <QSplitter>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_mainSplitter(nullptr)
    , m_tabWidget(nullptr)
    , m_connectionVC(nullptr)
    , m_modeSelectionVC(nullptr)
    , m_monitoringVC(nullptr)
    , m_databaseVC(nullptr)
    , m_dataRepository(nullptr)
{
    qDebug() << "MainWindow: Created";
}

void MainWindow::setupUI(ConnectionViewController* connectionVC,
                         ModeSelectionViewController* modeSelectionVC,
                         MonitoringViewController* monitoringVC,
                         DatabaseViewController* databaseVC) {
    m_connectionVC = connectionVC;
    m_modeSelectionVC = modeSelectionVC;
    m_monitoringVC = monitoringVC;
    m_databaseVC = databaseVC;

    qDebug() << "MainWindow: Setting up UI";

    // Главный splitter - левая панель (вкладки) и правая панель (график)
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);

    // Левая панель - вкладки
    QWidget* leftPanel = new QWidget();
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);

    m_tabWidget = new QTabWidget();

    // Добавляем вкладки
    if (m_connectionVC && m_modeSelectionVC) {
        QWidget* controlTab = new QWidget();
        QVBoxLayout* controlLayout = new QVBoxLayout(controlTab);

        // Вкладка управления: подключение + выбор режима
        controlLayout->addWidget(m_connectionVC->getWidget());
        controlLayout->addWidget(m_modeSelectionVC->getWidget());
        controlLayout->addStretch();

        m_tabWidget->addTab(controlTab, "Управление");
        qDebug() << "MainWindow: Added Control tab";
    }

    if (m_monitoringVC) {
        m_tabWidget->addTab(m_monitoringVC->getWidget(), "Мониторинг");
        qDebug() << "MainWindow: Added Monitoring tab";
    }

    if (m_databaseVC) {
        m_tabWidget->addTab(m_databaseVC->getWidget(), "История");
        qDebug() << "MainWindow: Added Database tab";
    }

    leftLayout->addWidget(m_tabWidget);

    // Добавляем панели в splitter
    m_mainSplitter->addWidget(leftPanel);

    setCentralWidget(m_mainSplitter);
    setWindowTitle("Delta AS332T Modbus Client");
    showMaximized();

    qDebug() << "MainWindow: UI setup completed";
}

void MainWindow::setupConnections() {
    // Здесь можно добавить связи между компонентами, если нужно
    qDebug() << "MainWindow: Setting up connections";
}

void MainWindow::closeEvent(QCloseEvent* event) {
    qDebug() << "MainWindow: Close event triggered";

    // Показываем диалог подтверждения
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Выход",
                                  "Вы уверены, что хотите выйти?\nВсе несохранённые данные будут сохранены.",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No) {
        event->ignore();
        qDebug() << "MainWindow: Close cancelled by user";
        return;
    }

    // Сохраняем данные перед закрытием
    if (m_dataRepository) {
        qDebug() << "MainWindow: Finalizing data session";
        m_dataRepository->finalizeSession();
    }

    // Останавливаем мониторинг
    if (m_monitoringVC) {
        qDebug() << "MainWindow: Stopping monitoring";
        m_monitoringVC->stopMonitoring();
    }

    qDebug() << "MainWindow: Application closing";
    event->accept();
    QMainWindow::closeEvent(event);
}
