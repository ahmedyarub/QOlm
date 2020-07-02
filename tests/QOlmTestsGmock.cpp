#include <QSignalSpy>
#include <QDateTime>
#include <QDebug>
#include <QObject>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <QOlm/QOlm.hpp>

class MockQOlm : public qolm::QOlm<QObject>
{
public:
    MOCK_METHOD2(onObjectAboutToBeInserted, void (QObject* object, int index));
    MOCK_METHOD2(onObjectInserted, void (QObject* object, int index));
    MOCK_METHOD3(onObjectAboutToBeMoved, void (QObject* object, int src, int dest));
    MOCK_METHOD3(onObjectMoved, void (QObject* object, int src, int dest));
    MOCK_METHOD2(onObjectAboutToBeRemoved, void (QObject* object, int index));
    MOCK_METHOD2(onObjectRemoved, void (QObject* object, int index));
};

TEST(QOlmTest, onObjectAboutToBeInsertedTest)
{
  MockQOlm qolm;
  EXPECT_CALL(qolm, onObjectAboutToBeInserted(new QObject(), 5))
    .Times(4);
}

TEST(QOlmTest, onObjectInsertedTest)
{
  MockQOlm qolm;
  EXPECT_CALL(qolm, onObjectInserted(new QObject(), 5))
    .Times(4);
}

TEST(QOlmTest, onObjectAboutToBeMovedTest)
{
  MockQOlm qolm;
  EXPECT_CALL(qolm, onObjectAboutToBeMoved(new QObject(), 5, 6))
    .Times(4);
}

TEST(QOlmTest, onObjectMovedTest)
{
  MockQOlm qolm;
  EXPECT_CALL(qolm, onObjectMoved(new QObject(), 5, 6))
    .Times(4);
}

TEST(QOlmTest, onObjectAboutToBeRemovedTest)
{
  MockQOlm qolm;
  EXPECT_CALL(qolm, onObjectAboutToBeRemoved(new QObject(), 5))
    .Times(4);
}

TEST(QOlmTest, onObjectRemovedTest)
{
  MockQOlm qolm;
  EXPECT_CALL(qolm, onObjectRemoved(new QObject(), 5))
    .Times(4);
}