# Modbus Client для Delta AS332T

Проект представляет собой графическое приложение для взаимодействия с контроллерами Delta AS332T по протоколу Modbus TCP.

## Структура проекта

### Основные модули

#### 1. Core (Ядро системы)
- **Interfaces** - абстрактные интерфейсы:
  - `IModbusClient` - клиент Modbus
  - `IRequestQueue` - очередь запросов
  - `IAddressMapper` - маппинг адресов
  - `IIndicator` - интерфейсы индикаторов

- **Modbus Implementation** - реализация Modbus:
  - `DeltaModbusClient` - специализированный клиент для Delta
  - `ModbusRequestQueue` - управление очередью запросов
  - `ModbusRequestHandler` - обработчик запросов

- **Mapping** - работа с адресами:
  - `DeltaAddressMapper` - маппинг для Delta AS332T
  - `DeltaController` - перечисления входов/выходов

- **Connection** - управление соединением:
  - `ConnectionManager` - менеджер подключения

#### 2. Data Layer (Работа с данными)
- **Data Repository** - хранение и управление данными
- **Database** - работа с БД SQLite:
  - `SqliteDatabaseRepository` - репозиторий БД
  - `DatabaseAsyncManager` - асинхронное управление
  - `DatabaseExportService` - экспорт данных

#### 3. Monitoring (Мониторинг)
- `DataMonitor` - главный монитор данных
- `DiscreteInputMonitor` - мониторинг дискретных входов
- `AnalogValueMonitor` - мониторинг аналоговых значений

#### 4. Control (Управление)
- `ModeController` - управление режимами тестирования
- `ControlStateMachine` - автомат управления процессом
- `ControlUIController` - контроллер UI управления

#### 5. GUI (Пользовательский интерфейс)
- **Widgets** - специализированные виджеты:
  - `ConnectionWidget` - подключение к устройству
  - `MonitorWidget` - панель мониторинга
  - `ChartWidget` - графики с мульти-осями
  - `DualIndicatorWidget` - комбинированные индикаторы
  - `SpeedometerWidget` - виджет спидометра

- **Factories** - фабрики виджетов

#### 6. Export (Экспорт данных)
- `PngExportStrategy` - экспорт в PNG
- Интерфейсы для расширения функциональности экспорта

## Основные возможности

- **Подключение** к устройствам Delta AS332T по Modbus TCP
- **Мониторинг** в реальном времени:
  - Дискретные входы (S1-S12)
  - Командные выходы (K1-K6) 
  - Аналоговые значения (обороты АД, ТК, СТ)
- **Управление** тестовыми режимами:
  - Расконсервация/Консервация
  - Холодная прокрутка турбостартера
  - Регулировка мощности
  - Холодная прокрутка основного двигателя
  - Имитация запуска
- **Визуализация** данных:
  - Комбинированные индикаторы (спидометр + LCD)
  - Мульти-осевые графики в реальном времени
- **История данных**:
  - Сохранение в SQLite БД
  - Загрузка и просмотр исторических данных
  - Экспорт в CSV и изображения

## Технологии

- C++17, Qt5, QModbus
- Многопоточность (QMutex, QReadWriteLock)
- SQLite для хранения данных
- Паттерны: MVC, State Machine, Factory, Strategy

## Сборка

Проект использует CMake для сборки. Основные зависимости:
- Qt5 (Core, Widgets, Charts, Network, SerialBus, Sql)
- SQLite3

Приложение состоит из статической библиотеки `ModbusCore` и исполняемого файла `ModbusClient`.
