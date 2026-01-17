#pragma once

#include "codecwrapper.h"

class LameWrapper : public CodecWrapper
{
    Q_OBJECT

public:
    explicit LameWrapper(QObject* parent = nullptr);

    bool isAvailable() const override;
    QString version() const override;
    QString executableName() const override { return "lame"; }

    bool convert(
        const QString& inputPath,
        const QString& outputPath,
        const ConversionOptions& options
    ) override;

    void convertAsync(
        const QString& inputPath,
        const QString& outputPath,
        const ConversionOptions& options
    ) override;

    void cancel() override;

private:
    QStringList buildArguments(
        const QString& inputPath,
        const QString& outputPath,
        const ConversionOptions& options
    );

    void parseProgress(const QString& output);

    QString m_execPath;
};
