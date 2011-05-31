#ifndef POWERMANAGEMENT_JOB_H
#define POWERMANAGEMENT_JOB_H

#include "powermanagementengine.h"

#include <Plasma/ServiceJob>

typedef QMap<QString, QString> StringStringMap;

class PowermanagementJob : public Plasma::ServiceJob
{
    Q_OBJECT
    
public:
    PowermanagementJob (PowermanagementEngine* engine,
                        const QString& destination,
                        const QString& operation,
                        QMap<QString,QVariant>& parameters,
                        QObject* parent = 0)
    : ServiceJob (destination, operation, parameters, parent),
      m_engine(engine)
    {
    }
    
    void start();
    
private:
    PowermanagementEngine* m_engine;
};

Q_DECLARE_METATYPE (StringStringMap);

#endif // POWERMANAGEMENT_JOB_H
