#include "DatabaseViewController.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QDateEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>

DatabaseViewController::DatabaseViewController(DatabaseAsyncManager* dbManager,
                                               DatabaseExportService* exportService,
                                               QObject* parent)
    : QObject(parent)
    , m_dbManager(dbManager)
    , m_exportService(exportService)
    , m_mainWidget(nullptr)
    , m_sessionsTable(nullptr)
    , m_fromDateEdit(nullptr)
    , m_toDateEdit(nullptr)
    , m_testTypeCombo(nullptr)
    , m_loadSessionsButton(nullptr)
    , m_loadDataButton(nullptr)
    , m_exportCsvButton(nullptr)
    , m_exportImageButton(nullptr)
    , m_statusLabel(nullptr)
    , m_currentSessionId(-1)
{
    setupUI();
    setupConnections();
}

QWidget* DatabaseViewController::getWidget() {
    return m_mainWidget;
}

void DatabaseViewController::setupUI() {
    m_mainWidget = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(m_mainWidget);

    // Заголовок
    QLabel* titleLabel = new QLabel("История тестов");
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    // Панель поиска
    QGroupBox* searchGroup = new QGroupBox("Поиск тестовых сессий");
    QFormLayout* searchLayout = new QFormLayout(searchGroup);

    m_fromDateEdit = new QDateEdit(QDate::currentDate().addDays(-30));
    m_fromDateEdit->setCalendarPopup(true);
    m_toDateEdit = new QDateEdit(QDate::currentDate());
    m_toDateEdit->setCalendarPopup(true);

    m_testTypeCombo = new QComboBox();
    m_testTypeCombo->addItem("Все типы", "");
    m_testTypeCombo->addItem("Расконсервация/Консервация", "Расконсервация/Консервация");
    m_testTypeCombo->addItem("Холодная прокрутка турбостартера", "Холодная прокрутка турбостартера");
    m_testTypeCombo->addItem("Регулировка мощности, замер параметров", "Регулировка мощности, замер параметров");
    m_testTypeCombo->addItem("Холодная прокрутка основного двигателя", "Холодная прокрутка основного двигателя");
    m_testTypeCombo->addItem("Имитация запуска основного двигателя", "Имитация запуска основного двигателя");

    m_loadSessionsButton = new QPushButton("Загрузить сессии");

    searchLayout->addRow("Период с:", m_fromDateEdit);
    searchLayout->addRow("по:", m_toDateEdit);
    searchLayout->addRow("Тип теста:", m_testTypeCombo);
    searchLayout->addRow(m_loadSessionsButton);

    mainLayout->addWidget(searchGroup);

    // Таблица сессий
    QGroupBox* sessionsGroup = new QGroupBox("Тестовые сессии");
    QVBoxLayout* sessionsLayout = new QVBoxLayout(sessionsGroup);

    m_sessionsTable = new QTableWidget();
    m_sessionsTable->setColumnCount(5);
    m_sessionsTable->setHorizontalHeaderLabels({
        "ID", "Тип теста", "Начало", "Окончание", "Длительность"
    });
    m_sessionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_sessionsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_sessionsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Настройка ширины колонок
    m_sessionsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_sessionsTable->setColumnWidth(0, 50);
    m_sessionsTable->setColumnWidth(2, 150);
    m_sessionsTable->setColumnWidth(3, 150);
    m_sessionsTable->setColumnWidth(4, 100);

    sessionsLayout->addWidget(m_sessionsTable);

    // Кнопки управления выбранной сессией
    QHBoxLayout* sessionButtonsLayout = new QHBoxLayout();
    m_loadDataButton = new QPushButton("Загрузить данные");
    m_exportCsvButton = new QPushButton("Экспорт в CSV");
    m_exportImageButton = new QPushButton("Экспорт в изображение");

    m_loadDataButton->setEnabled(false);
    m_exportCsvButton->setEnabled(false);
    m_exportImageButton->setEnabled(false);

    sessionButtonsLayout->addWidget(m_loadDataButton);
    sessionButtonsLayout->addWidget(m_exportCsvButton);
    sessionButtonsLayout->addWidget(m_exportImageButton);
    sessionButtonsLayout->addStretch();

    sessionsLayout->addLayout(sessionButtonsLayout);
    mainLayout->addWidget(sessionsGroup);

    // Статусная строка
    m_statusLabel = new QLabel("Готов к работе");
    m_statusLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    mainLayout->addWidget(m_statusLabel);

    mainLayout->addStretch();
}

void DatabaseViewController::setupConnections() {
    connect(m_loadSessionsButton, &QPushButton::clicked,
            this, &DatabaseViewController::onLoadSessionsClicked);
    connect(m_loadDataButton, &QPushButton::clicked,
            this, &DatabaseViewController::onLoadDataClicked);
    connect(m_exportCsvButton, &QPushButton::clicked,
            this, &DatabaseViewController::onExportCsvClicked);
    connect(m_exportImageButton, &QPushButton::clicked,
            this, &DatabaseViewController::onExportImageClicked);
    connect(m_sessionsTable, &QTableWidget::itemSelectionChanged,
            this, &DatabaseViewController::onSessionSelectionChanged);

    if (m_exportService) {
        connect(m_exportService, &DatabaseExportService::exportCompleted,
                this, &DatabaseViewController::onExportCompleted);
        connect(m_exportService, &DatabaseExportService::exportFailed,
                this, &DatabaseViewController::onExportFailed);
    }
}

void DatabaseViewController::onLoadSessionsClicked() {
    if (!m_dbManager) {
        showMessage("Ошибка: менеджер БД не инициализирован");
        return;
    }

    QDateTime from = QDateTime(m_fromDateEdit->date(), QTime(0, 0, 0));
    QDateTime to = QDateTime(m_toDateEdit->date(), QTime(23, 59, 59));
    QString testType = m_testTypeCombo->currentData().toString();

    showMessage(QString("Загрузка сессий с %1 по %2").arg(
        from.toString("dd.MM.yyyy"), to.toString("dd.MM.yyyy")));

    emit loadSessionsRequested(from, to, testType);
}

void DatabaseViewController::onLoadDataClicked() {
    if (m_currentSessionId <= 0) {
        QMessageBox::warning(nullptr, "Ошибка", "Не выбрана тестовая сессия");
        return;
    }

    showMessage(QString("Загрузка данных для сессии ID: %1").arg(m_currentSessionId));
    emit loadSessionDataRequested(m_currentSessionId);
}

void DatabaseViewController::onExportCsvClicked() {
    if (m_currentSessionId <= 0) {
        QMessageBox::warning(nullptr, "Ошибка", "Не выбрана тестовая сессия");
        return;
    }

    QString filename = QFileDialog::getSaveFileName(
        nullptr, "Экспорт в CSV",
        QString("session_%1.csv").arg(m_currentSessionId),
        "CSV Files (*.csv)");

    if (!filename.isEmpty()) {
        showMessage(QString("Экспорт сессии %1 в CSV: %2").arg(m_currentSessionId).arg(filename));
        emit exportToCsvRequested(m_currentSessionId, filename);
    }
}

void DatabaseViewController::onExportImageClicked() {
    if (m_currentSessionId <= 0) {
        QMessageBox::warning(nullptr, "Ошибка", "Не выбрана тестовая сессия");
        return;
    }

    QString filename = QFileDialog::getSaveFileName(
        nullptr, "Экспорт в изображение",
        QString("session_%1.png").arg(m_currentSessionId),
        "PNG Files (*.png);;JPEG Files (*.jpg)");

    if (!filename.isEmpty()) {
        showMessage(QString("Экспорт сессии %1 в изображение: %2").arg(m_currentSessionId).arg(filename));
        emit exportToImageRequested(m_currentSessionId, filename);
    }
}

void DatabaseViewController::onSessionSelectionChanged() {
    QList<QTableWidgetItem*> selectedItems = m_sessionsTable->selectedItems();
    bool hasSelection = !selectedItems.isEmpty();

    m_loadDataButton->setEnabled(hasSelection);
    m_exportCsvButton->setEnabled(hasSelection);
    m_exportImageButton->setEnabled(hasSelection);

    if (hasSelection) {
        int row = selectedItems.first()->row();
        QTableWidgetItem* idItem = m_sessionsTable->item(row, 0);
        m_currentSessionId = idItem->data(Qt::UserRole).toInt();
        showMessage(QString("Выбрана сессия ID: %1").arg(m_currentSessionId));
    } else {
        m_currentSessionId = -1;
    }
}

void DatabaseViewController::onExportCompleted(const QString& filename) {
    showMessage(QString("Экспорт успешно завершен: %1").arg(filename));
    QMessageBox::information(nullptr, "Экспорт",
                           QString("Данные успешно экспортированы в:\n%1").arg(filename));
}

void DatabaseViewController::onExportFailed(const QString& error) {
    showMessage(QString("Ошибка экспорта: %1").arg(error));
    QMessageBox::critical(nullptr, "Ошибка экспорта", error);
}

void DatabaseViewController::showSessions(const QVector<TestSession>& sessions) {
    updateSessionsTable(sessions);
    showMessage(QString("Загружено %1 тестовых сессий").arg(sessions.size()));
}

void DatabaseViewController::showSessionData(const QVector<DataPointRecord>& points) {
    if (!points.isEmpty()) {
        showMessage(QString("Загружено %1 точек данных").arg(points.size()));

        // Анализ данных
        QMap<QString, int> pointsByParameter;
        QDateTime minTime, maxTime;

        for (const auto& point : points) {
            pointsByParameter[point.parameter]++;

            if (!minTime.isValid() || point.timestamp < minTime) {
                minTime = point.timestamp;
            }
            if (!maxTime.isValid() || point.timestamp > maxTime) {
                maxTime = point.timestamp;
            }
        }

        QStringList stats;
        for (auto it = pointsByParameter.begin(); it != pointsByParameter.end(); ++it) {
            stats.append(QString("%1: %2 точек").arg(it.key()).arg(it.value()));
        }

        showMessage(QString("Период данных: %1 - %2 | %3")
                   .arg(minTime.toString("dd.MM.yyyy HH:mm:ss"))
                   .arg(maxTime.toString("dd.MM.yyyy HH:mm:ss"))
                   .arg(stats.join(", ")));
    } else {
        QMessageBox::information(nullptr, "Информация", "Для выбранной сессии нет данных");
    }
}

void DatabaseViewController::showExportProgress(int progress) {
    showMessage(QString("Прогресс экспорта: %1%").arg(progress));
}

void DatabaseViewController::showMessage(const QString& message) {
    if (m_statusLabel) {
        m_statusLabel->setText(message);
    }
}

int DatabaseViewController::getSelectedSessionId() const {
    return m_currentSessionId;
}

QDateTime DatabaseViewController::getSearchFromDate() const {
    return m_fromDateEdit ? QDateTime(m_fromDateEdit->date(), QTime(0, 0, 0)) : QDateTime();
}

QDateTime DatabaseViewController::getSearchToDate() const {
    return m_toDateEdit ? QDateTime(m_toDateEdit->date(), QTime(23, 59, 59)) : QDateTime();
}

QString DatabaseViewController::getSearchTestType() const {
    return m_testTypeCombo ? m_testTypeCombo->currentData().toString() : "";
}

void DatabaseViewController::updateSessionsTable(const QVector<TestSession>& sessions) {
    if (!m_sessionsTable) return;

    m_sessionsTable->setRowCount(sessions.size());

    for (int i = 0; i < sessions.size(); ++i) {
        const TestSession& session = sessions[i];

        // ID
        QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(session.id));
        idItem->setData(Qt::UserRole, session.id);
        m_sessionsTable->setItem(i, 0, idItem);

        // Тип теста
        m_sessionsTable->setItem(i, 1, new QTableWidgetItem(session.testType));

        // Время начала
        m_sessionsTable->setItem(i, 2, new QTableWidgetItem(
            session.startTime.toString("dd.MM.yyyy HH:mm:ss")));

        // Время окончания
        QString endTimeStr = session.endTime.isValid() ?
            session.endTime.toString("dd.MM.yyyy HH:mm:ss") : "Не завершена";
        m_sessionsTable->setItem(i, 3, new QTableWidgetItem(endTimeStr));

        // Длительность
        QString durationStr = formatDuration(session.startTime, session.endTime);
        m_sessionsTable->setItem(i, 4, new QTableWidgetItem(durationStr));
    }
}

QString DatabaseViewController::formatDuration(const QDateTime& start, const QDateTime& end) const {
    if (!end.isValid() || !start.isValid()) {
        return "N/A";
    }

    qint64 duration = start.secsTo(end);
    if (duration <= 0) {
        return "N/A";
    }

    int hours = duration / 3600;
    int minutes = (duration % 3600) / 60;
    int seconds = duration % 60;

    return QString("%1:%2:%3")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'));
}
