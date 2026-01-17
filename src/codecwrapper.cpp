#include "codecwrapper.h"
#include <QStandardPaths>

QString CodecWrapper::findExecutable(const QString& name) const
{
    return QStandardPaths::findExecutable(name);
}
