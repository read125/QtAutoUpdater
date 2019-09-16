#include "plugintest.h"
#include <QtCore/private/qfactoryloader_p.h>
#include <QtAutoUpdaterCore/private/updater_p.h>
using namespace QtAutoUpdater;
using namespace std::chrono;
using namespace std::chrono_literals;

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
						  (QtAutoUpdater_UpdaterPlugin_iid,
						   QLatin1String("/updaters")))

bool PluginTest::init()
{
	return true;
}

bool PluginTest::cleanup()
{
	return true;
}

void PluginTest::initTestCase()
{
	QVERIFY(init());
}

void PluginTest::cleanupTestCase()
{
	QVERIFY(cleanup());
}

void PluginTest::testUpdateCheck_data()
{
	QTest::addColumn<QVersionNumber>("installedVersion");
	QTest::addColumn<QVersionNumber>("updateVersion");
	QTest::addColumn<int>("abortLevel");
	QTest::addColumn<bool>("success");

	QTest::newRow("noUpdates") << QVersionNumber(1, 0, 0)
							   << QVersionNumber(1, 0, 0)
							   << 0
							   << true;

	QTest::newRow("simpleUpdate") << QVersionNumber(1, 0, 0)
								  << QVersionNumber(1, 1, 0)
								  << 0
								  << true;

	QTest::newRow("newerInstall") << QVersionNumber(1, 1, 0)
								  << QVersionNumber(1, 0, 0)
								  << 0
								  << true;

	QTest::newRow("abortSoft") << QVersionNumber(1, 0, 0)
							   << QVersionNumber(1, 1, 0)
							   << 1
							   << cancelState();

	QTest::newRow("abortHard") << QVersionNumber(1, 0, 0)
							   << QVersionNumber(1, 1, 0)
							   << 2
							   << cancelState();
}

void PluginTest::testUpdateCheck()
{
	QFETCH(QVersionNumber, installedVersion);
	QFETCH(QVersionNumber, updateVersion);
	QFETCH(int, abortLevel);
	QFETCH(bool, success);

	if (abortLevel > 0) {
		if (!canAbort(abortLevel == 2)) {
			QEXPECT_FAIL("", "Backend does not support given abort level", Abort);
			QVERIFY(false);
		}
	}

	QVERIFY(simulateInstall(installedVersion));
	QVERIFY(prepareUpdate(updateVersion));
	const auto updates = abortLevel > 0 ?
							 QList<UpdateInfo>{} :
							 createInfos(installedVersion, updateVersion);

	sptr updater { loadBackend() };
	if (!updater)
		return;

	QSignalSpy doneSpy{updater.data(), &UpdaterBackend::checkDone};
	QVERIFY(doneSpy.isValid());

	//start the check updates
	updater->checkForUpdates();

	if (abortLevel > 0) {
		if (!doneSpy.wait(0)) {
			updater->abort(abortLevel == 2);
			QVERIFY(doneSpy.wait(60000));
		}
	} else  //wait max 1 min for the process to finish
		QVERIFY(doneSpy.wait(60000));

	//check if the finished signal is without error
	QCOMPARE(doneSpy.size(), 1);
	QCOMPARE(doneSpy[0][0].toBool(), success);
	QCOMPARE(doneSpy[0][1].value<QList<UpdateInfo>>(), updates);
}

void PluginTest::testTriggerInstall()
{
	QVERIFY(simulateInstall(QVersionNumber{1,0,0}));
	QVERIFY(prepareUpdate(QVersionNumber{1,1,0}));

	sptr updater { loadBackend() };
	QVERIFY(updater);

	if (!updater->features().testFlag(UpdaterBackend::Feature::TriggerInstall)) {
		QEXPECT_FAIL("", "Backend does not support triggered installations", Abort);
		QVERIFY(false);
	}
	if (!updater->features().testFlag(UpdaterBackend::Feature::ParallelTrigger)) {  // TODO refactor because of flag change
		QEXPECT_FAIL("", "Backend does not support parallel installations", Abort);
		QVERIFY(false);
	}

	// check for updates
	QSignalSpy doneSpy{updater.data(), &UpdaterBackend::checkDone};
	QVERIFY(doneSpy.isValid());
	updater->checkForUpdates();
	QVERIFY(doneSpy.wait(60000));
	QCOMPARE(doneSpy.size(), 1);
	QCOMPARE(doneSpy[0][0].toBool(), true);
	const auto updates = doneSpy[0][1].value<QList<UpdateInfo>>();
	QVERIFY(updates.size() > 0);

	// install the updates
	QSignalSpy installSpy{updater.data(), &UpdaterBackend::triggerInstallDone};
	QVERIFY(installSpy.isValid());
	QVERIFY(updater->triggerUpdates(updates, true));
	QVERIFY(installSpy.wait(60000));
	QCOMPARE(installSpy.size(), 1);
	QCOMPARE(installSpy.takeFirst()[0].toBool(), true);

	// check for updates again to make shure there are none
	doneSpy.clear();
	updater->checkForUpdates();
	QVERIFY(doneSpy.wait(60000));
	QCOMPARE(doneSpy.size(), 1);
	QCOMPARE(doneSpy[0][0].toBool(), true);
	QVERIFY(doneSpy[0][1].value<QList<UpdateInfo>>().isEmpty());
}

void PluginTest::testPerformInstall()
{
	sptr updater { loadBackend() };
	QVERIFY(updater);

	if (!updater->features().testFlag(UpdaterBackend::Feature::PerformInstall)) {
		QEXPECT_FAIL("", "Backend does not support performed installations", Abort);
		QVERIFY(false);
	}
}

UpdaterBackend *PluginTest::loadBackend()
{
	UpdaterBackend *backendRes = nullptr;
	[&]() {
		auto uBackend = qLoadPlugin<UpdaterBackend, UpdaterPlugin>(loader, backend(), this);
		QVERIFY(uBackend);
		auto reader = new VariantConfigReader{backend(), config()};
		QVERIFY(uBackend->initialize(QScopedPointer<UpdaterBackend::IConfigReader>{reader}));
		backendRes = uBackend;
	}();
	return backendRes;
}
