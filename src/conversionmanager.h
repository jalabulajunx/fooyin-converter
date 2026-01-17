#pragma once

#include "codecwrapper.h"
#include <QObject>
#include <QMap>
#include <QString>

class FlacWrapper;
class LameWrapper;
class OpusWrapper;
class OggWrapper;

class ConversionManager : public QObject
{
    Q_OBJECT

public:
    explicit ConversionManager(QObject* parent = nullptr);
    ~ConversionManager() override;

    // Check which codecs are available
    bool isCodecAvailable(const QString& format) const;
    QStringList availableCodecs() const;
    QString codecVersion(const QString& format) const;

    // Conversion operations
    bool convert(
        const QString& inputPath,
        const QString& outputPath,
        const ConversionOptions& options
    );

    void convertAsync(
        const QString& inputPath,
        const QString& outputPath,
        const ConversionOptions& options
    );

    void cancel();

    // Status
    bool isConverting() const { return m_converting; }

signals:
    void progressChanged(int percent);
    void conversionFinished(bool success, const QString& error);
    void conversionStarted();

private:
    CodecWrapper* getCodecWrapper(const QString& format);

    FlacWrapper* m_flacWrapper;
    LameWrapper* m_lameWrapper;
    OpusWrapper* m_opusWrapper;
    OggWrapper* m_oggWrapper;

    QMap<QString, CodecWrapper*> m_codecMap;
    CodecWrapper* m_currentCodec{nullptr};
    bool m_converting{false};
};
