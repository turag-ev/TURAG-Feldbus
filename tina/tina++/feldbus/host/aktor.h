/**
 *  @brief		TURAG feldbus actor base class
 *  @file		aktor.h
 *  @date		22.03.2013
 *  @author		Martin Oemus
 *
 */


#ifndef TURAGFELDBUSAKTOR_H_
#define TURAGFELDBUSAKTOR_H_

#include "device.h"
#include <tina++/tina.h>
#include <tina/feldbus/protocol/turag_feldbus_fuer_stellantriebe.h>
#include <tina/tina.h>

#ifdef TURAG_FELDBUS_AKTOR_STRUCTURED_OUTPUT_AVAILABLE
# include <vector>
#endif

namespace TURAG {
namespace Feldbus {

enum class AktorCommandWriteAccess : uint8_t {
    no_write = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_NO_WRITE_ACCESS,
    write = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_WRITE_ACCESS
};
    
enum class AktorCommandLength : uint8_t {
    none = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_NONE,
    length_char = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_CHAR,
    length_short = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_SHORT,
    length_long = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_LONG
};


struct AktorCommand_t {
    float factor;
    union buffer_t {
        float floatValue;
        int32_t intValue;

        buffer_t() : floatValue(0) {}
    } buffer;
    AktorCommandWriteAccess writeAccess;
    AktorCommandLength length;
    bool bufferValid;
    
    AktorCommand_t() : 
	factor(1), 
    buffer(),
    writeAccess(AktorCommandWriteAccess::no_write),
	length(AktorCommandLength::length_char),
    bufferValid(false) {}
};

#ifdef TURAG_FELDBUS_AKTOR_STRUCTURED_OUTPUT_AVAILABLE
struct StructuredDataPair_t {
    uint8_t key;
    float value;
    
    StructuredDataPair_t(uint8_t key_, float value_) :
	key(key_),
	value(value_) {}
};
#endif
    
/*
 *
 */
class Aktor : public TURAG::Feldbus::Device {
protected:
    AktorCommand_t* commandSet;
    unsigned int commandSetLength;
    bool commandSetPopulated;

#ifdef TURAG_FELDBUS_AKTOR_STRUCTURED_OUTPUT_AVAILABLE
    unsigned int structuredOutputTableLength;
    std::vector<uint8_t> structuredOutputTable;
#endif    

    
public:
#ifdef TURAG_FELDBUS_AKTOR_STRUCTURED_OUTPUT_AVAILABLE
    Aktor(const char* name_, unsigned int address, ChecksumType type = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_CHECKSUM_TYPE) :
                Device(name_, address, type), commandSet(nullptr), commandSetLength(0), commandSetPopulated(false),
                structuredOutputTableLength(0)  { }
#else
    Aktor(const char* name_, unsigned int address, ChecksumType type = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_CHECKSUM_TYPE) :
                Device(name_, address, type), commandSet(nullptr), commandSetLength(0), commandSetPopulated(false) { }
#endif


    bool getValue(uint8_t key, float* value);
    bool setValue(uint8_t key, float value);
    bool getValue(uint8_t key, int32_t* value);
    bool setValue(uint8_t key, int32_t value);

    bool populateCommandSet(AktorCommand_t* commandSet_, unsigned int commandSetLength_);
    unsigned int getCommandsetLength(void);
    bool getCommandName(uint8_t key, char* name);
    unsigned int getCommandNameLength(uint8_t key);

#ifdef TURAG_FELDBUS_AKTOR_STRUCTURED_OUTPUT_AVAILABLE
    unsigned int getStructuredOutputTableLength(void);
    bool setStructuredOutputTable(const std::vector<uint8_t>& keys);
    bool getStructuredOutput(std::vector<StructuredDataPair_t>* values);
#endif

};

} // namespace Feldbus
} // namespace TURAG

#endif /* TURAGFELDBUSAKTOR_H_ */
