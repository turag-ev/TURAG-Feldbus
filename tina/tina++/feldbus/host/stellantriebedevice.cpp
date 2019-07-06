/* We are not doing anything special with float values, but we need to check
 * whether a passed value (factor) is equal to a special value.
 * See TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE */
#pragma GCC diagnostic ignored "-Wfloat-equal"

#include <tina++/debug.h>
#include "stellantriebedevice.h"

namespace TURAG {
namespace Feldbus {

struct GetCommandInfo {
    uint8_t key;
    uint8_t cmd0;
    uint8_t cmd1;
    uint8_t cmd2;
} _packed;

struct GetCommandInfoResponse {
    Aktor::Command_t::WriteAccess writeAccess;
    Aktor::Command_t::CommandLength length;
    float factor;
} _packed;

bool StellantriebeDevice::init() {
    //query size of command set
    Request<GetCommandInfo> req;
    req.data.key =  TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET_COMMANDSET_SIZE;
    req.data.cmd0 = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET_COMMANDSET_SIZE;
    req.data.cmd1 = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET_COMMANDSET_SIZE;
    req.data.cmd2 = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET_COMMANDSET_SIZE;

    Response<uint8_t> command_set_size;
    if(!transceive(req, &command_set_size)) {
        turag_errorf("%s: Unable to query command set size.", name());
        return false;
    }

    turag_debugf("%s: Command set size is: %u.", name(), command_set_size.data);

    Request<GetCommandInfo> name_length_req;
    Response<uint8_t> name_length_resp;
    name_length_req.data.cmd0 = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET_NAME_LENGTH;
    name_length_req.data.cmd1 = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET_NAME_LENGTH;
    name_length_req.data.cmd2 = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET_NAME_LENGTH;

    uint8_t name_req[addressLength() + 4 + 1];
    name_req[2] = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET_NAME;
    name_req[3] = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET_NAME;
    name_req[4] = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET_NAME;
    //buffer for fieldbus package containing command name
    char name_resp[255+addressLength()+1] = { 0 };

    Request<GetCommandInfo> cmd_info_req;
    cmd_info_req.data.cmd0 = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET;
    cmd_info_req.data.cmd1 = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET;
    cmd_info_req.data.cmd2 = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET;
    Response<GetCommandInfoResponse> cmd_info_resp;

    //try to find all commands in the command set
    for(CommandBase* cmd = first_command_; cmd != nullptr; cmd = cmd->next()) {
        bool found = false;
        //valid keys start at 1
        for(uint8_t i = 1; i <= command_set_size.data; i++) {
            name_length_req.data.key = i;
            name_req[1] = i;
            cmd_info_req.data.key = i;

            //query length of command name
            if(!transceive(name_length_req, &name_length_resp)) {
                turag_errorf("%s: Failed to query name length of key %u.", name(), i);
                return false;
            }
            if(name_length_resp.data == 0)
                continue;

            //query command name
            if(!transceive(name_req, sizeof(name_req),
                           reinterpret_cast<uint8_t*>(name_resp), name_length_resp.data + addressLength() + 1)) {
                turag_errorf("%s: Failed to query name of key %u.", name(), i);
                return false;
            }

            //add null-termination and check name (it is offset by addressLength())
            name_resp[addressLength() + name_length_resp.data] = 0;
            if(strcmp(&name_resp[addressLength()], cmd->name()))
                continue; //no match

            //names match, query properties
            if(!transceive(cmd_info_req, &cmd_info_resp)) {
                turag_errorf("%s: Failed to query command info for %s (key %u).", name(), cmd->name(), i);
                return false;
            }
            //check properties
            if(cmd_info_resp.data.length != cmd->length()) {
                turag_errorf("%s: Command \"%s\" length mismatched, device reports %u, required %u.",
                             name(), cmd->name(),
                             unsigned(cmd_info_resp.data.length), unsigned(cmd->length()));
                return false;
            }
            if(cmd_info_resp.data.writeAccess != cmd->access()) {
                turag_errorf("%s: Command \"%s\" write access mismatched, device reports %u, required %u.",
                             name(), cmd->name(),
                             unsigned(cmd_info_resp.data.writeAccess), unsigned(cmd->access()));
                return false;
            }
            bool dev_ctrl = cmd_info_resp.data.factor == TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE;
            bool cmd_ctrl = cmd->type() == CommandType::control;
            if(dev_ctrl != cmd_ctrl) {
                turag_errorf("%s: Command \"%s\" type mismatched, device reports %s, required %s.",
                             name(), cmd->name(),
                             dev_ctrl?"control":"real",
                             cmd_ctrl?"control":"real");
                return false;
            }
            //all checks successful
            cmd->setKey(i);
            cmd->setConversionFactor(cmd_info_resp.data.factor);
            found = true;
            break;
        }
        if(!found) {
            turag_errorf("%s: Command %s not found in device command set.", name(), cmd->name());
            return false;
        }
    }
    return true;
}

} // namespace IM
} // namespace TURAG
