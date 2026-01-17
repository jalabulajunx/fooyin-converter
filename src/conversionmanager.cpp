#include "conversionmanager.h"
#include "flacwrapper.h"
#include "lamewrapper.h"
#include "opuswrapper.h"
#include "oggwrapper.h"
#include <QDebug>

ConversionManager::ConversionManager(QObject* parent)
    : QObject(parent)
{
    // Initialize codec wrappers
    m_flacWrapper = new FlacWrapper(this);
    m_lameWrapper = new LameWrapper(this);
    m_opusWrapper = new OpusWrapper(this);
    m_oggWrapper = new OggWrapper(this);

    // Build codec map
    m_codecMap["flac"] = m_flacWrapper;
    m_codecMap["mp3"] = m_lameWrapper;
    m_codecMap["opus"] = m_opusWrapper;
    m_codecMap["ogg"] = m_oggWrapper;

    // Log available codecs
    qInfo() << "Audio Converter - Available codecs:";
    for (auto it = m_codecMap.constBegin(); it != m_codecMap.constEnd(); ++it) {
        if (it.value()->isAvailable()) {
            qInfo() << "  " << it.key() << "-" << it.value()->version();
        } else {
            qInfo() << "  " << it.key() << "- Not available";
        }
    }
}

ConversionManager::~ConversionManager()
{
    cancel();
}

bool ConversionManager::isCodecAvailable(const QString& format) const
{
    CodecWrapper* codec = m_codecMap.value(format.toLower(), nullptr);
    return codec && codec->isAvailable();
}

QStringList ConversionManager::availableCodecs() const
{
    QStringList available;
    for (auto it = m_codecMap.constBegin(); it != m_codecMap.constEnd(); ++it) {
        if (it.value()->isAvailable()) {
            available << it.key();
        }
    }
    return available;
}

QString ConversionManager::codecVersion(const QString& format) const
{
    CodecWrapper* codec = m_codecMap.value(format.toLower(), nullptr);
    if (codec) {
        return codec->version();
    }
    return "Unknown";
}

CodecWrapper* ConversionManager::getCodecWrapper(const QString& format)
{
    return m_codecMap.value(format.toLower(), nullptr);
}

bool ConversionManager::convert(
    const QString& inputPath,
    const QString& outputPath,
    const ConversionOptions& options)
{
    CodecWrapper* codec = getCodecWrapper(options.format);
    if (!codec) {
        qWarning() << "No codec found for format:" << options.format;
        return false;
    }

    if (!codec->isAvailable()) {
        qWarning() << "Codec not available:" << options.format;
        return false;
    }

    return codec->convert(inputPath, outputPath, options);
}

void ConversionManager::convertAsync(
    const QString& inputPath,
    const QString& outputPath,
    const ConversionOptions& options)
{
    if (m_converting) {
        qWarning() << "Conversion already in progress";
        emit conversionFinished(false, "Another conversion is already in progress");
        return;
    }

    CodecWrapper* codec = getCodecWrapper(options.format);
    if (!codec) {
        qWarning() << "No codec found for format:" << options.format;
        emit conversionFinished(false, "Unsupported format: " + options.format);
        return;
    }

    if (!codec->isAvailable()) {
        qWarning() << "Codec not available:" << options.format;
        emit conversionFinished(false, "Codec not installed: " + codec->executableName());
        return;
    }

    m_currentCodec = codec;
    m_converting = true;

    // Connect codec signals
    connect(codec, &CodecWrapper::progressChanged, this, [this](int percent) {
        emit progressChanged(percent);
    });

    connect(codec, &CodecWrapper::conversionFinished, this, [this](bool success, const QString& error) {
        m_converting = false;

        // Disconnect codec signals
        if (m_currentCodec) {
            disconnect(m_currentCodec, nullptr, this, nullptr);
            m_currentCodec = nullptr;
        }

        emit conversionFinished(success, error);
    });

    emit conversionStarted();
    codec->convertAsync(inputPath, outputPath, options);
}

void ConversionManager::cancel()
{
    if (m_converting && m_currentCodec) {
        m_currentCodec->cancel();
        m_converting = false;
        disconnect(m_currentCodec, nullptr, this, nullptr);
        m_currentCodec = nullptr;
    }
}
