#include <QtTest/QtTest>
#include <QtTest/qtestcase.h>
#include <QSignalSpy>
#include <QBuffer>
#include <QByteArray>
#include <QDebug>
#include "dataprocessor_p.h"
#include "unittests.h"

Q_DECLARE_METATYPE(QWebSocketProtocol::CloseCode)

//TODO: test on invalid handshakes; like the errors I got with FireFox (multiple values in field)
//TODO: test on valid handshakes; like the errors I got with FireFox (multiple values in field)
//TODO: test valid close codes
//TODO: test invalid close codes
//TODO: test valid UTF8 sequences
//DONE: test invalid UTF8 sequences
//TODO: test for valid fields
//DONE: test for invalid fields
//TODO: test for valid opcodes
//DONE: test for invalid opcodes
//TODO: test continuation frames for bad rsv fields, etc.
//TODO: test continuation frames for incomplete payload
//DONE: test continuation frames for too big payload
//DONE: test continuation frames for too small frame
//TODO: test valid frame sequences
//TODO: test invalid frame sequences
//TODO: test invalid UTF8 sequences in control/close frames

const quint8 FIN = 0x80;
const quint8 RSV1 = 0x40;
const quint8 RSV2 = 0x30;
const quint8 RSV3 = 0x10;
const quint8 MASK = 0x80;

class tst_DataProcessor : public QObject
{
    Q_OBJECT

public:
    tst_DataProcessor();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    /***************************************************************************
     * Happy Flows
     ***************************************************************************/
    /*!
      \brief Tests all kinds of valid binary frames, including zero length frames
      */
    void goodBinaryFrames();

    /*!
      \brief Tests all kinds of valid text frames, including zero length frames
      */
    void goodTextFrames();

    /*!
      Tests the DataProcessor for the correct handling of non-charactercodes
      Due to a workaround in QTextCodec, non-characters are treated as illegal.
      This workaround is not necessary anymore, and hence code should be changed in Qt
      to allow non-characters again.
     */
    void nonCharacterCodes();

    /***************************************************************************
     * Unhappy Flows
     ***************************************************************************/
    /*!
        \brief Tests the DataProcessor for correct handling of frames that don't contain the starting 2 bytes.
        This test is a border case, where not enough bytes are received to even start parsing a frame.
        This test does not test sequences of frames, only single frames are tested
     */
    void frameTooSmall();

    void continuationFrameTooSmall();
    /*!
        \brief Tests the DataProcessor for correct handling of frames that are oversized.
        This test does not test sequences of frames, only single frames are tested
     */
    void frameTooBig();

    void continuationFrameTooBig();

    /*!
        \brief Tests the DataProcessor for the correct handling of malformed frame headers.
        This test does not test sequences of frames, only single frames are tested
     */
    void invalidHeader();

    /*!
        \brief Tests the DataProcessor for the correct handling of invalid control frames.
        Invalid means: payload bigger than 125, frame is fragmented, ...
        This test does not test sequences of frames, only single frames are tested
     */
    void invalidControlFrame();

    /*!
        \brief Tests the DataProcess for the correct handling of incomplete size fields for large and big payloads.
     */
    void incompleteSizeField();

    /*!
        \brief Tests the DataProcessor for the correct handling of incomplete payloads.
        This includes:
        - incomplete length bytes for large and big payloads (16- and 64-bit values),
        - minimum size representation (see RFC 6455 paragraph 5.2),
        - frames that are too large (larger than MAX_INT in bytes)
        - incomplete payloads (less bytes than indicated in the size field)
        This test does not test sequences of frames, only single frames are tested
     */
    void incompletePayload();

    /*!
        \brief Tests the DataProcessor for the correct handling of invalid UTF-8 payloads.
        This test does not test sequences of frames, only single frames are tested
     */
    void invalidPayload();

    /*!
      Tests the DataProcessor for the correct handling of the minimum size representation requirement of RFC 6455 (see paragraph 5.2)
     */
    void minimumSizeRequirement();

    void invalidHeader_data();
    void incompleteSizeField_data();
    void incompletePayload_data();
    void invalidPayload_data();
    void minimumSizeRequirement_data();
    void invalidControlFrame_data();
    void frameTooBig_data();

    void nonCharacterCodes_data();

private:
    //helper function that constructs a new row of test data for invalid UTF8 sequences
    void invalidUTF8(const char *dataTag, const char *utf8Sequence);
    //helper function that constructs a new row of test data for invalid leading field values
    void invalidField(const char *dataTag, quint8 invalidFieldValue);
    //helper functions that construct a new row of test data for size fields that do not adhere to the minimum size requirement
    void minimumSize16Bit(quint8 controlCode, quint16 sizeInBytes);
    void minimumSize64Bit(quint8 controlCode, quint64 sizeInBytes);
    //helper function to construct a new row of test data containing frames with a payload size smaller than indicated in the header
    void incompleteFrame(quint8 controlCode, quint64 indicatedSize, quint64 actualPayloadSize);

    //helper function to construct a new row of test data containing text frames containing non-character code sequences
    void nonCharacterSequence(const char *sequence);

    void doTest();

    QString opCodeToString(quint8 opCode);
};

tst_DataProcessor::tst_DataProcessor()
{
}

void tst_DataProcessor::initTestCase()
{
    qRegisterMetaType<QWebSocketProtocol::CloseCode>("QWebSocketProtocol::CloseCode");
}

void tst_DataProcessor::cleanupTestCase()
{
}

void tst_DataProcessor::init()
{
}

void tst_DataProcessor::cleanup()
{
}

void tst_DataProcessor::goodBinaryFrames()
{
    QByteArray data;
    QBuffer buffer;
    DataProcessor dataProcessor;

    //TODO: add goodTextFrames_data() to provide more data

    //empty binary payload; this should be OK
    data.append((char)(FIN | QWebSocketProtocol::OC_BINARY)).append(char(0x0));
    buffer.setData(data);
    buffer.open(QIODevice::ReadWrite);

    QSignalSpy spyFrameReceived(&dataProcessor, SIGNAL(binaryFrameReceived(QByteArray,bool)));
    QSignalSpy spyMessageReceived(&dataProcessor, SIGNAL(binaryMessageReceived(QByteArray)));
    dataProcessor.process(&buffer);
    QCOMPARE(spyFrameReceived.count(), 1);
    QCOMPARE(spyMessageReceived.count(), 1);
    QList<QVariant> arguments = spyFrameReceived.takeFirst();
    QCOMPARE(arguments.at(0).toByteArray().length(), 0);
    arguments = spyMessageReceived.takeFirst();
    QCOMPARE(arguments.at(0).toByteArray().length(), 0);
    buffer.close();
    spyFrameReceived.clear();
    spyMessageReceived.clear();
    data.clear();
}

void tst_DataProcessor::goodTextFrames()
{
    QByteArray data;
    QBuffer buffer;
    DataProcessor dataProcessor;

    //TODO: add goodTextFrames_data() to provide more data

    //empty text payload; this should be OK
    data.append((char)(FIN | QWebSocketProtocol::OC_TEXT)).append(char(0x0));
    buffer.setData(data);
    buffer.open(QIODevice::ReadWrite);

    QSignalSpy spyFrameReceived(&dataProcessor, SIGNAL(textFrameReceived(QString,bool)));
    QSignalSpy spyMessageReceived(&dataProcessor, SIGNAL(textMessageReceived(QString)));
    dataProcessor.process(&buffer);
    QCOMPARE(spyFrameReceived.count(), 1);
    QCOMPARE(spyMessageReceived.count(), 1);
    QList<QVariant> arguments = spyFrameReceived.takeFirst();
    QCOMPARE(arguments.at(0).toString().length(), 0);
    arguments = spyMessageReceived.takeFirst();
    QCOMPARE(arguments.at(0).toString().length(), 0);
    buffer.close();
    spyFrameReceived.clear();
    spyMessageReceived.clear();
    data.clear();
}

void tst_DataProcessor::nonCharacterCodes()
{
    QFETCH(quint8, firstByte);
    QFETCH(quint8, secondByte);
    QFETCH(QByteArray, payload);

    QByteArray data;
    QBuffer buffer;
    DataProcessor dataProcessor;
    QSignalSpy frameSpy(&dataProcessor, SIGNAL(textFrameReceived(QString,bool)));
    QSignalSpy messageSpy(&dataProcessor, SIGNAL(textMessageReceived(QString)));

    data.append(firstByte).append(secondByte);
    data.append(payload);
    buffer.setData(data);
    buffer.open(QIODevice::ReadWrite);
    dataProcessor.process(&buffer);
    QEXPECT_FAIL(QTest::currentDataTag(), "Due to QTextCode interpeting non-characters unicode points as invalid (QTBUG-33229).", Abort);

    QCOMPARE(frameSpy.count(), 1);
    QCOMPARE(messageSpy.count(), 1);

    QVariantList arguments = frameSpy.takeFirst();
    QCOMPARE(arguments.at(0).value<QString>().toUtf8(), payload);
    arguments = messageSpy.takeFirst();
    QCOMPARE(arguments.at(0).value<QString>().toUtf8(), payload);
    buffer.close();
    frameSpy.clear();
    messageSpy.clear();
    data.clear();
}

void tst_DataProcessor::frameTooSmall()
{
    QByteArray data;
    QBuffer buffer;
    DataProcessor dataProcessor;

    //with nothing in the buffer, the dataProcessor should time out and the error should be CC_GOING_AWAY
    //meaning the socket will be closed
    buffer.setData(data);
    buffer.open(QIODevice::ReadWrite);
    QSignalSpy spy(&dataProcessor, SIGNAL(errorEncountered(QWebSocketProtocol::CloseCode,QString)));
    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), QWebSocketProtocol::CC_GOING_AWAY);
    spy.clear();
    buffer.close();
    data.clear();

    //only one byte; this is far too little; should get a time out as well and the error should be CC_GOING_AWAY
    //meaning the socket will be closed
    data.append(quint8('1'));   //put 1 byte in the buffer; this is too little
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);
    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), QWebSocketProtocol::CC_GOING_AWAY);
    buffer.close();
    spy.clear();
    data.clear();
}

void tst_DataProcessor::continuationFrameTooSmall()
{
    QByteArray data;
    QBuffer buffer;
    DataProcessor dataProcessor;

    //text frame with final bit not set
    data.append((char)(QWebSocketProtocol::OC_TEXT)).append(char(0x0));
    buffer.setData(data);
    buffer.open(QIODevice::ReadWrite);
    dataProcessor.process(&buffer);

    buffer.close();
    data.clear();

    //with nothing in the buffer, the dataProcessor should time out and the error should be CC_GOING_AWAY
    //meaning the socket will be closed
    buffer.setData(data);
    buffer.open(QIODevice::ReadWrite);
    QSignalSpy spy(&dataProcessor, SIGNAL(errorEncountered(QWebSocketProtocol::CloseCode,QString)));
    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), QWebSocketProtocol::CC_GOING_AWAY);
    spy.clear();
    buffer.close();
    data.clear();

    //text frame with final bit not set
    data.append((char)(QWebSocketProtocol::OC_TEXT)).append(char(0x0));
    buffer.setData(data);
    buffer.open(QIODevice::ReadWrite);
    dataProcessor.process(&buffer);

    buffer.close();
    data.clear();
    spy.clear();

    //only 1 byte follows in continuation frame; should time out with close code CC_GOING_AWAY
    data.append('a');
    buffer.setData(data);
    buffer.open(QIODevice::ReadWrite);

    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), QWebSocketProtocol::CC_GOING_AWAY);
    spy.clear();
    buffer.close();
}

void tst_DataProcessor::frameTooBig()
{
    doTest();
}

void tst_DataProcessor::continuationFrameTooBig()
{
    QByteArray data;
    QBuffer buffer;
    DataProcessor dataProcessor;
    quint64 swapped64 = qToBigEndian<quint64>(quint64(INT_MAX + 1));
    const char *wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped64));

    //text frame with final bit not set
    data.append((char)(QWebSocketProtocol::OC_TEXT)).append(char(0x0));
    buffer.setData(data);
    buffer.open(QIODevice::ReadWrite);
    dataProcessor.process(&buffer);

    buffer.close();
    data.clear();
    //continuation frame with size too big
    data.append((char)(FIN | QWebSocketProtocol::OC_CONTINUE)).append(char(127)).append(QByteArray(wireRepresentation, 8)).append(QByteArray());
    buffer.setData(data);
    buffer.open(QIODevice::ReadWrite);

    QSignalSpy spy(&dataProcessor, SIGNAL(errorEncountered(QWebSocketProtocol::CloseCode,QString)));
    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), QWebSocketProtocol::CC_TOO_MUCH_DATA);
    spy.clear();
    buffer.close();
}

void tst_DataProcessor::invalidHeader()
{
    doTest();
}

void tst_DataProcessor::invalidControlFrame()
{
    doTest();
}

void tst_DataProcessor::minimumSizeRequirement()
{
    doTest();
}

void tst_DataProcessor::invalidPayload()
{
    doTest();
}

void tst_DataProcessor::incompletePayload()
{
    doTest();
}

void tst_DataProcessor::incompleteSizeField()
{
    doTest();
}

//////////////////////////////////////////////////////////////////////////////////////////
/// HELPER FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////////
void tst_DataProcessor::doTest()
{
    QFETCH(quint8, firstByte);
    QFETCH(quint8, secondByte);
    QFETCH(QByteArray, payload);
    QFETCH(QWebSocketProtocol::CloseCode, expectedCloseCode);

    QByteArray data;
    QBuffer buffer;
    DataProcessor dataProcessor;
    QSignalSpy spy(&dataProcessor, SIGNAL(errorEncountered(QWebSocketProtocol::CloseCode,QString)));

    data.append(firstByte).append(secondByte);
    data.append(payload);
    buffer.setData(data);
    buffer.open(QIODevice::ReadWrite);
    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    QVariantList arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), expectedCloseCode);
    buffer.close();
    spy.clear();
    data.clear();
}

QString tst_DataProcessor::opCodeToString(quint8 opCode)
{
    QString frameType;
    switch (opCode)
    {
        case QWebSocketProtocol::OC_BINARY:
        {
            frameType = "Binary";
            break;
        }
        case QWebSocketProtocol::OC_TEXT:
        {
            frameType = "Text";
            break;
        }
        case QWebSocketProtocol::OC_PING:
        {
            frameType = "Ping";
            break;
        }
        case QWebSocketProtocol::OC_PONG:
        {
            frameType = "Pong";
            break;
        }
        case QWebSocketProtocol::OC_CLOSE:
        {
            frameType = "Close";
            break;
        }
        case QWebSocketProtocol::OC_CONTINUE:
        {
            frameType = "Continuation";
            break;
        }
        case QWebSocketProtocol::OC_RESERVED_3:
        {
            frameType = "Reserved3";
            break;
        }
        case QWebSocketProtocol::OC_RESERVED_4:
        {
            frameType = "Reserved5";
            break;
        }
        case QWebSocketProtocol::OC_RESERVED_5:
        {
            frameType = "Reserved5";
            break;
        }
        case QWebSocketProtocol::OC_RESERVED_6:
        {
            frameType = "Reserved6";
            break;
        }
        case QWebSocketProtocol::OC_RESERVED_7:
        {
            frameType = "Reserved7";
            break;
        }
        case QWebSocketProtocol::OC_RESERVED_B:
        {
            frameType = "ReservedB";
            break;
        }
        case QWebSocketProtocol::OC_RESERVED_C:
        {
            frameType = "ReservedC";
            break;
        }
        case QWebSocketProtocol::OC_RESERVED_D:
        {
            frameType = "ReservedD";
            break;
        }
        case QWebSocketProtocol::OC_RESERVED_E:
        {
            frameType = "ReservedE";
            break;
        }
        case QWebSocketProtocol::OC_RESERVED_F:
        {
            frameType = "ReservedF";
            break;
        }
        default:
        {
            //should never come here
            Q_ASSERT(false);
        }
    }
    return frameType;
}

void tst_DataProcessor::minimumSize16Bit(quint8 controlCode, quint16 sizeInBytes)
{
    QVERIFY((controlCode == QWebSocketProtocol::OC_BINARY) || (controlCode == QWebSocketProtocol::OC_TEXT));
    quint16 swapped16 = qToBigEndian<quint16>(0);
    const char *wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped16));
    QString frameType = opCodeToString(controlCode);
    QTest::newRow(frameType.append(QString(" frame with payload size %1, represented in 2 bytes").arg(sizeInBytes)).toStdString().data())
            << quint8(FIN | controlCode)
            << quint8(126)
            << QByteArray(wireRepresentation, 2)
            << QWebSocketProtocol::CC_PROTOCOL_ERROR;
}

void tst_DataProcessor::minimumSize64Bit(quint8 controlCode, quint64 sizeInBytes)
{
    QVERIFY((controlCode == QWebSocketProtocol::OC_BINARY) || (controlCode == QWebSocketProtocol::OC_TEXT));
    quint64 swapped64 = qToBigEndian<quint64>(sizeInBytes);
    const char *wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped64));
    QString frameType = opCodeToString(controlCode);

    QTest::newRow(frameType.append(QString(" frame with payload size %1, represented in 8 bytes").arg(sizeInBytes)).toStdString().data())
            << quint8(FIN | controlCode)
            << quint8(127)
            << QByteArray(wireRepresentation, 8)
            << QWebSocketProtocol::CC_PROTOCOL_ERROR;
}

void tst_DataProcessor::invalidUTF8(const char *dataTag, const char *utf8Sequence)
{
    QByteArray payload = QByteArray::fromHex(utf8Sequence);
    QTest::newRow(QString("Invalid UTF8-sequence: %1").arg(dataTag).toStdString().data())
            << quint8(FIN | QWebSocketProtocol::OC_TEXT)
            << quint8(payload.length())
            << payload
            << QWebSocketProtocol::CC_WRONG_DATATYPE;
}

void tst_DataProcessor::invalidField(const char *dataTag, quint8 invalidFieldValue)
{
    QTest::newRow(dataTag) << quint8(FIN | invalidFieldValue)
                           << quint8(0x00)
                           << QByteArray()
                           << QWebSocketProtocol::CC_PROTOCOL_ERROR;
}

void tst_DataProcessor::incompleteFrame(quint8 controlCode, quint64 indicatedSize, quint64 actualPayloadSize)
{
    QVERIFY((controlCode == QWebSocketProtocol::OC_TEXT) ||
            (controlCode == QWebSocketProtocol::OC_BINARY) ||
            (controlCode == QWebSocketProtocol::OC_CLOSE) ||
            (controlCode == QWebSocketProtocol::OC_PING) ||
            (controlCode == QWebSocketProtocol::OC_PONG));
    QVERIFY(indicatedSize > actualPayloadSize);
    QTest::addColumn<quint8>("firstByte");
    QTest::addColumn<quint8>("secondByte");
    QTest::addColumn<QByteArray>("payload");
    QTest::addColumn<QWebSocketProtocol::CloseCode>("expectedCloseCode");

    QString frameType = opCodeToString(controlCode);

    if (indicatedSize < 126)
    {
        QTest::newRow(frameType.append(QString(" frame with payload size %1, but only %2 bytes of data").arg(indicatedSize).arg(actualPayloadSize)).toStdString().data())
                << quint8(FIN | controlCode)
                << quint8(indicatedSize)
                << QByteArray(actualPayloadSize, 'a')
                << QWebSocketProtocol::CC_GOING_AWAY;
    }
    else if (indicatedSize <= 0xFFFFu)
    {
        quint16 swapped16 = qToBigEndian<quint16>(static_cast<quint16>(indicatedSize));
        const char *wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped16));
        QTest::newRow(frameType.append(QString(" frame with payload size %1, but only %2 bytes of data").arg(indicatedSize).arg(actualPayloadSize)).toStdString().data())
                << quint8(FIN | controlCode)
                << quint8(126)
                << QByteArray(wireRepresentation, 2).append(QByteArray(actualPayloadSize, 'a'))
                << QWebSocketProtocol::CC_GOING_AWAY;
    }
    else
    {
        quint64 swapped64 = qToBigEndian<quint64>(indicatedSize);
        const char *wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped64));
        QTest::newRow(frameType.append(QString(" frame with payload size %1, but only %2 bytes of data").arg(indicatedSize).arg(actualPayloadSize)).toStdString().data())
                << quint8(FIN | controlCode)
                << quint8(127)
                << QByteArray(wireRepresentation, 8).append(QByteArray(actualPayloadSize, 'a'))
                << QWebSocketProtocol::CC_GOING_AWAY;
    }
}

void tst_DataProcessor::nonCharacterSequence(const char *sequence)
{
    QByteArray utf8Sequence = QByteArray::fromHex(sequence);
    const char *tagName = QString("Text frame with payload containing the non-control character sequence 0x%1").arg(sequence).toStdString().data();

    QTest::newRow(tagName)
            << quint8(FIN | QWebSocketProtocol::OC_TEXT)
            << quint8(utf8Sequence.size())
            << utf8Sequence;
}

//////////////////////////////////////////////////////////////////////////////////
//// TEST DATA
//////////////////////////////////////////////////////////////////////////////////
void tst_DataProcessor::invalidHeader_data()
{
    //The first byte contain the FIN, RSV1, RSV2, RSV3 and the Opcode
    //The second byte contains the MaskFlag and the length of the frame
    QTest::addColumn<quint8>("firstByte");
    QTest::addColumn<quint8>("secondByte");
    QTest::addColumn<QByteArray>("payload");    //superfluous, but present to be able to call doTest(), which expects a payload field
    QTest::addColumn<QWebSocketProtocol::CloseCode>("expectedCloseCode");

    //invalid bit fields
    invalidField("RSV1 set", RSV1);
    invalidField("RSV2 set", RSV2);
    invalidField("RSV3 set", RSV3);
    invalidField("RSV1 and RSV2 set", RSV1 | RSV2);
    invalidField("RSV1 and RSV3 set", RSV1 | RSV3);
    invalidField("RSV2 and RSV3 set", RSV2 | RSV3);
    invalidField("RSV1, RSV2 and RSV3 set", RSV1 | RSV2 | RSV3);

    //invalid opcodes
    invalidField("Invalid OpCode 3", QWebSocketProtocol::OC_RESERVED_3);
    invalidField("Invalid OpCode 4", QWebSocketProtocol::OC_RESERVED_4);
    invalidField("Invalid OpCode 5", QWebSocketProtocol::OC_RESERVED_5);
    invalidField("Invalid OpCode 6", QWebSocketProtocol::OC_RESERVED_6);
    invalidField("Invalid OpCode 7", QWebSocketProtocol::OC_RESERVED_7);
    invalidField("Invalid OpCode B", QWebSocketProtocol::OC_RESERVED_B);
    invalidField("Invalid OpCode C", QWebSocketProtocol::OC_RESERVED_C);
    invalidField("Invalid OpCode D", QWebSocketProtocol::OC_RESERVED_D);
    invalidField("Invalid OpCode E", QWebSocketProtocol::OC_RESERVED_E);
    invalidField("Invalid OpCode F", QWebSocketProtocol::OC_RESERVED_F);

    invalidField("Continuation Frame without previous data frame", QWebSocketProtocol::OC_CONTINUE);
}

void tst_DataProcessor::invalidPayload_data()
{
    QTest::addColumn<quint8>("firstByte");
    QTest::addColumn<quint8>("secondByte");
    QTest::addColumn<QByteArray>("payload");
    QTest::addColumn<QWebSocketProtocol::CloseCode>("expectedCloseCode");

    //6.3: invalid UTF-8 sequence
    invalidUTF8("case 6.3.1",   "cebae1bdb9cf83cebcceb5eda080656469746564");

    //6.4.: fail fast tests; invalid UTF-8 in middle of string
    invalidUTF8("case 6.4.1",   "cebae1bdb9cf83cebcceb5f4908080656469746564");
    invalidUTF8("case 6.4.4",   "cebae1bdb9cf83cebcceb5eda080656469746564");

    //6.6: All prefixes of a valid UTF-8 string that contains multi-byte code points
    invalidUTF8("case 6.6.1",   "ce");
    invalidUTF8("case 6.6.3",   "cebae1");
    invalidUTF8("case 6.6.4",   "cebae1bd");
    invalidUTF8("case 6.6.6",   "cebae1bdb9cf");
    invalidUTF8("case 6.6.8",   "cebae1bdb9cf83ce");
    invalidUTF8("case 6.6.10",  "cebae1bdb9cf83cebcce");

    //6.8: First possible sequence length 5/6 (invalid codepoints)
    invalidUTF8("case 6.8.1",   "f888808080");
    invalidUTF8("case 6.8.2",   "fc8480808080");

    //6.10: Last possible sequence length 4/5/6 (invalid codepoints)
    invalidUTF8("case 6.10.1",  "f7bfbfbf");
    invalidUTF8("case 6.10.2",  "fbbfbfbfbf");
    invalidUTF8("case 6.10.3",  "fdbfbfbfbfbf");

    //5.11: boundary conditions
    invalidUTF8("case 6.11.5",  "f4908080");

    //6.12: unexpected continuation bytes
    invalidUTF8("case 6.12.1",  "80");
    invalidUTF8("case 6.12.2",  "bf");
    invalidUTF8("case 6.12.3",  "80bf");
    invalidUTF8("case 6.12.4",  "80bf80");
    invalidUTF8("case 6.12.5",  "80bf80bf");
    invalidUTF8("case 6.12.6",  "80bf80bf80");
    invalidUTF8("case 6.12.7",  "80bf80bf80bf");
    invalidUTF8("case 6.12.8",  "808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9fa0a1a2a3a4a5a6a7a8a9aaabacadaeafb0b1b2b3b4b5b6b7b8b9babbbcbdbe");

    //6.13: lonely start characters
    invalidUTF8("case 6.13.1",  "c020c120c220c320c420c520c620c720c820c920ca20cb20cc20cd20ce20cf20d020d120d220d320d420d520d620d720d820d920da20db20dc20dd20de20");
    invalidUTF8("case 6.13.2",  "e020e120e220e320e420e520e620e720e820e920ea20eb20ec20ed20ee20");
    invalidUTF8("case 6.13.3",  "f020f120f220f320f420f520f620");
    invalidUTF8("case 6.13.4",  "f820f920fa20");
    invalidUTF8("case 6.13.5",  "fc20");

    //6.14: sequences with last continuation byte missing
    invalidUTF8("case 6.14.1",  "c0");
    invalidUTF8("case 6.14.2",  "e080");
    invalidUTF8("case 6.14.3",  "f08080");
    invalidUTF8("case 6.14.4",  "f8808080");
    invalidUTF8("case 6.14.5",  "fc80808080");
    invalidUTF8("case 6.14.6",  "df");
    invalidUTF8("case 6.14.7",  "efbf");
    invalidUTF8("case 6.14.8",  "f7bfbf");
    invalidUTF8("case 6.14.9",  "fbbfbfbf");
    invalidUTF8("case 6.14.10", "fdbfbfbfbf");

    //6.15: concatenation of incomplete sequences
    invalidUTF8("case 6.15.1",  "c0e080f08080f8808080fc80808080dfefbff7bfbffbbfbfbffdbfbfbfbf");

    //6.16: impossible bytes
    invalidUTF8("case 6.16.1",  "fe");
    invalidUTF8("case 6.16.2",  "ff");
    invalidUTF8("case 6.16.3",  "fefeffff");

    //6.17: overlong ASCII characters
    invalidUTF8("case 6.17.1",  "c0af");
    invalidUTF8("case 6.17.2",  "e080af");
    invalidUTF8("case 6.17.3",  "f08080af");
    invalidUTF8("case 6.17.4",  "f8808080af");
    invalidUTF8("case 6.17.5",  "fc80808080af");

    //6.18: maximum overlong sequences
    invalidUTF8("case 6.18.1",  "c1bf");
    invalidUTF8("case 6.18.2",  "e09fbf");
    invalidUTF8("case 6.18.3",  "f08fbfbf");
    invalidUTF8("case 6.18.4",  "f887bfbfbf");
    invalidUTF8("case 6.18.5",  "fc83bfbfbfbf");

    //6.19: overlong presentation of the NUL character
    invalidUTF8("case 6.19.1",  "c080");
    invalidUTF8("case 6.19.2",  "e08080");
    invalidUTF8("case 6.19.3",  "f0808080");
    invalidUTF8("case 6.19.4",  "f880808080");
    invalidUTF8("case 6.19.5",  "fc8080808080");

    //6.20: Single UTF-16 surrogates
    invalidUTF8("case 6.20.1",  "eda080");
    invalidUTF8("case 6.20.2",  "edadbf");
    invalidUTF8("case 6.20.3",  "edae80");
    invalidUTF8("case 6.20.4",  "edafbf");
    invalidUTF8("case 6.20.5",  "edb080");
    invalidUTF8("case 6.20.6",  "edbe80");
    invalidUTF8("case 6.20.7",  "edbfbf");

    //6.21: Paired UTF-16 surrogates
    invalidUTF8("case 6.21.1",  "eda080edb080");
    invalidUTF8("case 6.21.2",  "eda080edbfbf");
    invalidUTF8("case 6.21.3",  "edadbfedb080");
    invalidUTF8("case 6.21.4",  "edadbfedbfbf");
    invalidUTF8("case 6.21.5",  "edae80edb080");
    invalidUTF8("case 6.21.6",  "edae80edbfbf");
    invalidUTF8("case 6.21.7",  "edafbfedb080");
    invalidUTF8("case 6.21.8",  "edafbfedbfbf");
}

void tst_DataProcessor::minimumSizeRequirement_data()
{
    QTest::addColumn<quint8>("firstByte");
    QTest::addColumn<quint8>("secondByte");
    QTest::addColumn<QByteArray>("payload");
    QTest::addColumn<QWebSocketProtocol::CloseCode>("expectedCloseCode");

    minimumSize16Bit(QWebSocketProtocol::OC_TEXT, 0);
    minimumSize16Bit(QWebSocketProtocol::OC_TEXT, 64);
    minimumSize16Bit(QWebSocketProtocol::OC_TEXT, 125);

    minimumSize16Bit(QWebSocketProtocol::OC_BINARY, 0);
    minimumSize16Bit(QWebSocketProtocol::OC_BINARY, 64);
    minimumSize16Bit(QWebSocketProtocol::OC_BINARY, 125);


    minimumSize64Bit(QWebSocketProtocol::OC_TEXT, 0);
    minimumSize64Bit(QWebSocketProtocol::OC_TEXT, 64);
    minimumSize64Bit(QWebSocketProtocol::OC_TEXT, 125);
    minimumSize64Bit(QWebSocketProtocol::OC_TEXT, 126);
    minimumSize64Bit(QWebSocketProtocol::OC_TEXT, 256);
    minimumSize64Bit(QWebSocketProtocol::OC_TEXT, 512);
    minimumSize64Bit(QWebSocketProtocol::OC_TEXT, 1024);
    minimumSize64Bit(QWebSocketProtocol::OC_TEXT, 2048);
    minimumSize64Bit(QWebSocketProtocol::OC_TEXT, 4096);
    minimumSize64Bit(QWebSocketProtocol::OC_TEXT, 8192);
    minimumSize64Bit(QWebSocketProtocol::OC_TEXT, 16384);
    minimumSize64Bit(QWebSocketProtocol::OC_TEXT, 32768);
    minimumSize64Bit(QWebSocketProtocol::OC_TEXT, 0xFFFFu);

    minimumSize64Bit(QWebSocketProtocol::OC_BINARY, 0);
    minimumSize64Bit(QWebSocketProtocol::OC_BINARY, 64);
    minimumSize64Bit(QWebSocketProtocol::OC_BINARY, 125);
    minimumSize64Bit(QWebSocketProtocol::OC_BINARY, 126);
    minimumSize64Bit(QWebSocketProtocol::OC_BINARY, 256);
    minimumSize64Bit(QWebSocketProtocol::OC_BINARY, 512);
    minimumSize64Bit(QWebSocketProtocol::OC_BINARY, 1024);
    minimumSize64Bit(QWebSocketProtocol::OC_BINARY, 2048);
    minimumSize64Bit(QWebSocketProtocol::OC_BINARY, 4096);
    minimumSize64Bit(QWebSocketProtocol::OC_BINARY, 8192);
    minimumSize64Bit(QWebSocketProtocol::OC_BINARY, 16384);
    minimumSize64Bit(QWebSocketProtocol::OC_BINARY, 32768);
    minimumSize64Bit(QWebSocketProtocol::OC_BINARY, 0xFFFFu);
}

void tst_DataProcessor::invalidControlFrame_data()
{
    QTest::addColumn<quint8>("firstByte");
    QTest::addColumn<quint8>("secondByte");
    QTest::addColumn<QByteArray>("payload");
    QTest::addColumn<QWebSocketProtocol::CloseCode>("expectedCloseCode");

    QTest::newRow("Close control frame with payload size 126")
            << quint8(FIN | QWebSocketProtocol::OC_CLOSE) << quint8(126) << QByteArray() << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    QTest::newRow("Ping control frame with payload size 126")
            << quint8(FIN | QWebSocketProtocol::OC_PING) << quint8(126) << QByteArray() << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    QTest::newRow("Close control frame with payload size 126")
            << quint8(FIN | QWebSocketProtocol::OC_PONG) << quint8(126) << QByteArray() << QWebSocketProtocol::CC_PROTOCOL_ERROR;

    QTest::newRow("Non-final close control frame (fragmented)")
            << quint8(QWebSocketProtocol::OC_CLOSE) << quint8(32) << QByteArray() << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    QTest::newRow("Non-final ping control frame (fragmented)")
            << quint8(QWebSocketProtocol::OC_PING) << quint8(32) << QByteArray() << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    QTest::newRow("Non-final pong control frame (fragmented)")
            << quint8(QWebSocketProtocol::OC_PONG) << quint8(32) << QByteArray() << QWebSocketProtocol::CC_PROTOCOL_ERROR;
}

void tst_DataProcessor::incompleteSizeField_data()
{
    QTest::addColumn<quint8>("firstByte");
    QTest::addColumn<quint8>("secondByte");
    QTest::addColumn<QByteArray>("payload");
    QTest::addColumn<QWebSocketProtocol::CloseCode>("expectedCloseCode");

    //////////////////////////////////////////////////////////////////////////////////////////////////
    //for a frame length value of 126, there should be 2 bytes following to form a 16-bit frame length
    //////////////////////////////////////////////////////////////////////////////////////////////////
    QTest::newRow("Text frame with payload size 126, but no data")
            << quint8(FIN | QWebSocketProtocol::OC_TEXT) << quint8(126) << QByteArray() << QWebSocketProtocol::CC_GOING_AWAY;
    QTest::newRow("Text frame with payloadsize 126, but only 1 byte following")
            << quint8(FIN | QWebSocketProtocol::OC_TEXT) << quint8(126) << QByteArray().append(quint8(1)) << QWebSocketProtocol::CC_GOING_AWAY;

    //////////////////////////////////////////////////////////////////////////////////////////////////
    //for a frame length value of 127, there should be 8 bytes following to form a 64-bit frame length
    //////////////////////////////////////////////////////////////////////////////////////////////////
    QTest::newRow("Binary frame with payload size 127, but no data")
            << quint8(FIN | QWebSocketProtocol::OC_BINARY) << quint8(127) << QByteArray() << QWebSocketProtocol::CC_GOING_AWAY;
    QTest::newRow("Binary frame with payloadsize 127, but only 1 byte following")
            << quint8(FIN | QWebSocketProtocol::OC_BINARY) << quint8(127) << QByteArray(1, quint8(1)) << QWebSocketProtocol::CC_GOING_AWAY;
    QTest::newRow("Binary frame with payload size 127, but only 2 bytes following")
            << quint8(FIN | QWebSocketProtocol::OC_BINARY) << quint8(127) << QByteArray(2, quint8(1)) << QWebSocketProtocol::CC_GOING_AWAY;
    QTest::newRow("Binary frame with payload size 127, but only 3 bytes following")
            << quint8(FIN | QWebSocketProtocol::OC_BINARY) << quint8(127) << QByteArray(3, quint8(1)) << QWebSocketProtocol::CC_GOING_AWAY;
    QTest::newRow("Binary frame with payload size 127, but only 4 bytes following")
            << quint8(FIN | QWebSocketProtocol::OC_BINARY) << quint8(127) << QByteArray(4, quint8(1)) << QWebSocketProtocol::CC_GOING_AWAY;
    QTest::newRow("Binary frame with payload size 127, but only 5 bytes following")
            << quint8(FIN | QWebSocketProtocol::OC_BINARY) << quint8(127) << QByteArray(5, quint8(1)) << QWebSocketProtocol::CC_GOING_AWAY;
    QTest::newRow("Binary frame with payload size 127, but only 6 bytes following")
            << quint8(FIN | QWebSocketProtocol::OC_BINARY) << quint8(127) << QByteArray(6, quint8(1)) << QWebSocketProtocol::CC_GOING_AWAY;
    QTest::newRow("Binary frame with payload size 127, but only 7 bytes following")
            << quint8(FIN | QWebSocketProtocol::OC_BINARY) << quint8(127) << QByteArray(7, quint8(1)) << QWebSocketProtocol::CC_GOING_AWAY;

    QTest::newRow("Text frame with payload size 127, but no data")
            << quint8(FIN | QWebSocketProtocol::OC_TEXT) << quint8(127) << QByteArray() << QWebSocketProtocol::CC_GOING_AWAY;
    QTest::newRow("Text frame with payloadsize 127, but only 1 byte following")
            << quint8(FIN | QWebSocketProtocol::OC_TEXT) << quint8(127) << QByteArray(1, quint8(1)) << QWebSocketProtocol::CC_GOING_AWAY;
    QTest::newRow("Text frame with payload size 127, but only 2 bytes following")
            << quint8(FIN | QWebSocketProtocol::OC_TEXT) << quint8(127) << QByteArray(2, quint8(1)) << QWebSocketProtocol::CC_GOING_AWAY;
    QTest::newRow("Text frame with payload size 127, but only 3 bytes following")
            << quint8(FIN | QWebSocketProtocol::OC_TEXT) << quint8(127) << QByteArray(3, quint8(1)) << QWebSocketProtocol::CC_GOING_AWAY;
    QTest::newRow("Text frame with payload size 127, but only 4 bytes following")
            << quint8(FIN | QWebSocketProtocol::OC_TEXT) << quint8(127) << QByteArray(4, quint8(1)) << QWebSocketProtocol::CC_GOING_AWAY;
    QTest::newRow("Text frame with payload size 127, but only 5 bytes following")
            << quint8(FIN | QWebSocketProtocol::OC_TEXT) << quint8(127) << QByteArray(5, quint8(1)) << QWebSocketProtocol::CC_GOING_AWAY;
    QTest::newRow("Text frame with payload size 127, but only 6 bytes following")
            << quint8(FIN | QWebSocketProtocol::OC_TEXT) << quint8(127) << QByteArray(6, quint8(1)) << QWebSocketProtocol::CC_GOING_AWAY;
    QTest::newRow("Text frame with payload size 127, but only 7 bytes following")
            << quint8(FIN | QWebSocketProtocol::OC_TEXT) << quint8(127) << QByteArray(7, quint8(1)) << QWebSocketProtocol::CC_GOING_AWAY;
}

void tst_DataProcessor::nonCharacterCodes_data()
{
    QTest::addColumn<quint8>("firstByte");
    QTest::addColumn<quint8>("secondByte");
    QTest::addColumn<QByteArray>("payload");

    nonCharacterSequence("efbfbe");
    nonCharacterSequence("efbfbf");
    nonCharacterSequence("f09fbfbe");
    nonCharacterSequence("f09fbfbf");
    nonCharacterSequence("f0afbfbe");
    nonCharacterSequence("f0afbfbf");
    nonCharacterSequence("f0bfbfbe");
    nonCharacterSequence("f0bfbfbf");
    nonCharacterSequence("f18fbfbe");
    nonCharacterSequence("f18fbfbf");
    nonCharacterSequence("f19fbfbe");
    nonCharacterSequence("f19fbfbf");
    nonCharacterSequence("f1afbfbe");
    nonCharacterSequence("f1afbfbf");
    nonCharacterSequence("f1bfbfbe");
    nonCharacterSequence("f1bfbfbf");
    nonCharacterSequence("f28fbfbe");
    nonCharacterSequence("f28fbfbf");
    nonCharacterSequence("f29fbfbe");
    nonCharacterSequence("f29fbfbf");
    nonCharacterSequence("f2afbfbe");
    nonCharacterSequence("f2afbfbf");
    nonCharacterSequence("f2bfbfbe");
    nonCharacterSequence("f2bfbfbf");
    nonCharacterSequence("f38fbfbe");
    nonCharacterSequence("f38fbfbf");
    nonCharacterSequence("f39fbfbe");
    nonCharacterSequence("f39fbfbf");
    nonCharacterSequence("f3afbfbe");
    nonCharacterSequence("f3afbfbf");
    nonCharacterSequence("f3bfbfbe");
    nonCharacterSequence("f3bfbfbf");
    nonCharacterSequence("f48fbfbe");
    nonCharacterSequence("f48fbfbf");
}

void tst_DataProcessor::incompletePayload_data()
{
    QTest::addColumn<quint8>("firstByte");
    QTest::addColumn<quint8>("secondByte");
    QTest::addColumn<QByteArray>("payload");
    QTest::addColumn<QWebSocketProtocol::CloseCode>("expectedCloseCode");

    incompleteFrame(QWebSocketProtocol::OC_TEXT, 125, 0);
    incompleteFrame(QWebSocketProtocol::OC_TEXT, 64, 32);
    incompleteFrame(QWebSocketProtocol::OC_TEXT, 256, 32);
    incompleteFrame(QWebSocketProtocol::OC_TEXT, 128000, 32);
    incompleteFrame(QWebSocketProtocol::OC_BINARY, 125, 0);
    incompleteFrame(QWebSocketProtocol::OC_BINARY, 64, 32);
    incompleteFrame(QWebSocketProtocol::OC_BINARY, 256, 32);
    incompleteFrame(QWebSocketProtocol::OC_BINARY, 128000, 32);

    incompleteFrame(QWebSocketProtocol::OC_CLOSE, 64, 32);
    incompleteFrame(QWebSocketProtocol::OC_PING, 64, 32);
    incompleteFrame(QWebSocketProtocol::OC_PONG, 64, 32);
}

void tst_DataProcessor::frameTooBig_data()
{
    QTest::addColumn<quint8>("firstByte");
    QTest::addColumn<quint8>("secondByte");
    QTest::addColumn<QByteArray>("payload");
    QTest::addColumn<QWebSocketProtocol::CloseCode>("expectedCloseCode");

    quint64 swapped64 = 0;
    const char *wireRepresentation = 0;

    //only data frames can be too big
    //control frames have explicit checking on a maximum payload size of 125

    //TODO: get maximum framesize from the frame somehow
    //the maximum framesize is currently defined as a constant in the dataprocessor_p.cpp file
    //and is set to MAX_INT; changing that value will make this test fail (which is not good)
    swapped64 = qToBigEndian<quint64>(quint64(INT_MAX + 1));
    wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped64));
    QTest::newRow("Text frame with payload size > INT_MAX")
            << quint8(FIN | QWebSocketProtocol::OC_TEXT)
            << quint8(127)
            << QByteArray(wireRepresentation, 8).append(QByteArray(32, 'a'))
            << QWebSocketProtocol::CC_TOO_MUCH_DATA;

    swapped64 = qToBigEndian<quint64>(quint64(INT_MAX + 1));
    wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped64));
    QTest::newRow("Binary frame with payload size > INT_MAX")
            << quint8(FIN | QWebSocketProtocol::OC_BINARY)
            << quint8(127)
            << QByteArray(wireRepresentation, 8).append(QByteArray(32, 'a'))
            << QWebSocketProtocol::CC_TOO_MUCH_DATA;

}

DECLARE_TEST(tst_DataProcessor)

#include "tst_dataprocessor.moc"
