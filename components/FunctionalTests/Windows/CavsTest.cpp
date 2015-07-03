/*
********************************************************************************
*                              INTEL CONFIDENTIAL
*   Copyright(C) 2015 Intel Corporation. All Rights Reserved.
*   The source code contained  or  described herein and all documents related to
*   the source code ("Material") are owned by Intel Corporation or its suppliers
*   or licensors.  Title to the  Material remains with  Intel Corporation or its
*   suppliers and licensors. The Material contains trade secrets and proprietary
*   and  confidential  information of  Intel or its suppliers and licensors. The
*   Material  is  protected  by  worldwide  copyright  and trade secret laws and
*   treaty  provisions. No part of the Material may be used, copied, reproduced,
*   modified, published, uploaded, posted, transmitted, distributed or disclosed
*   in any way without Intel's prior express written permission.
*   No license  under any  patent, copyright, trade secret or other intellectual
*   property right is granted to or conferred upon you by disclosure or delivery
*   of the Materials,  either expressly, by implication, inducement, estoppel or
*   otherwise.  Any  license  under  such  intellectual property  rights must be
*   express and approved by Intel in writing.
*
********************************************************************************
*/

#include "Core/DebugAgent.hpp"
#include "TestCommon/HttpClientSimulator.hpp"
#include "cAVS/Windows/DeviceInjectionDriverFactory.hpp"
#include "cAVS/Windows/MockedDevice.hpp"
#include "cAVS/Windows/MockedDeviceCommands.hpp"
#include "catch.hpp"

using namespace debug_agent::core;
using namespace debug_agent::cavs;
using namespace debug_agent::test_common;

static const std::size_t ModuleUIDSize = 4;
using ModuleUID = uint32_t[ModuleUIDSize];

/** Defining some module uuids... */
const ModuleUID Module0UID = { 1, 2, 3, 4 };
const ModuleUID Module1UID = { 11, 12, 13, 14 };

/** Helper function to set a module entry */
void setModuleEntry(dsp_fw::ModuleEntry &entry, const std::string &name,
    const ModuleUID &uuid)
{
    /* Setting name */
    assert(name.size() <= sizeof(entry.name));
    for (std::size_t i = 0; i < sizeof(entry.name); i++) {
        if (i < name.size()) {
            entry.name[i] = name[i];
        }
        else {
            /* Filling buffer with 0 after name end */
            entry.name[i] = 0;
        }
    }

    /* Setting GUID*/
    for (std::size_t i = 0; i < ModuleUIDSize; i++) {
        entry.uuid[i] = uuid[i];
    }
}


TEST_CASE("DebugAgent: module entries")
{
    /* Creating the mocked device */
    std::unique_ptr<windows::MockedDevice> device(new windows::MockedDevice());

    /* Setting the test vector
     * ----------------------- */

    windows::MockedDeviceCommands commands(*device);

    /* Creating 2 module entries */
    std::vector<dsp_fw::ModuleEntry> returnedEntries(2);
    setModuleEntry(returnedEntries[0], "module_0", Module0UID);
    setModuleEntry(returnedEntries[1], "module_1", Module1UID);

    /* Adding the "get module entries" command to the test vector */
    commands.addGetModuleEntriesCommand(
        STATUS_SUCCESS,
        dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS,
        returnedEntries);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(std::move(device));

    /* Creating and starting the debug agent */
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    /* The expected content is an html table describing the mapping uuid<-> module_id */
    std::string expectedContent(
        "<p>Module type count: 2</p>"
        "<table border='1'><tr><td>name</td><td>uuid</td><td>module id</td></tr>"
        "<tr><td>module_0</td><td>00000001000000020000000300000004</td><td>0</td></tr>"
        "<tr><td>module_1</td><td>0000000b0000000c0000000d0000000e</td><td>1</td></tr>"
        "</table>");

    /* Doing the request on the "/cAVS/module/entries" URI */
    client.request(
        "/cAVS/module/entries",
        HttpClientSimulator::Verb::Get,
        "",
        HttpClientSimulator::Status::Ok,
        "text/html",
        expectedContent
        );
}