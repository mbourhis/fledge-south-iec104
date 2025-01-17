#include <config_category.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iec104.h>
#include <plugin_api.h>
#include <string.h>
#include "cs104_slave.h"

#include <boost/thread.hpp>
#include <utility>
#include <vector>

using namespace std;

#define TEST_PORT 2404

static string protocol_config = QUOTE({
        "protocol_stack" : {
            "name" : "iec104client",
            "version" : "1.0",
            "transport_layer" : {
                "redundancy_groups" : [
                    { 
                        "connections" : [
                            {     
                                "srv_ip" : "127.0.0.1",        
                                "port" : 2404          
                            },    
                            {
                                "srv_ip" : "127.0.0.1", 
                                "port" : 2404
                            }
                        ],
                        "rg_name" : "red-group1",  
                        "tls" : false,
                        "k_value" : 12,  
                        "w_value" : 8,
                        "t0_timeout" : 10,                 
                        "t1_timeout" : 15,                 
                        "t2_timeout" : 10,                 
                        "t3_timeout" : 20    
                    }
                ]                  
            },                
            "application_layer" : {                
                "orig_addr" : 10, 
                "ca_asdu_size" : 2,                
                "ioaddr_size" : 3,                             
                "asdu_size" : 0, 
                "gi_time" : 60,  
                "gi_cycle" : 0,                
                "gi_all_ca" : true,                             
                "utc_time" : false,                
                "cmd_with_timetag" : false,              
                "cmd_parallel" : 0,                              
                "time_sync" : 100                 
            }                 
        }                     
    });

static string protocol_config2 = QUOTE({
        "protocol_stack" : {
            "name" : "iec104client",
            "version" : "1.0",
            "transport_layer" : {
                "redundancy_groups" : [
                    { 
                        "connections" : [
                            {     
                                "srv_ip" : "127.0.0.1",        
                                "port" : 2404          
                            },    
                            {
                                "srv_ip" : "127.0.0.1", 
                                "port" : 2404
                            }
                        ],
                        "rg_name" : "red-group1",  
                        "tls" : false,
                        "k_value" : 12,  
                        "w_value" : 8,
                        "t0_timeout" : 10,                 
                        "t1_timeout" : 15,                 
                        "t2_timeout" : 10,                 
                        "t3_timeout" : 20    
                    }
                ]                  
            },                
            "application_layer" : {                
                "orig_addr" : 10, 
                "ca_asdu_size" : 2,                
                "ioaddr_size" : 3,                          
                "asdu_size" : 0, 
                "gi_time" : 60,  
                "gi_cycle" : 0,                
                "gi_all_ca" : false,                        
                "utc_time" : false,                
                "cmd_with_timetag" : false,              
                "cmd_parallel" : 0,                           
                "time_sync" : 100                
            }                 
        }                     
    });

static string protocol_config3 = QUOTE({
        "protocol_stack" : {
            "name" : "iec104client",
            "version" : "1.0",
            "transport_layer" : {
                "redundancy_groups" : [
                    { 
                        "connections" : [
                            {     
                                "srv_ip" : "127.0.0.1",        
                                "port" : 2404          
                            }
                        ],
                        "rg_name" : "red-group1",  
                        "tls" : false,
                        "k_value" : 12,  
                        "w_value" : 8,
                        "t0_timeout" : 10,                 
                        "t1_timeout" : 15,                 
                        "t2_timeout" : 10,                 
                        "t3_timeout" : 20    
                    }
                ]                  
            },                
            "application_layer" : {                
                "orig_addr" : 10, 
                "ca_asdu_size" : 2,                
                "ioaddr_size" : 3,                          
                "asdu_size" : 0, 
                "gi_time" : 1,  
                "gi_cycle" : 1,                
                "gi_all_ca" : false,                            
                "utc_time" : false,                
                "cmd_with_timetag" : false,              
                "cmd_parallel" : 0,                              
                "time_sync" : 100                
            }                 
        }                     
    });


static string exchanged_data = QUOTE({
        "exchanged_data": {
            "name" : "iec104client",        
            "version" : "1.0",               
            "datapoints" : [          
                {
                    "label":"TM-1",
                    "protocols":[
                       {
                          "name":"iec104",
                          "address":"41025-4202832",
                          "typeid":"M_ME_NA_1"
                       }
                    ]
                },
                {
                    "label":"TM-2",
                    "protocols":[
                       {
                          "name":"iec104",
                          "address":"41025-4202852",
                          "typeid":"M_ME_NA_1"
                       }
                    ]
                },
                {
                    "label":"TS-1",
                    "protocols":[
                       {
                          "name":"iec104",
                          "address":"41025-4206948",
                          "typeid":"M_SP_TB_1"
                       }
                    ]
                },
                {
                    "label":"C-1",
                    "protocols":[
                       {
                          "name":"iec104",
                          "address":"41025-2000",
                          "typeid":"C_SC_NA_1"
                       }
                    ]
                },                          
                {
                    "label":"C-2",
                    "protocols":[
                       {
                          "name":"iec104",
                          "address":"41025-2001",
                          "typeid":"C_SC_TA_1"
                       }
                    ]
                },                            
                {
                    "label":"C-3",
                    "protocols":[
                       {
                          "name":"iec104",
                          "address":"41025-2002",
                          "typeid":"C_DC_NA_1"
                       }
                    ]
                },
                {
                    "label":"TM-B-1",
                    "protocols":[
                       {
                          "name":"iec104",
                          "address":"41026-2001",
                          "typeid":"M_ME_NA_1"
                       }
                    ]
                }                   
            ]
        }
    });


// PLUGIN DEFAULT TLS CONF
static string tls_config =  QUOTE({       
        "tls_conf:" : {
            "private_key" : "server-key.pem",
            "server_cert" : "server.cer",
            "ca_cert" : "root.cer"
        }         
    });

class IEC104TestComp : public IEC104
{
public:
    IEC104TestComp() : IEC104()
    {
    }
};

class InterrogationTest : public testing::Test
{
protected:

    void SetUp()
    {
        clockSyncHandlerCalled = 0;

        iec104 = new IEC104TestComp();

        iec104->registerIngest(NULL, ingestCallback);
    }

    void TearDown()
    {
        iec104->stop();

        delete iec104;
    }

    void startIEC104() { iec104->start(); }

    static bool hasChild(Datapoint& dp, std::string childLabel)
    {
        DatapointValue& dpv = dp.getData();

        auto dps = dpv.getDpVec();

        for (auto sdp : *dps) {
            if (sdp->getName() == childLabel) {
                return true;
            }
        }

        return false;
    }

    static Datapoint* getChild(Datapoint& dp, std::string childLabel)
    {
        DatapointValue& dpv = dp.getData();

        auto dps = dpv.getDpVec();

        for (Datapoint* childDp : *dps) {
            if (childDp->getName() == childLabel) {
                return childDp;
            }
        }

        return nullptr;
    }

    static int64_t getIntValue(Datapoint* dp)
    {
        DatapointValue dpValue = dp->getData();
        return dpValue.toInt();
    }

    static std::string getStrValue(Datapoint* dp)
    {
        return dp->getData().toStringValue();
    }

    static bool hasObject(Reading& reading, std::string label)
    {
        std::vector<Datapoint*> dataPoints = reading.getReadingData();

        for (Datapoint* dp : dataPoints) 
        {
            if (dp->getName() == label) {
                return true;
            }
        }

        return false;
    }

    static Datapoint* getObject(Reading& reading, std::string label)
    {
        std::vector<Datapoint*> dataPoints = reading.getReadingData();

        for (Datapoint* dp : dataPoints) 
        {
            if (dp->getName() == label) {
                return dp;
            }
        }

        return nullptr;
    }

    static void ingestCallback(void* parameter, Reading reading)
    {
        printf("ingestCallback called -> asset: (%s)\n", reading.getAssetName().c_str());

        std::vector<Datapoint*> dataPoints = reading.getReadingData();

        // for (Datapoint* sdp : dataPoints) {
        //     printf("name: %s value: %s\n", sdp->getName().c_str(), sdp->getData().toString().c_str());
        // }
        storedReading = new Reading(reading);

        ingestCallbackCalled++;
    }

    static bool clockSynchronizationHandler(void* parameter, IMasterConnection connection, CS101_ASDU asdu, CP56Time2a newTime)
    {
        InterrogationTest* self = (InterrogationTest*)parameter;

        char addrBuf[100];

        IMasterConnection_getPeerAddress(connection, addrBuf, 100);

        printf("Clock sync called from %s\n", addrBuf);

        self->clockSyncHandlerCalled++;

        return true;
    }

    static bool asduHandler (void* parameter, IMasterConnection connection, CS101_ASDU asdu)
    {
        InterrogationTest* self = (InterrogationTest*)parameter;

        printf("asduHandler: type: %i\n", CS101_ASDU_getTypeID(asdu));

        lastConnection = NULL;
        lastOA = CS101_ASDU_getOA(asdu);

        int ca = CS101_ASDU_getCA(asdu);

        InformationObject io = CS101_ASDU_getElement(asdu, 0);

        int ioa = InformationObject_getObjectAddress(io);

        if (CS101_ASDU_getTypeID(asdu) == C_SC_NA_1) {
            printf("  C_SC_NA_1 (single-command)\n");

            if (ca == 41025 && ioa == 2000) {
                IMasterConnection_sendACT_CON(connection, asdu, false);
                lastConnection = connection;
            }

            
        }
        else if (CS101_ASDU_getTypeID(asdu) == C_SC_TA_1) {
            printf("  C_SC_TA_1 (single-command w/timetag)\n");

            if (ca == 41025 && ioa == 2001) {
                IMasterConnection_sendACT_CON(connection, asdu, false);
                lastConnection = connection;
            }
        }
        else if (CS101_ASDU_getTypeID(asdu) == C_DC_NA_1) {
            printf("  C_DC_NA_1 (double-command)\n");

            if (ca == 41025 && ioa == 2002) {
                IMasterConnection_sendACT_CON(connection, asdu, false);
                lastConnection = connection;
            }
        }

        asduHandlerCalled++;

        return true;
    }

    static bool interrogationHandler(void* parameter, IMasterConnection connection, CS101_ASDU asdu, uint8_t qoi)
    {
        InterrogationTest* self = (InterrogationTest*)parameter;

        interrogationRequestsReceived++;

        printf("Received interrogation for group %i\n", qoi);

        if (qoi == 20) { /* only handle station interrogation */

            CS101_AppLayerParameters alParams = IMasterConnection_getApplicationLayerParameters(connection);

            IMasterConnection_sendACT_CON(connection, asdu, false);

            /* The CS101 specification only allows information objects without timestamp in GI responses */

            CS101_ASDU newAsdu = CS101_ASDU_create(alParams, false, CS101_COT_INTERROGATED_BY_STATION,
                    0, 1, false, false);

            InformationObject io = (InformationObject) MeasuredValueScaled_create(NULL, 100, -1, IEC60870_QUALITY_GOOD);

            CS101_ASDU_addInformationObject(newAsdu, io);

            CS101_ASDU_addInformationObject(newAsdu, (InformationObject)
                MeasuredValueScaled_create((MeasuredValueScaled) io, 101, 23, IEC60870_QUALITY_GOOD));

            CS101_ASDU_addInformationObject(newAsdu, (InformationObject)
                MeasuredValueScaled_create((MeasuredValueScaled) io, 102, 2300, IEC60870_QUALITY_GOOD));

            InformationObject_destroy(io);

            IMasterConnection_sendASDU(connection, newAsdu);

            CS101_ASDU_destroy(newAsdu);

            newAsdu = CS101_ASDU_create(alParams, false, CS101_COT_INTERROGATED_BY_STATION,
                        0, 1, false, false);

            io = (InformationObject) SinglePointInformation_create(NULL, 104, true, IEC60870_QUALITY_GOOD);

            CS101_ASDU_addInformationObject(newAsdu, io);

            CS101_ASDU_addInformationObject(newAsdu, (InformationObject)
                SinglePointInformation_create((SinglePointInformation) io, 105, false, IEC60870_QUALITY_GOOD));

            InformationObject_destroy(io);

            IMasterConnection_sendASDU(connection, newAsdu);

            CS101_ASDU_destroy(newAsdu);

            newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_INTERROGATED_BY_STATION,
                    0, 1, false, false);

            CS101_ASDU_addInformationObject(newAsdu, io = (InformationObject) SinglePointInformation_create(NULL, 300, true, IEC60870_QUALITY_GOOD));
            CS101_ASDU_addInformationObject(newAsdu, (InformationObject) SinglePointInformation_create((SinglePointInformation) io, 301, false, IEC60870_QUALITY_GOOD));
            CS101_ASDU_addInformationObject(newAsdu, (InformationObject) SinglePointInformation_create((SinglePointInformation) io, 302, true, IEC60870_QUALITY_GOOD));
            CS101_ASDU_addInformationObject(newAsdu, (InformationObject) SinglePointInformation_create((SinglePointInformation) io, 303, false, IEC60870_QUALITY_GOOD));
            CS101_ASDU_addInformationObject(newAsdu, (InformationObject) SinglePointInformation_create((SinglePointInformation) io, 304, true, IEC60870_QUALITY_GOOD));
            CS101_ASDU_addInformationObject(newAsdu, (InformationObject) SinglePointInformation_create((SinglePointInformation) io, 305, false, IEC60870_QUALITY_GOOD));
            CS101_ASDU_addInformationObject(newAsdu, (InformationObject) SinglePointInformation_create((SinglePointInformation) io, 306, true, IEC60870_QUALITY_GOOD));
            CS101_ASDU_addInformationObject(newAsdu, (InformationObject) SinglePointInformation_create((SinglePointInformation) io, 307, false, IEC60870_QUALITY_GOOD));

            InformationObject_destroy(io);

            IMasterConnection_sendASDU(connection, newAsdu);

            CS101_ASDU_destroy(newAsdu);

            newAsdu = CS101_ASDU_create(alParams, false, CS101_COT_INTERROGATED_BY_STATION,
                            0, 1, false, false);

            io = (InformationObject) BitString32_create(NULL, 500, 0xaaaa);

            CS101_ASDU_addInformationObject(newAsdu, io);

            InformationObject_destroy(io);

            IMasterConnection_sendASDU(connection, newAsdu);

            CS101_ASDU_destroy(newAsdu);

            IMasterConnection_sendACT_TERM(connection, asdu);
        }
        else {
            IMasterConnection_sendACT_CON(connection, asdu, true);
        }

        return true;
    }

    static bool interrogationHandler_No_ACT_CON(void* parameter, IMasterConnection connection, CS101_ASDU asdu, uint8_t qoi)
    {
        InterrogationTest* self = (InterrogationTest*)parameter;

        interrogationRequestsReceived++;

        printf("Received interrogation for group %i\n", qoi);

        return true;
    }

    static boost::thread thread_;
    IEC104TestComp* iec104;
    static int ingestCallbackCalled;
    static Reading* storedReading;
    int clockSyncHandlerCalled = 0;
    static int asduHandlerCalled;
    static IMasterConnection lastConnection;
    static int lastOA;
    static int interrogationRequestsReceived;
};

boost::thread InterrogationTest::thread_;
int InterrogationTest::ingestCallbackCalled;
Reading* InterrogationTest::storedReading;
int InterrogationTest::asduHandlerCalled;
int InterrogationTest::interrogationRequestsReceived;
IMasterConnection InterrogationTest::lastConnection;
int InterrogationTest::lastOA;


TEST_F(InterrogationTest, IEC104Client_startupProcedureSeparateRequestForEachCA)
{
    iec104->setJsonConfig(protocol_config, exchanged_data, tls_config);

    asduHandlerCalled = 0;
    interrogationRequestsReceived = 0;
    clockSyncHandlerCalled = 0;
    lastConnection = NULL;
    ingestCallbackCalled = 0;

    CS104_Slave slave = CS104_Slave_create(10, 10);

    CS104_Slave_setLocalPort(slave, TEST_PORT);

    CS104_Slave_setClockSyncHandler(slave, clockSynchronizationHandler, this);
    CS104_Slave_setASDUHandler(slave, asduHandler, this);
    CS104_Slave_setInterrogationHandler(slave, interrogationHandler, this);

    CS104_Slave_start(slave);

    CS101_AppLayerParameters alParams = CS104_Slave_getAppLayerParameters(slave);

    startIEC104();

    Thread_sleep(500);

    ASSERT_EQ(1, clockSyncHandlerCalled);
    ASSERT_EQ(0, asduHandlerCalled);

    Thread_sleep(2500);

    ASSERT_EQ(2, interrogationRequestsReceived);

    CS104_Slave_stop(slave);

    CS104_Slave_destroy(slave);

    Thread_sleep(500);
}

TEST_F(InterrogationTest, IEC104Client_startupProcedureBroadcastCA)
{
    iec104->setJsonConfig(protocol_config2, exchanged_data, tls_config);

    asduHandlerCalled = 0;
    interrogationRequestsReceived = 0;
    clockSyncHandlerCalled = 0;
    lastConnection = NULL;
    ingestCallbackCalled = 0;

    CS104_Slave slave = CS104_Slave_create(10, 10);

    CS104_Slave_setLocalPort(slave, TEST_PORT);

    CS104_Slave_setClockSyncHandler(slave, clockSynchronizationHandler, this);
    CS104_Slave_setASDUHandler(slave, asduHandler, this);
    CS104_Slave_setInterrogationHandler(slave, interrogationHandler, this);

    CS104_Slave_start(slave);

    CS101_AppLayerParameters alParams = CS104_Slave_getAppLayerParameters(slave);

    startIEC104();

    Thread_sleep(500);

    ASSERT_EQ(1, clockSyncHandlerCalled);
    ASSERT_EQ(0, asduHandlerCalled);

    Thread_sleep(2500);

    ASSERT_EQ(1, interrogationRequestsReceived);

    CS104_Slave_stop(slave);

    CS104_Slave_destroy(slave);

    Thread_sleep(500);
}

TEST_F(InterrogationTest, IEC104Client_GIcycleOneSecond)
{
    iec104->setJsonConfig(protocol_config3, exchanged_data, tls_config);

    asduHandlerCalled = 0;
    interrogationRequestsReceived = 0;
    clockSyncHandlerCalled = 0;
    lastConnection = NULL;
    ingestCallbackCalled = 0;

    CS104_Slave slave = CS104_Slave_create(10, 10);

    CS104_Slave_setLocalPort(slave, TEST_PORT);

    CS104_Slave_setClockSyncHandler(slave, clockSynchronizationHandler, this);
    CS104_Slave_setASDUHandler(slave, asduHandler, this);
    CS104_Slave_setInterrogationHandler(slave, interrogationHandler, this);

    CS104_Slave_start(slave);

    CS101_AppLayerParameters alParams = CS104_Slave_getAppLayerParameters(slave);

    startIEC104();

    Thread_sleep(500);

    ASSERT_EQ(1, clockSyncHandlerCalled);
    ASSERT_EQ(0, asduHandlerCalled);
    ASSERT_EQ(1, interrogationRequestsReceived);

    Thread_sleep(2000);

    ASSERT_EQ(3, interrogationRequestsReceived);

    CS104_Slave_stop(slave);

    CS104_Slave_destroy(slave);

    Thread_sleep(500);
}


TEST_F(InterrogationTest, IEC104Client_GIcycleOneSecondNoACT_CON)
{
    iec104->setJsonConfig(protocol_config3, exchanged_data, tls_config);

    asduHandlerCalled = 0;
    interrogationRequestsReceived = 0;
    clockSyncHandlerCalled = 0;
    lastConnection = NULL;
    ingestCallbackCalled = 0;

    CS104_Slave slave = CS104_Slave_create(10, 10);

    CS104_Slave_setLocalPort(slave, TEST_PORT);

    CS104_Slave_setClockSyncHandler(slave, clockSynchronizationHandler, this);
    CS104_Slave_setASDUHandler(slave, asduHandler, this);
    CS104_Slave_setInterrogationHandler(slave, interrogationHandler_No_ACT_CON, this);

    CS104_Slave_start(slave);

    CS101_AppLayerParameters alParams = CS104_Slave_getAppLayerParameters(slave);

    startIEC104();

    Thread_sleep(500);

    ASSERT_EQ(1, clockSyncHandlerCalled);
    ASSERT_EQ(0, asduHandlerCalled);
    ASSERT_EQ(1, interrogationRequestsReceived);

    Thread_sleep(2000);

    ASSERT_EQ(2, interrogationRequestsReceived);

    CS104_Slave_stop(slave);

    CS104_Slave_destroy(slave);

    Thread_sleep(500);
}

TEST_F(InterrogationTest, InterrogationRequestAfter_M_EI_NA_1)
{
    iec104->setJsonConfig(protocol_config2, exchanged_data, tls_config);

    asduHandlerCalled = 0;
    interrogationRequestsReceived = 0;
    clockSyncHandlerCalled = 0;
    lastConnection = NULL;
    ingestCallbackCalled = 0;

    CS104_Slave slave = CS104_Slave_create(10, 10);

    CS104_Slave_setLocalPort(slave, TEST_PORT);

    CS104_Slave_setClockSyncHandler(slave, clockSynchronizationHandler, this);
    CS104_Slave_setASDUHandler(slave, asduHandler, this);
    CS104_Slave_setInterrogationHandler(slave, interrogationHandler, this);

    CS104_Slave_start(slave);

    CS101_AppLayerParameters alParams = CS104_Slave_getAppLayerParameters(slave);

    startIEC104();

    Thread_sleep(500);

    ASSERT_EQ(1, clockSyncHandlerCalled);
    ASSERT_EQ(0, asduHandlerCalled);

    Thread_sleep(1000);

    ASSERT_EQ(1, interrogationRequestsReceived);

    CS101_ASDU asdu = CS101_ASDU_create(alParams, false, CS101_COT_INITIALIZED,
                    0, 1, false, false);

    InformationObject io = (InformationObject)EndOfInitialization_create(NULL, 0);

    CS101_ASDU_addInformationObject(asdu, io);

    InformationObject_destroy(io);

    CS104_Slave_enqueueASDU(slave, asdu);

    CS101_ASDU_destroy(asdu);

    Thread_sleep(1000);

    ASSERT_EQ(2, interrogationRequestsReceived);

    CS104_Slave_stop(slave);

    CS104_Slave_destroy(slave);

    Thread_sleep(500);
}

