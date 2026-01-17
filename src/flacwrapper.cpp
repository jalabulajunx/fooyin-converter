#include "flacwrapper.h"
#include <QDebug>
#include <QRegularExpression>
#include <QFile>

FlacWrapper::FlacWrapper(QObject* parent)
    : CodecWrapper(parent)
{
    m_execPath = findExecutable("flac");

    if (m_execPath.isEmpty()) {
        qWarning() << "FLAC encoder not found in PATH";
    }
}

bool FlacWrapper::isAvailable() const
{
    return !m_execPath.isEmpty();
}

QString FlacWrapper::version() const
{
    if (!isAvailable()) {
        return "Not found";
    }

    QProcess process;
    process.start(m_execPath, {"--version"});
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QRegularExpression versionRegex("flac ([0-9.]+)");
    QRegularExpressionMatch match = versionRegex.match(output);

    if (match.hasMatch()) {
        return match.captured(1);
    }

    return "Unknown";
}

QStringList FlacWrapper::buildArguments(
    const QString& inputPath,
    const QString& outputPath,
    const ConversionOptions& options)
{
    QStringList args;

    // Compression level (0-8, default 8)
    args << QString("-%1").arg(options.compressionLevel);

    // Force overwrite
    args << "--force";

    // Don't use --silent as it suppresses progress output
    // FLAC shows progress by default

    // Verify encoding
    args << "--verify";

    // Preserve metadata tags
    args << "--keep-foreign-metadata";

    // Output file
    args << "-o" << outputPath;

    // Input file (must be last)
    args << inputPath;

    return args;
}

bool FlacWrapper::convert(
    const QString& inputPath,
    const QString& outputPath,
    const ConversionOptions& options)
{
    if (!isAvailable()) {
        qWarning() << "FLAC encoder not available";
        return false;
    }

    QStringList args = buildArguments(inputPath, outputPath, options);

    QProcess process;
    process.start(m_execPath, args);

    if (!process.waitForStarted()) {
        qWarning() << "Failed to start FLAC encoder";
        return false;
    }

    if (!process.waitForFinished(-1)) {
        qWarning() << "FLAC encoding process error";
        return false;
    }

    if (process.exitCode() != 0) {
        QString error = process.readAllStandardError();
        qWarning() << "FLAC encoding failed:" << error;
        return false;
    }

    return true;
}

void FlacWrapper::convertAsync(
    const QString& inputPath,
    const QString& outputPath,
    const ConversionOptions& options)
{
    if (m_process) {
        qWarning() << "Conversion already in progress";
        return;
    }

    m_outputPath = outputPath;
    m_process = new QProcess(this);

    // Connect progress monitoring (FLAC outputs progress to stderr)
    connect(m_process, &QProcess::readyReadStandardError, this, [this]() {
        QString output = m_process->readAllStandardError();
        parseProgress(output);
    });

    // Connect completion
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus status) {
        bool success = (exitCode == 0 && status == QProcess::NormalExit);
        QString error;

        if (!success) {
            error = m_process->readAllStandardError();
        }

        m_process->deleteLater();
        m_process = nullptr;
        m_outputPath.clear();

        emit conversionFinished(success, error);
    });

    // Start conversion
    QStringList args = buildArguments(inputPath, outputPath, options);
    m_process->start(m_execPath, args);
}

void FlacWrapper::cancel()
{
    if (m_process && m_process->state() != QProcess::NotRunning) {
        // Disconnect signals to avoid spurious callbacks
        m_process->disconnect();

        // Try graceful termination first
        m_process->terminate();

        // Wait up to 1 second for graceful shutdown
        if (!m_process->waitForFinished(1000)) {
            // Force kill if termination didn't work
            m_process->kill();
            m_process->waitForFinished(100);
        }

        m_process->deleteLater();
        m_process = nullptr;
    }

    // Delete partially converted file if it exists
    if (!m_outputPath.isEmpty()) {
        QFile::remove(m_outputPath);
        m_outputPath.clear();
    }
}

void FlacWrapper::parseProgress(const QString& output)
{
    // FLAC outputs progress like: "filename: 45% complete"
    static QRegularExpression progressRegex("(\\d+)% complete");

    QRegularExpressionMatch match = progressRegex.match(output);
    if (match.hasMatch()) {
        int percent = match.captured(1).toInt();
        emit progressChanged(qBound(0, percent, 100));
    }
}
