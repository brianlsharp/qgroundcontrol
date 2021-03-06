/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


#include "ComplexMissionItemTest.h"

ComplexMissionItemTest::ComplexMissionItemTest(void)
{    
    _polyPoints << QGeoCoordinate(47.633550640000003, -122.08982199) << QGeoCoordinate(47.634129020000003, -122.08887249) <<
                  QGeoCoordinate(47.633619320000001, -122.08811074) << QGeoCoordinate(47.633189139999999, -122.08900124);
}

void ComplexMissionItemTest::init(void)
{
    _rgComplexMissionItemSignals[polygonPathChangedIndex] =         SIGNAL(polygonPathChanged());
    _rgComplexMissionItemSignals[lastSequenceNumberChangedIndex] =  SIGNAL(lastSequenceNumberChanged(int));
    _rgComplexMissionItemSignals[altitudeChangedIndex] =            SIGNAL(altitudeChanged(double));
    _rgComplexMissionItemSignals[gridAngleChangedIndex] =           SIGNAL(gridAngleChanged(double));
    _rgComplexMissionItemSignals[gridPointsChangedIndex] =          SIGNAL(gridPointsChanged());
    _rgComplexMissionItemSignals[cameraTriggerChangedIndex] =       SIGNAL(cameraTriggerChanged(bool));

    _rgComplexMissionItemSignals[altDifferenceChangedIndex] =           SIGNAL(altDifferenceChanged(double));
    _rgComplexMissionItemSignals[altPercentChangedIndex] =              SIGNAL(altPercentChanged(double));
    _rgComplexMissionItemSignals[azimuthChangedIndex] =                 SIGNAL(azimuthChanged(double));
    _rgComplexMissionItemSignals[commandDescriptionChangedIndex] =      SIGNAL(commandDescriptionChanged());
    _rgComplexMissionItemSignals[commandNameChangedIndex] =             SIGNAL(commandNameChanged());
    _rgComplexMissionItemSignals[abbreviationChangedIndex] =            SIGNAL(abbreviationChanged());
    _rgComplexMissionItemSignals[coordinateChangedIndex] =              SIGNAL(coordinateChanged(const QGeoCoordinate&));
    _rgComplexMissionItemSignals[exitCoordinateChangedIndex] =          SIGNAL(exitCoordinateChanged(const QGeoCoordinate&));
    _rgComplexMissionItemSignals[dirtyChangedIndex] =                   SIGNAL(dirtyChanged(bool));
    _rgComplexMissionItemSignals[distanceChangedIndex] =                SIGNAL(distanceChanged(double));
    _rgComplexMissionItemSignals[isCurrentItemChangedIndex] =           SIGNAL(isCurrentItemChanged(bool));
    _rgComplexMissionItemSignals[sequenceNumberChangedIndex] =          SIGNAL(sequenceNumberChanged(int));
    _rgComplexMissionItemSignals[isSimpleItemChangedIndex] =            SIGNAL(isSimpleItemChanged(bool));
    _rgComplexMissionItemSignals[specifiesCoordinateChangedIndex] =     SIGNAL(specifiesCoordinateChanged());
    _rgComplexMissionItemSignals[isStandaloneCoordinateChangedIndex] =  SIGNAL(isStandaloneCoordinateChanged());

    _rgComplexMissionItemSignals[coordinateHasRelativeAltitudeChangedIndex] =       SIGNAL(coordinateHasRelativeAltitudeChanged(bool));
    _rgComplexMissionItemSignals[exitCoordinateHasRelativeAltitudeChangedIndex] =   SIGNAL(exitCoordinateHasRelativeAltitudeChanged(bool));
    _rgComplexMissionItemSignals[exitCoordinateSameAsEntryChangedIndex] =           SIGNAL(exitCoordinateSameAsEntryChanged(bool));

    _complexItem = new SurveyMissionItem(NULL /* Vehicle */, this);

    // It's important to check that the right signals are emitted at the right time since that drives ui change.
    // It's also important to check that things are not being over-signalled when they should not be, since that can lead
    // to incorrect ui or perf impact of uneeded signals propogating ui change.

    _multiSpy = new MultiSignalSpy();
    Q_CHECK_PTR(_multiSpy);
    QCOMPARE(_multiSpy->init(_complexItem, _rgComplexMissionItemSignals, _cComplexMissionItemSignals), true);
}

void ComplexMissionItemTest::cleanup(void)
{
    delete _complexItem;
    delete _multiSpy;
}

void ComplexMissionItemTest::_testDirty(void)
{
    QVERIFY(!_complexItem->dirty());
    _complexItem->setDirty(false);
    QVERIFY(!_complexItem->dirty());
    QVERIFY(_multiSpy->checkNoSignals());
    _complexItem->setDirty(true);
    QVERIFY(_complexItem->dirty());
    QVERIFY(_multiSpy->checkOnlySignalByMask(dirtyChangedMask));
    QVERIFY(_multiSpy->pullBoolFromSignalIndex(dirtyChangedIndex));
    _multiSpy->clearAllSignals();
    _complexItem->setDirty(false);
    QVERIFY(!_complexItem->dirty());
    QVERIFY(_multiSpy->checkOnlySignalByMask(dirtyChangedMask));
    QVERIFY(!_multiSpy->pullBoolFromSignalIndex(dirtyChangedIndex));
}

void ComplexMissionItemTest::_testAddPolygonCoordinate(void)
{
    QCOMPARE(_complexItem->polygonPath().count(), 0);

    // First call to addPolygonCoordinate should trigger:
    //      polygonPathChanged
    //      dirtyChanged

    _complexItem->addPolygonCoordinate(_polyPoints[0]);
    QVERIFY(_multiSpy->checkOnlySignalByMask(polygonPathChangedMask | dirtyChangedMask));

    // Validate object data
    QVariantList polyList = _complexItem->polygonPath();
    QCOMPARE(polyList.count(), 1);
    QCOMPARE(polyList[0].value<QGeoCoordinate>(), _polyPoints[0]);

    // Reset
    _complexItem->setDirty(false);
    _multiSpy->clearAllSignals();

    // Second call to addPolygonCoordinate should only trigger:
    //      polygonPathChanged
    //      dirtyChanged

    _complexItem->addPolygonCoordinate(_polyPoints[1]);
    QVERIFY(_multiSpy->checkOnlySignalByMask(polygonPathChangedMask | dirtyChangedMask));

    polyList = _complexItem->polygonPath();
    QCOMPARE(polyList.count(), 2);
    for (int i=0; i<polyList.count(); i++) {
        QCOMPARE(polyList[i].value<QGeoCoordinate>(), _polyPoints[i]);
    }

    _complexItem->setDirty(false);
    _multiSpy->clearAllSignals();

    // Third call to addPolygonCoordinate should trigger:
    //      polygonPathChanged
    //      dirtyChanged
    // Grid is generated for the first time on closing of polygon which triggers:
    //      coordinateChanged - grid generates new entry coordinate
    //      exitCoordinateChanged - grid generates new exit coordinate
    //      specifiesCoordinateChanged - once grid entry/exit shows up specifiesCoordinate gets set to true
    // Grid generation triggers the following signals
    //      lastSequenceNumberChanged -  number of internal mission items changes
    //      gridPointsChanged - grid points show up for the first time

    _complexItem->addPolygonCoordinate(_polyPoints[2]);
    QVERIFY(_multiSpy->checkOnlySignalByMask(polygonPathChangedMask | lastSequenceNumberChangedMask | gridPointsChangedMask | coordinateChangedMask |
                                            exitCoordinateChangedMask | specifiesCoordinateChangedMask | dirtyChangedMask));
    int seqNum = _multiSpy->pullIntFromSignalIndex(lastSequenceNumberChangedIndex);
    QVERIFY(seqNum > 0);

    polyList = _complexItem->polygonPath();
    QCOMPARE(polyList.count(), 3);
    for (int i=0; i<polyList.count(); i++) {
        QCOMPARE(polyList[i].value<QGeoCoordinate>(), _polyPoints[i]);
    }

    // Test that number of waypoints is doubled when using turnaround waypoints
    _complexItem->setTurnaroundDist(60.0);
    QVariantList gridPoints = _complexItem->gridPoints();
    _complexItem->setTurnaroundDist(0.0);
    QVariantList gridPointsNoT = _complexItem->gridPoints();
    QCOMPARE(gridPoints.count(), 2 * gridPointsNoT.count());

}

void ComplexMissionItemTest::_testClearPolygon(void)
{
    for (int i=0; i<3; i++) {
        _complexItem->addPolygonCoordinate(_polyPoints[i]);
    }
    _complexItem->setDirty(false);
    _multiSpy->clearAllSignals();

    // Call to clearPolygon should trigger:
    //      polygonPathChangedMask
    //      dirtyChanged
    //      lastSequenceNumberChangedMask
    //      gridPointsChangedMask
    //      dirtyChangedMask
    //      specifiesCoordinateChangedMask

    _complexItem->clearPolygon();
    QVERIFY(_multiSpy->checkOnlySignalByMask(polygonPathChangedMask | lastSequenceNumberChangedMask | gridPointsChangedMask | dirtyChangedMask |
                                             specifiesCoordinateChangedMask));
    QVERIFY(!_multiSpy->pullBoolFromSignalIndex(specifiesCoordinateChangedIndex));
    QCOMPARE(_multiSpy->pullIntFromSignalIndex(lastSequenceNumberChangedIndex), 0);

    QCOMPARE(_complexItem->polygonPath().count(), 0);
    QCOMPARE(_complexItem->gridPoints().count(), 0);

    _complexItem->setDirty(false);
    _multiSpy->clearAllSignals();
}

void ComplexMissionItemTest::_testCameraTrigger(void)
{
    QCOMPARE(_complexItem->property("cameraTrigger").toBool(), true);

    // Turning on/off camera triggering while there is no grid should trigger:
    //      cameraTriggerChanged
    //      dirtyChanged
    // lastSequenceNumber should not change

    int lastSeq = _complexItem->lastSequenceNumber();

    _complexItem->setProperty("cameraTrigger", false);
    QVERIFY(_multiSpy->checkOnlySignalByMask(dirtyChangedMask | cameraTriggerChangedMask));
    QVERIFY(!_multiSpy->pullBoolFromSignalIndex(cameraTriggerChangedIndex));
    QCOMPARE(_complexItem->lastSequenceNumber(), lastSeq);

    _complexItem->setDirty(false);
    _multiSpy->clearAllSignals();

    _complexItem->setProperty("cameraTrigger", true);
    QVERIFY(_multiSpy->checkOnlySignalByMask(dirtyChangedMask | cameraTriggerChangedMask));
    QVERIFY(_multiSpy->pullBoolFromSignalIndex(cameraTriggerChangedIndex));
    QCOMPARE(_complexItem->lastSequenceNumber(), lastSeq);

    // Set up a grid

    for (int i=0; i<3; i++) {
        _complexItem->addPolygonCoordinate(_polyPoints[i]);
    }

    _complexItem->setDirty(false);
    _multiSpy->clearAllSignals();

    lastSeq = _complexItem->lastSequenceNumber();
    QVERIFY(lastSeq > 0);

    // Turning off camera triggering should remove two camera trigger mission items, this should trigger:
    //      lastSequenceNumberChanged
    //      dirtyChanged

    _complexItem->setProperty("cameraTrigger", false);
    QVERIFY(_multiSpy->checkOnlySignalByMask(lastSequenceNumberChangedMask | dirtyChangedMask | cameraTriggerChangedMask));
    QCOMPARE(_multiSpy->pullIntFromSignalIndex(lastSequenceNumberChangedIndex), lastSeq - 2);

    _complexItem->setDirty(false);
    _multiSpy->clearAllSignals();

    // Turn on camera triggering and make sure things go back to previous count

    _complexItem->setProperty("cameraTrigger", true);
    QVERIFY(_multiSpy->checkOnlySignalByMask(lastSequenceNumberChangedMask | dirtyChangedMask | cameraTriggerChangedMask));
    QCOMPARE(_multiSpy->pullIntFromSignalIndex(lastSequenceNumberChangedIndex), lastSeq);
}
