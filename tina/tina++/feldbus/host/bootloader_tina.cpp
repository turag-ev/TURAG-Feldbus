/**
 *  @brief		TURAG feldbus bootloader base class
 *  @file		bootloader_tina.cpp
 *  @date		11.11.2014
 *  @author		Florian Voelker
 *
 */

#define TURAG_DEBUG_LOG_SOURCE "B"


#include "bootloader.h"

#include <tina++/tina.h>
#include <tina/debug.h>

#include <algorithm>
#include <cstring>



namespace TURAG {
namespace Feldbus {

	
bool Bootloader::sendEnterBootloaderBroadcast(void) {
    Device::Broadcast<uint8_t> broadcast;
    broadcast.id = TURAG_FELDBUS_DEVICE_PROTOCOL_BOOTLOADER;
    broadcast.data = TURAG_FELDBUS_BOOTLOADER_COMMAND_ENTER_BOOTLOADER;

    return transceive(broadcast);
}

bool Bootloader::sendStartProgramBroadcast(void) {
    Device::Broadcast<uint8_t> broadcast;
    broadcast.id = TURAG_FELDBUS_DEVICE_PROTOCOL_BOOTLOADER;
    broadcast.data = TURAG_FELDBUS_BOOTLOADER_COMMAND_START_PROGRAMM;

    return transceive(broadcast);
}

uint16_t Bootloader::getMcuId(void) {
	if (myMcuId == TURAG_FELDBUS_BOOTLOADER_MCU_ID_INVALID) {
		struct McuId {
			uint16_t id;
		} _packed;

		Device::Request<uint8_t> request;
		request.data = TURAG_FELDBUS_BOOTLOADER_COMMAND_GET_MCUID;
		
		Device::Response<McuId> response;
		
		if (transceive(request, &response)) {
			myMcuId = response.data.id;
		}
	}
	return myMcuId;
}

bool Bootloader::unlockBootloader(void) {
	getMcuId();
	
	if (myMcuId == TURAG_FELDBUS_BOOTLOADER_MCU_ID_INVALID) {
		turag_errorf("%s: tried to call Bootloader::unlockBootloader, but couldn't read MCU-ID", name);
		return false;
	}
	
	struct UnlockBootloader {
		uint8_t key;
		uint16_t mcuId;
		uint16_t unlockCode;
	} _packed;

	Device::Request<UnlockBootloader> request;
	request.data.key = TURAG_FELDBUS_BOOTLOADER_COMMAND_UNLOCK_BOOTLOADER;
	request.data.mcuId = myMcuId;
	request.data.unlockCode = TURAG_FELDBUS_BOOTLOADER_UNLOCK_CODE;
	
	Device::Response<uint8_t> response;
	
	if (!transceive(request, &response)) {
		return false;
	} else {
		if (response.data == TURAG_FELDBUS_BOOTLOADER_RESPONSE_UNLOCKED) {
			return true;
		} else {
			turag_infof("%s: bootloader unlock rejected", name);
			return false;
		}
	}
}



bool BootloaderAtmega::readFuses(Fuses* fuseBuffer) {
	Device::Request<uint8_t> request;
	request.data = TURAG_FELDBUS_BOOTLOADER_ATMEGA_GET_FUSES;
	
	Device::Response<Fuses> response;
	
	if (!transceive(request, &response)) {
		return false;
	} else {
		*fuseBuffer = response.data;
		return true;
	}
}

uint16_t BootloaderAtmega::getPageSize(void) {
	if (myPageSize == 0) {
		struct PageSize {
			uint16_t size;
		} _packed;

		Device::Request<uint8_t> request;
		request.data = TURAG_FELDBUS_BOOTLOADER_ATMEGA_GET_PAGE_SIZE;
		
		Device::Response<PageSize> response;
		
		if (transceive(request, &response)) {
			myPageSize = response.data.size;
		}
	}
	return myPageSize;
}

uint32_t BootloaderAtmega::getFlashSize(bool writable) {
	struct GetFlashSize {
		uint8_t key;
		uint8_t writable;
	};
	
	if (writable) {
		if (myWritableFlashSize == 0) {
			struct FlashSize {
				uint32_t size;
			} _packed;

			Device::Request<GetFlashSize> request;
			request.data.key = TURAG_FELDBUS_BOOTLOADER_ATMEGA_GET_FLASH_SIZE;
			request.data.writable = 1;
			
			Device::Response<FlashSize> response;
			
			if (transceive(request, &response)) {
				myWritableFlashSize = response.data.size;
			}
		}
		return myWritableFlashSize;
	} else {
		if (myFlashSize == 0) {
			struct FlashSize {
				uint32_t size;
			} _packed;

			Device::Request<GetFlashSize> request;
			request.data.key = TURAG_FELDBUS_BOOTLOADER_ATMEGA_GET_FLASH_SIZE;
			request.data.writable = 0;
			
			Device::Response<FlashSize> response;
			
			if (transceive(request, &response)) {
				myFlashSize = response.data.size;
			}
		}
		return myFlashSize;
	}
}

BootloaderAtmega::ErrorCode BootloaderAtmega::writeFlash(uint32_t byteAddress, uint32_t length, uint8_t* data) {
	if (!data) {
		return ErrorCode::invalid_args;
	}
	getPageSize();
	if (myPageSize == 0) {
		turag_errorf("%s: tried to call BootloaderAtmega::writePage, but couldn't read page size", name);
		return ErrorCode::preconditions_not_met;
	}
	getFlashSize(true);
	if (myWritableFlashSize == 0) {
		turag_errorf("%s: tried to call BootloaderAtmega::writePage, but couldn't read writable flash size", name);
		return ErrorCode::preconditions_not_met;
	}
	if (myWritableFlashSize - byteAddress < length) {
		turag_errorf("%s: in call to BootloaderAtmega::writePage: write would exceed writable flash boundary", name);
		return ErrorCode::invalid_args;
	}
	if (byteAddress % myPageSize != 0) {
		turag_errorf("%s: in call to BootloaderAtmega::writePage: address must be page-aligned", name);
		return ErrorCode::invalid_args;
	}
	
	unsigned pages = length / myPageSize;
	if (length % myPageSize) {
		++pages;
	}
	
	uint8_t request[myAddressLength + 1 + 4 + myPageSize + 1];
	request[myAddressLength] = TURAG_FELDBUS_BOOTLOADER_ATMEGA_PAGE_WRITE;
	
	uint8_t response[myAddressLength + 1 + 1];
	
	uint32_t targetAddress = byteAddress;
	
	for (unsigned i = 0; i < pages; ++i) {
		*((uint32_t*)(request + myAddressLength + 1)) = targetAddress;
		
		uint16_t currentPageSize = std::min(myPageSize, (uint16_t)(byteAddress + length - targetAddress));
		memcpy(request + myAddressLength + 5, data, currentPageSize);
		
		unsigned k = 0;
		for (k = 0; k < maxTriesForWriting; ++k) {
			if (!transceive(request, sizeof(request), response, sizeof(response))) {
				return ErrorCode::transceive_error;
			}
			if (response[myAddressLength] == TURAG_FELDBUS_BOOTLOADER_ATMEGA_RESPONSE_SUCCESS) {
				break;
			} else if (response[myAddressLength] == TURAG_FELDBUS_BOOTLOADER_ATMEGA_RESPONSE_FAIL_CONTENT) {
				continue;
			} else {
				return static_cast<ErrorCode>(response[myAddressLength]);
			}
		}
		// we give up after a few tries.
		if (k == maxTriesForWriting) {
			return ErrorCode::content_mismatch;
		}
		
		targetAddress += myPageSize;
		data += myPageSize;
	}
	
	return ErrorCode::success;
}

BootloaderAtmega::ErrorCode BootloaderAtmega::readFlash(uint32_t byteAddress, uint32_t length, uint8_t* buffer) {
	if (!buffer) {
		return ErrorCode::invalid_args;
	}
	getFlashSize(false);
	if (myFlashSize == 0) {
		turag_errorf("%s: tried to call BootloaderAtmega::readData, but couldn't read flash size", name);
		return ErrorCode::preconditions_not_met;
	}
	getDeviceInfo(nullptr);
	if (myDeviceInfo.bufferSize == 0) {
		turag_errorf("%s: tried to call BootloaderAtmega::readData, but couldn't read device info", name);
		return ErrorCode::preconditions_not_met;
	}
	if (myFlashSize - byteAddress < length) {
		turag_errorf("%s: in call to BootloaderAtmega::readData: read would exceed flash size", name);
		return ErrorCode::invalid_args;
	}
	
	uint32_t packetSize = myDeviceInfo.bufferSize - myAddressLength - 1 - 1;
	
	// We are conservative because the transmission is only secured with an 8 bit CRC.
	packetSize = std::min(packetSize, (uint32_t)64);
	
	unsigned packets = length / packetSize;
	if (length % packetSize) {
		++packets;
	}

	uint8_t request[myAddressLength + 1 + 4 + 2 + 1];
	request[myAddressLength] = TURAG_FELDBUS_BOOTLOADER_ATMEGA_DATA_READ;
	
	uint32_t targetAddress = byteAddress;
	
	for (unsigned i = 0; i < packets; ++i) {
		uint16_t currentPacketSize = std::min(packetSize, byteAddress + length - targetAddress);
		
		*((uint32_t*)(request + myAddressLength + 1)) = targetAddress;
		*((uint16_t*)(request + myAddressLength + 5)) = currentPacketSize;
		
		uint8_t response[myAddressLength + 1 + currentPacketSize + 1];
		
		if (!transceive(request, sizeof(request), response, sizeof(response))) {
			return ErrorCode::transceive_error;
		}
		// this shouldn't happen
		if (response[myAddressLength] == TURAG_FELDBUS_BOOTLOADER_ATMEGA_RESPONSE_FAIL_ADDRESS) {
			return ErrorCode::invalid_address;
		}
		
		memcpy(buffer, response + myAddressLength + 1, currentPacketSize);
		
		targetAddress += currentPacketSize;
		buffer += currentPacketSize;
	}
	return ErrorCode::success;
}

const char* BootloaderAtmega::errorName(BootloaderAtmega::ErrorCode errorCode) {
	switch (errorCode) {
		case ErrorCode::success: return "Erfolgreich."; break;
		case ErrorCode::invalid_size: return "Ungültige Größe."; break;
		case ErrorCode::invalid_address: return "Ungültige Adresse."; break;
		case ErrorCode::content_mismatch: return "Schreibvorgang fehlgeschlagen."; break;
		case ErrorCode::invalid_args: return "Ungültige Argumente."; break;
		case ErrorCode::transceive_error: return "Übertragung fehlgeschlagen."; break;
		case ErrorCode::preconditions_not_met: return "Voraussetzungen nicht erfüllt."; break;
	}
	return "";
}

const char* BootloaderAtmega::errorDescription(BootloaderAtmega::ErrorCode errorCode) {
	switch (errorCode) {
		case ErrorCode::success: return "Keine Fehler."; break;
		case ErrorCode::invalid_size: return "Das Gerät meldet, dass die vom Host angeforderte Datenmenge ungültig ist."; break;
		case ErrorCode::invalid_address: return "Das Gerät meldet, dass die vom Host angegebene Adresse ungültig ist.."; break;
		case ErrorCode::content_mismatch: return "Das Gerät meldet, dass die geschriebenen Daten nicht den gelesenen entsprechen."; break;
		case ErrorCode::invalid_args: return "Einige der übergebenen Argumente sind nicht gültig."; break;
		case ErrorCode::transceive_error: return "Die Übertragung der Daten zum Gerät schlug fehl."; break;
		case ErrorCode::preconditions_not_met: return "Einige zum Ausführen dieses Befehls nötigen Kommandos scheiterten."; break;
	}
	return "";
}


} // namespace Feldbus
} // namespace TURAG

