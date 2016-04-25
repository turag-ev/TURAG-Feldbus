/**
 *  @brief		TURAG feldbus bootloader base class
 *  @file		bootloader.h
 *  @date		11.11.2014
 *  @author		Florian Voelker
 *
 */


#ifndef TURAGFELDBUSBOOTLOADER_H_
#define TURAGFELDBUSBOOTLOADER_H_

#include "device.h"
#include <tina++/tina.h>
#include <tina/feldbus/protocol/turag_feldbus_fuer_bootloader.h>


namespace TURAG {
namespace Feldbus {

class Bootloader : public TURAG::Feldbus::Device {
public:
	/**
	 * \brief Konstruktor.
	 * \param[in] name_
	 * \param[in] address
	 * \param[in] type
	 * \param[in] addressLength
	 */
    Bootloader(const char* name_, unsigned address, FeldbusAbstraction* feldbus, ChecksumType type = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_CHECKSUM_TYPE,
		const AddressLength addressLength = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_ADDRESS_LENGTH) :
        Device(name_, address, feldbus, type, addressLength), myMcuId(TURAG_FELDBUS_BOOTLOADER_MCU_ID_INVALID), unlocked(false)
    {
    }
    
    bool sendEnterBootloaderBroadcast(void);
	bool sendStartProgramBroadcast(void);

	uint16_t getMcuId(void);
	bool unlockBootloader(void);
	bool isUnlocked(void) const { return unlocked; }
	
private:
	uint16_t myMcuId;
	bool unlocked;
};


class BootloaderAvrBase : public Bootloader {
public:
	enum class ErrorCode {
		success = TURAG_FELDBUS_BOOTLOADER_AVR_RESPONSE_SUCCESS,
		invalid_size = TURAG_FELDBUS_BOOTLOADER_AVR_RESPONSE_FAIL_SIZE,
		invalid_address = TURAG_FELDBUS_BOOTLOADER_AVR_RESPONSE_FAIL_ADDRESS,
		unsupported = TURAG_FELDBUS_BOOTLOADER_AVR_RESPONSE_FAIL_NOT_SUPPORTED,
		content_mismatch = TURAG_FELDBUS_BOOTLOADER_AVR_RESPONSE_FAIL_CONTENT,
		invalid_args,
		transceive_error,
		preconditions_not_met
	};
	
	static constexpr unsigned maxTriesForWriting = 3;
	
	/**
	 * \brief Konstruktor.
	 * \param[in] name_
	 * \param[in] address
	 * \param[in] type
	 * \param[in] addressLength
	 */
    BootloaderAvrBase(const char* name_, unsigned address, FeldbusAbstraction* feldbus, ChecksumType type = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_CHECKSUM_TYPE,
		const AddressLength addressLength = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_ADDRESS_LENGTH) :
        Bootloader(name_, address, feldbus, type, addressLength), myPageSize(0), myFlashSize(0), myWritableFlashSize(0)
    {
    }
	
	/**
	 * \brief Liest die Größe einer Flashseite aus.
	 * \return 0 bei einem Fehler, sonst die Größe.
	 */
	uint16_t getPageSize(void);
	
	/**
	 * \brief Liest die Größe des Flashs aus.
	 * \param[in] writable Gibt an ob die Größe des beschreibbaren 
	 * Teils oder die Gesamtgröße zurückgegeben werden soll.
	 * \return 0 bei einem Fehler, sonst die Größe.
	 */
	uint32_t getFlashSize(bool writable);
	
	/**
	 * \brief Beschreibt den Flash des Gerätes.
	 * \param[in] byteAddress Zieladresse im Gerät. Muss ein Vielfaches der Pagegröße sein.
	 * \param[in] length Menge der zu schreibenden Daten.
	 * \param[in] data Pointer zu den Daten.
	 * \return ErrorCode.
	 */
	ErrorCode writeFlash(uint32_t byteAddress, uint32_t length, uint8_t* data);
	
	/**
	 * \brief Liest Daten aus dem Flash.
	 * \param[in] byteAddress Quelladresse im Gerät.
	 * \param[in] length Menge der auszulesenden Daten.
	 * \param[out] buffer Puffer, der die ausgelesenen Daten aufnimmt.
	 * \return ErrorCode.
	 */
	ErrorCode readFlash(uint32_t byteAddress, uint32_t length, uint8_t* buffer);

	static const char* errorName(ErrorCode errorCode);
	static const char* errorDescription(ErrorCode errorCode);
	
private:
	uint16_t myPageSize;
	uint32_t myFlashSize;
	uint32_t myWritableFlashSize;
};


class BootloaderAtmega : public BootloaderAvrBase {
public:
	struct Fuses {
		uint8_t low;
		uint8_t high;
		uint8_t extended;
	};

	/**
	 * \brief Konstruktor.
	 * \param[in] name_
	 * \param[in] address
	 * \param[in] type
	 * \param[in] addressLength
	 */
    BootloaderAtmega(const char* name_, unsigned address, FeldbusAbstraction* feldbus, ChecksumType type = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_CHECKSUM_TYPE,
		const AddressLength addressLength = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_ADDRESS_LENGTH) :
        BootloaderAvrBase(name_, address, feldbus, type, addressLength)
	{
	}

	/**
	 * \brief Liest die 3 Fuse-Bytes des Gerätes aus.
	 * \param[out] fuseBuffer Ausgabepuffer.
	 * \return True bei Erfolg, ansonsten false.
	 */
	bool readFuses(Fuses* fuseBuffer);

};


class BootloaderXmega : public BootloaderAvrBase {
public:
	struct Fusebyte {
		bool available;
		uint8_t value;
	};

	struct Fuses {
		Fusebyte fuse0;
		Fusebyte fuse1;
		Fusebyte fuse2;
		Fusebyte fuse3;
		Fusebyte fuse4;
		Fusebyte fuse5;
		Fusebyte fuse6;
	};

	/**
	 * \brief Konstruktor.
	 * \param[in] name_
	 * \param[in] address
	 * \param[in] type
	 * \param[in] addressLength
	 */
    BootloaderXmega(const char* name_, unsigned address, FeldbusAbstraction* feldbus, ChecksumType type = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_CHECKSUM_TYPE,
		const AddressLength addressLength = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_ADDRESS_LENGTH) :
        BootloaderAvrBase(name_, address, feldbus, type, addressLength), revisionId(0)
	{
	}

	/**
	 * \brief Liest die Fuse-Bytes des Gerätes aus.
	 * \param[out] fuseBuffer Ausgabepuffer.
	 * \return True bei Erfolg, ansonsten false.
	 */
	bool readFuses(Fuses* fuseBuffer);

	/**
	 * @brief Gibt die Revision ID des Controllers zurück.
	 * @return 0 bei einem Fehler, ansonsten eine Zahl zwischen
	 * 65 und 80 (Buchstaben A...P).
	 */
	char getRevisionId(void);

private:
	char revisionId;
};


} // namespace Feldbus
} // namespace TURAG

#endif /* TURAGFELDBUSBOOTLOADER_H_ */
