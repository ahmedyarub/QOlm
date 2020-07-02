// Spy signal call without having a main loop
#include <QSignalSpy>
#include <QDateTime>
#include <QDebug>

// gtest framework
#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Our test classes
#include <QOlm/QOlm.hpp>

class QOlmTestListFilled : public ::testing::Test
{
protected:
    void SetUp() override { list.append({foo1, foo2, foo3, foo4}); }
    void TearDown() override {}

    qolm::QOlm<QObject> list;
    QObject* foo1 = new QObject(&list);
    QObject* foo2 = new QObject(&list);
    QObject* foo3 = new QObject(&list);
    QObject* foo4 = new QObject(&list);
    QObject* foo5 = new QObject(&list);
};

TEST_F(QOlmTestListFilled, At)
{
    ASSERT_EQ(list.at(2), foo3);
    ASSERT_EQ(list.at(4), nullptr);
    ASSERT_EQ(list.at(-1), nullptr);
}

TEST_F(QOlmTestListFilled, Get)
{
    ASSERT_EQ(list.get(1), foo2);
    ASSERT_EQ(list.get(4), nullptr);
    ASSERT_EQ(list.get(-1), nullptr);
}

TEST_F(QOlmTestListFilled, Contains)
{
    ASSERT_TRUE(list.contains(foo1));
    ASSERT_TRUE(list.contains(foo2));
    ASSERT_TRUE(list.contains(foo3));
    ASSERT_TRUE(list.contains(foo4));

    //With a null pointer
    ASSERT_FALSE(list.contains(foo5));
    ASSERT_FALSE(list.contains(nullptr));
}

TEST_F(QOlmTestListFilled, IndexOf)
{
    ASSERT_EQ(list.indexOf(foo1), 0);
    ASSERT_EQ(list.indexOf(foo2), 1);
    ASSERT_EQ(list.indexOf(foo3), 2);

    //With a null pointer
    ASSERT_EQ(list.indexOf(nullptr), -1);
    ASSERT_EQ(list.indexOf(foo5), -1);
}

template<class T = QObject>
class QOlmMock : public qolm::QOlm<T>
{
public:
    MOCK_METHOD(void, onObjectAboutToBeInserted, (T * object, int index), (override));
    MOCK_METHOD(void, onObjectInserted, (T * object, int index), (override));
    MOCK_METHOD(void, onObjectAboutToBeMoved, (T * object, int src, int dest), (override));
    MOCK_METHOD(void, onObjectMoved, (T * object, int src, int dest), (override));
    MOCK_METHOD(void, onObjectAboutToBeRemoved, (T * object, int index), (override));
    MOCK_METHOD(void, onObjectRemoved, (T * object, int index), (override));
};

// First basic test fixture that have only one QObject
class QOlmTestEmptyList : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}

    QOlmMock<QObject> list;
    QObject* foo1 = new QObject(&list);
    QObject* foo2 = new QObject(&list);
    QObject* foo3 = new QObject(&list);
};

TEST_F(QOlmTestEmptyList, AppendQObject)
{
    QSignalSpy spyAboutToInsert(&list, &qolm::QOlmBase::rowsAboutToBeInserted);
    QSignalSpy spyInsert(&list, &qolm::QOlmBase::rowsInserted);

    EXPECT_CALL(list, onObjectAboutToBeInserted(foo1, 0));
    EXPECT_CALL(list, onObjectInserted(foo1, 0));
    EXPECT_CALL(list, onObjectAboutToBeInserted(foo2, 1));
    EXPECT_CALL(list, onObjectInserted(foo2, 1));
    EXPECT_CALL(list, onObjectAboutToBeInserted(foo1, 2));
    EXPECT_CALL(list, onObjectInserted(foo1, 2));

    ASSERT_EQ(list.size(), 0);

    bool expectedCallback = false;
    int expectedIndex = -1;
    QObject* expectedObject = nullptr;

    list.onInserted(
        [&](const auto& args)
        {
            ASSERT_TRUE(expectedCallback);
            ASSERT_EQ(expectedIndex, args.index);
            ASSERT_EQ(expectedObject, args.object);

            expectedCallback = false;
            expectedIndex = -1;
            expectedObject = nullptr;
        });
    list.onMoved([&](const auto& args) { ASSERT_TRUE(false); });
    list.onRemoved([&](const auto& args) { ASSERT_TRUE(false); });

    expectedCallback = true;
    expectedIndex = 0;
    expectedObject = foo1;
    list.append(foo1);

    ASSERT_EQ(list.size(), 1);
    ASSERT_EQ(spyAboutToInsert.count(), 1);
    ASSERT_EQ(spyInsert.count(), 1);
    {
        const auto args = spyAboutToInsert.takeFirst();
        const auto first = args.at(1).toInt();
        const auto last = args.at(2).toInt();
        ASSERT_EQ(first, 0);
        ASSERT_EQ(last, 0);
    }
    {
        const auto args = spyInsert.takeFirst();
        const auto first = args.at(1).toInt();
        const auto last = args.at(2).toInt();
        ASSERT_EQ(first, 0);
        ASSERT_EQ(last, 0);
    }

    expectedCallback = true;
    expectedIndex = 1;
    expectedObject = foo2;
    list.append(foo2);

    expectedCallback = true;
    expectedIndex = 2;
    expectedObject = foo1;
    list.append(foo1);

    ASSERT_EQ(list.size(), 3);
    ASSERT_EQ(spyAboutToInsert.count(), 2);
    ASSERT_EQ(spyInsert.count(), 2);

    {
        const auto args = spyAboutToInsert.takeFirst();
        const auto first = args.at(1).toInt();
        const auto last = args.at(2).toInt();
        ASSERT_EQ(first, 1);
        ASSERT_EQ(last, 1);
    }
    {
        const auto args = spyAboutToInsert.takeFirst();
        const auto first = args.at(1).toInt();
        const auto last = args.at(2).toInt();
        ASSERT_EQ(first, 2);
        ASSERT_EQ(last, 2);
    }
    {
        const auto args = spyInsert.takeFirst();
        const auto first = args.at(1).toInt();
        const auto last = args.at(2).toInt();
        ASSERT_EQ(first, 1);
        ASSERT_EQ(last, 1);
    }
    {
        const auto args = spyInsert.takeFirst();
        const auto first = args.at(1).toInt();
        const auto last = args.at(2).toInt();
        ASSERT_EQ(first, 2);
        ASSERT_EQ(last, 2);
    }

    ASSERT_EQ(list.get(0), foo1);
    ASSERT_EQ(list.get(1), foo2);
    ASSERT_EQ(list.get(2), foo1);

    //Append a null pointer Object. Not signal should be called
    list.append(nullptr);

    ASSERT_EQ(list.size(), 3);
    ASSERT_EQ(spyAboutToInsert.count(), 0);
    ASSERT_EQ(spyInsert.count(), 0);

    // Check list is untouched
    ASSERT_EQ(list.get(0), foo1);
    ASSERT_EQ(list.get(1), foo2);
    ASSERT_EQ(list.get(2), foo1);
}

TEST_F(QOlmTestEmptyList, PrependQObject)
{
    QSignalSpy spyAboutToInsert(&list, &qolm::QOlmBase::rowsAboutToBeInserted);
    QSignalSpy spyInsert(&list, &qolm::QOlmBase::rowsInserted);
    ASSERT_EQ(list.size(), 0);

    list.prepend(foo1);

    ASSERT_EQ(list.size(), 1);
    ASSERT_EQ(spyAboutToInsert.count(), 1);
    ASSERT_EQ(spyInsert.count(), 1);
    ASSERT_EQ(list.get(0), foo1);

    list.prepend(foo2);

    ASSERT_EQ(list.size(), 2);
    ASSERT_EQ(spyAboutToInsert.count(), 2);
    ASSERT_EQ(spyInsert.count(), 2);

    ASSERT_EQ(list.get(0), foo2);
    ASSERT_EQ(list.get(1), foo1);

    //Prepend a null pointer Object
    list.prepend(nullptr);

    ASSERT_EQ(list.size(), 2);
    ASSERT_EQ(spyAboutToInsert.count(), 2);
    ASSERT_EQ(spyInsert.count(), 2);

    ASSERT_EQ(list.get(0), foo2);
    ASSERT_EQ(list.get(1), foo1);
}

TEST_F(QOlmTestEmptyList, InsertQObject)
{
    QSignalSpy spyAboutToInsert(&list, &qolm::QOlmBase::rowsAboutToBeInserted);
    QSignalSpy spyInsert(&list, &qolm::QOlmBase::rowsInserted);
    ASSERT_EQ(list.size(), 0);

    list.insert(0, foo1);

    ASSERT_EQ(list.size(), 1);
    ASSERT_EQ(spyAboutToInsert.count(), 1);
    ASSERT_EQ(spyInsert.count(), 1);

    list.insert(0, foo2);

    ASSERT_EQ(list.size(), 2);
    ASSERT_EQ(spyAboutToInsert.count(), 2);
    ASSERT_EQ(spyInsert.count(), 2);

    ASSERT_EQ(list.get(0), foo2);
    ASSERT_EQ(list.get(1), foo1);

    // if the user put an index greater than the maximal index in the list, the object will be inserted get the end of the list

    list.insert(4, foo2);

    ASSERT_EQ(list.size(), 3);
    ASSERT_EQ(spyAboutToInsert.count(), 3);
    ASSERT_EQ(spyInsert.count(), 3);

    ASSERT_EQ(list.get(0), foo2);
    ASSERT_EQ(list.get(1), foo1);
    ASSERT_EQ(list.get(2), foo2);

    //if the user insert an object get an index which already contains an object, it will create a shift in the list with the next objects

    list.insert(1, foo2);

    ASSERT_EQ(list.size(), 4);
    ASSERT_EQ(spyAboutToInsert.count(), 4);
    ASSERT_EQ(spyInsert.count(), 4);

    ASSERT_EQ(list.get(0), foo2);
    ASSERT_EQ(list.get(1), foo2);
    ASSERT_EQ(list.get(2), foo1);
    ASSERT_EQ(list.get(3), foo2);
}

TEST_F(QOlmTestEmptyList, InsertList)
{
    QSignalSpy spyAboutToInsert(&list, &qolm::QOlmBase::rowsAboutToBeInserted);
    QSignalSpy spyInsert(&list, &qolm::QOlmBase::rowsInserted);
    ASSERT_EQ(list.size(), 0);

    QList<QObject*> mylist = {foo1, foo2, foo3};
    list.insert(0, mylist);

    ASSERT_EQ(list.size(), 3);
    ASSERT_EQ(spyAboutToInsert.count(), 1);
    ASSERT_EQ(spyInsert.count(), 1);

    ASSERT_EQ(list.get(0), foo1);
    ASSERT_EQ(list.get(1), foo2);
    ASSERT_EQ(list.get(2), foo3);

    list.insert(1, mylist);

    ASSERT_EQ(list.size(), 6);
    ASSERT_EQ(spyAboutToInsert.count(), 2);
    ASSERT_EQ(spyInsert.count(), 2);

    ASSERT_EQ(list.get(0), foo1);
    ASSERT_EQ(list.get(1), foo1);
    ASSERT_EQ(list.get(2), foo2);
    ASSERT_EQ(list.get(3), foo3);
    ASSERT_EQ(list.get(4), foo2);
    ASSERT_EQ(list.get(5), foo3);

    //Empty List
    list.insert(2, QList<QObject*>());
    ASSERT_EQ(list.size(), 6);
    ASSERT_EQ(spyAboutToInsert.count(), 2);
    ASSERT_EQ(spyInsert.count(), 2);

    ASSERT_EQ(list.get(0), foo1);
    ASSERT_EQ(list.get(1), foo1);
    ASSERT_EQ(list.get(2), foo2);
    ASSERT_EQ(list.get(3), foo3);
    ASSERT_EQ(list.get(4), foo2);
    ASSERT_EQ(list.get(5), foo3);
}

TEST_F(QOlmTestEmptyList, AppendList)
{
    QSignalSpy spyAboutToInsert(&list, &qolm::QOlmBase::rowsAboutToBeInserted);
    QSignalSpy spyInsert(&list, &qolm::QOlmBase::rowsInserted);
    ASSERT_EQ(list.size(), 0);

    list.append({foo1, foo2, foo3});

    ASSERT_EQ(list.size(), 3);
    ASSERT_EQ(spyAboutToInsert.count(), 1);
    ASSERT_EQ(spyInsert.count(), 1);

    ASSERT_EQ(list.get(0), foo1);
    ASSERT_EQ(list.get(1), foo2);
    ASSERT_EQ(list.get(2), foo3);

    list.append({foo2, foo1, foo3});

    ASSERT_EQ(list.size(), 6);
    ASSERT_EQ(spyAboutToInsert.count(), 2);
    ASSERT_EQ(spyInsert.count(), 2);

    ASSERT_EQ(list.get(0), foo1);
    ASSERT_EQ(list.get(1), foo2);
    ASSERT_EQ(list.get(2), foo3);
    ASSERT_EQ(list.get(3), foo2);
    ASSERT_EQ(list.get(4), foo1);
    ASSERT_EQ(list.get(5), foo3);

    //Empty List
    list.append(QList<QObject*>());
    ASSERT_EQ(list.size(), 6);
    ASSERT_EQ(spyAboutToInsert.count(), 2);
    ASSERT_EQ(spyInsert.count(), 2);
}

TEST_F(QOlmTestEmptyList, PrependList)
{
    QSignalSpy spyAboutToInsert(&list, &qolm::QOlmBase::rowsAboutToBeInserted);
    QSignalSpy spyInsert(&list, &qolm::QOlmBase::rowsInserted);
    ASSERT_EQ(list.size(), 0);

    list.prepend({foo1, foo2, foo3});

    ASSERT_EQ(list.size(), 3);
    ASSERT_EQ(spyAboutToInsert.count(), 1);
    ASSERT_EQ(spyInsert.count(), 1);

    ASSERT_EQ(list.get(0), foo1);
    ASSERT_EQ(list.get(1), foo2);
    ASSERT_EQ(list.get(2), foo3);

    list.prepend({foo2, foo1, foo3});

    ASSERT_EQ(list.size(), 6);
    ASSERT_EQ(spyAboutToInsert.count(), 2);
    ASSERT_EQ(spyInsert.count(), 2);

    ASSERT_EQ(list.get(0), foo2);
    ASSERT_EQ(list.get(1), foo1);
    ASSERT_EQ(list.get(2), foo3);
    ASSERT_EQ(list.get(3), foo1);
    ASSERT_EQ(list.get(4), foo2);
    ASSERT_EQ(list.get(5), foo3);

    //Empty List
    list.prepend(QList<QObject*>());
    ASSERT_EQ(list.size(), 6);
    ASSERT_EQ(spyAboutToInsert.count(), 2);
    ASSERT_EQ(spyInsert.count(), 2);
}

TEST_F(QOlmTestListFilled, Move)
{
    QSignalSpy spyAboutToMoved(&list, &qolm::QOlmBase::rowsAboutToBeMoved);
    QSignalSpy spyMoved(&list, &qolm::QOlmBase::rowsMoved);
    ASSERT_EQ(list.size(), 4);

    list.move(0, 2);
    ASSERT_EQ(spyAboutToMoved.count(), 1);
    ASSERT_EQ(spyMoved.count(), 1);

    ASSERT_EQ(list.get(0), foo2);
    ASSERT_EQ(list.get(1), foo3);
    ASSERT_EQ(list.get(2), foo1);
    ASSERT_EQ(list.get(3), foo4);
}

TEST_F(QOlmTestListFilled, RemoveQObject)
{
    QSignalSpy spyAboutToRemoved(&list, &qolm::QOlmBase::rowsAboutToBeRemoved);
    QSignalSpy spyRemoved(&list, &qolm::QOlmBase::rowsRemoved);
    ASSERT_EQ(list.size(), 4);

    list.remove(foo2);
    ASSERT_EQ(spyAboutToRemoved.count(), 1);
    ASSERT_EQ(spyRemoved.count(), 1);

    ASSERT_EQ(list.get(0), foo1);
    ASSERT_EQ(list.get(1), foo3);
    ASSERT_EQ(list.get(2), foo4);
}

TEST_F(QOlmTestListFilled, RemoveList)
{
    QSignalSpy spyAboutToRemoved(&list, &qolm::QOlmBase::rowsAboutToBeRemoved);
    QSignalSpy spyRemoved(&list, &qolm::QOlmBase::rowsRemoved);
    ASSERT_EQ(list.size(), 4);

    list.remove({foo3, foo2});
    ASSERT_EQ(spyAboutToRemoved.count(), 2);
    ASSERT_EQ(spyRemoved.count(), 2);

    ASSERT_EQ(list.get(0), foo1);
    ASSERT_EQ(list.get(1), foo4);
}
TEST_F(QOlmTestListFilled, RemoveIndex)
{
    QSignalSpy spyAboutToRemoved(&list, &qolm::QOlmBase::rowsAboutToBeRemoved);
    QSignalSpy spyRemoved(&list, &qolm::QOlmBase::rowsRemoved);
    ASSERT_EQ(list.size(), 4);

    list.remove(3);
    ASSERT_EQ(spyAboutToRemoved.count(), 1);
    ASSERT_EQ(spyRemoved.count(), 1);

    ASSERT_EQ(list.get(0), foo1);
    ASSERT_EQ(list.get(1), foo2);
    ASSERT_EQ(list.get(2), foo3);

    list.remove(5);
    ASSERT_EQ(spyAboutToRemoved.count(), 1);
    ASSERT_EQ(spyRemoved.count(), 1);

    ASSERT_EQ(list.get(0), foo1);
    ASSERT_EQ(list.get(1), foo2);
    ASSERT_EQ(list.get(2), foo3);
}

TEST_F(QOlmTestListFilled, Clear)
{
    QSignalSpy spyAboutToRemoved(&list, &qolm::QOlmBase::rowsAboutToBeRemoved);
    QSignalSpy spyRemoved(&list, &qolm::QOlmBase::rowsRemoved);
    ASSERT_EQ(list.size(), 4);

    list.clear();
    ASSERT_EQ(list.size(), 0);
}

TEST_F(QOlmTestListFilled, First) { ASSERT_EQ(list.first(), foo1); }

TEST_F(QOlmTestEmptyList, First) { ASSERT_EQ(list.first(), nullptr); }

TEST_F(QOlmTestListFilled, Last) { ASSERT_EQ(list.last(), foo4); }
TEST_F(QOlmTestEmptyList, Last) { ASSERT_EQ(list.last(), nullptr); }

TEST_F(QOlmTestListFilled, ToList)
{
    QList<QObject*> myList = {foo1, foo2, foo3, foo4};
    ASSERT_EQ(list.toList(), myList);
}

// TEST_F(QOlmTest, AppendFuzz)
// {
//     auto initTime = QDateTime::currentMSecsSinceEpoch();
//     for(int i = 0; i < 10000; ++i)
//         _list.append(new QObject());
//     auto appendTime = QDateTime::currentMSecsSinceEpoch();
//     ASSERT_EQ(_list.size(), 10000);
//     int i = 0;
//     _list.clear();
//     auto clearTime = QDateTime::currentMSecsSinceEpoch();

//     qDebug("Append 10000 QObject Time : %llu ms. Clear time : %llu ms", (appendTime - initTime), (clearTime - appendTime));
// }
