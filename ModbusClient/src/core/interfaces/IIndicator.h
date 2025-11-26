#ifndef IINDICATOR_H
#define IINDICATOR_H

#include <QObject>

class IValueIndicator {
public:
    virtual ~IValueIndicator() = default;
    virtual void setValue(double value) = 0;
    virtual void setRange(double min, double max) = 0;
//    virtual void setVisible(bool visible) = 0;
};

class IDualIndicator : public IValueIndicator {
public:
    virtual ~IDualIndicator() = default;
    virtual void setSecondaryValue(double value) = 0;
    virtual void setSecondaryRange(double min, double max) = 0;
};

#endif // IINDICATOR_H
