#ifndef QHOMEBREWUPDATERBACKEND_H
#define QHOMEBREWUPDATERBACKEND_H

#include <QtCore/QLoggingCategory>

#include <QtAutoUpdaterCore/ProcessBackend>

class QHomebrewUpdaterBackend : public QtAutoUpdater::ProcessBackend
{
	Q_OBJECT

public:
	explicit QHomebrewUpdaterBackend(QString &&key, QObject *parent = nullptr);

	Features features() const override;
	void checkForUpdates() override;
	QtAutoUpdater::UpdateInstaller *createInstaller() override;

protected:
	bool initialize() override;
	void onToolDone(int id, int exitCode, QIODevice *processDevice) override;
	std::optional<InstallProcessInfo> installerInfo(const QList<QtAutoUpdater::UpdateInfo> &infos, bool track) override;

private:
	enum RunId {
		Update = 0,
		Outdated = 1
	};
	QStringList _packages;

	QString brewPath() const;
	QString cakebrewPath() const;

	void onUpdated(int exitCode);
	void onOutdated(int exitCode, QIODevice *processDevice);
};

Q_DECLARE_LOGGING_CATEGORY(logBrewBackend)

#endif // QHOMEBREWUPDATERBACKEND_H