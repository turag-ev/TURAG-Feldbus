/*
 * feldbus_slave_base.h
 *
 *  Created on: 20.11.2016
 *      Author: martin
 */

#ifndef SLAVE_FELDBUS_SLAVE_BASE_H_
#define SLAVE_FELDBUS_SLAVE_BASE_H_

/**
 * @defgroup feldbus-slave-base Basis-Implementierung
 * @ingroup feldbus-slave
 */


#include <tina++/tina.h>
#include <tina++/thread.h>
#include <tina++/crc.h>

#include <tina/feldbus/slave/feldbus_config_check.h>


namespace TURAG {
namespace Feldbus {
namespace Slave {

/**
 * @brief
 * @ingroup feldbus-slave-base
 */
class Base {
public:
	/**
	 *
	 * @param message
	 * @param message_length
	 * @param response
	 * @return
	 */
	typedef FeldbusSize_t (*PacketProcessor)(const uint8_t* message, FeldbusSize_t message_length, uint8_t* response);

	/**
	 *
	 * @param message
	 * @param message_length
	 * @param protocol_id
	 */
	typedef void (*BroadcastProcessor)(const uint8_t* message, FeldbusSize_t message_length, uint8_t protocol_id);



	/**
	 * @brief
	 * @param packetProcessor
	 * @param broadcastProcessor
	 */
	static void init(PacketProcessor packetProcessor = nullptr, BroadcastProcessor broadcastProcessor = nullptr);

	/**
	 * @brief Verarbeitet ein Packet.
	 * @param[in] message Nachricht incl. Adresse und Checksumme.
	 * @param[in] message_length Nachrichtenlänge.
	 * @param[out] response Antwort, die zurückgesendet werden soll.
	 * Es wird angenommen, dass die Adresse bereits eingetragen ist
	 * und wird folglich nicht von dieser Funktion geschrieben.
	 * @return Anzahl zurückzusendender Bytes. Bei 0 soll keine
	 * Antwort gesendet werden.
	 */
	static FeldbusSize_t processPacket(const uint8_t* message, FeldbusSize_t message_length, uint8_t* response);

#if TURAG_FELDBUS_SLAVE_CONFIG_FLASH_LED || defined(__DOXYGEN__)
	/**
	 *
	 * @param frequency
	 */
	static void doLedPattern(unsigned frequency);
#endif
#if TURAG_FELDBUS_SLAVE_CONFIG_PACKAGE_STATISTICS_AVAILABLE || defined(__DOXYGEN__)
	/**
	 * @brief
	 */
	static void increasePacketLost(void) {
		++info.packetcount_lost;
	}

	/**
	 * @brief
	 */
	static void increaseBufferOverflow(void) {
		++info.packetcount_buffer_overflow;
	}
#endif

private:
	struct Info {
		const char* name;
		const char* versioninfo;
	#if TURAG_FELDBUS_SLAVE_CONFIG_PACKAGE_STATISTICS_AVAILABLE
		uint32_t packetcount_correct;
		uint32_t packetcount_buffer_overflow;
		uint32_t packetcount_lost;
		uint32_t packetcount_chksum_mismatch;
	#endif
	};

	static PacketProcessor packetProcessor;
	#if TURAG_FELDBUS_SLAVE_BROADCASTS_AVAILABLE
		static BroadcastProcessor broadcastProcessor;
	#endif

	static Info info;



};




// debugging functions
#if TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED || defined(__DOXYGEN__)
/** @name Debug-Functions
 * Diese Funktionen können verwendet werden, um Debugausgaben in Form
 * konventioneller Zeichenketten auf dem Bus auszugeben.
 *
 * Soll eine der Debug-Funktionen von turag_feldbus_slave_process_package()
 * aus aufgerufen werden, kann das Makro TURAG_FELDBUS_DEBUG_SAFE() benutzt werden,
 * welches als Argument den kompletten Funktionsaufruf entgegennimmt.
 *
 * \pre Nur verfügbar, wenn \ref TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED auf 1 definiert ist.
 *
 */
///@{
	/// printf-like function
	void printf(const char *fmt, ...);

	void puts(const char* s);
///@}
#else
	inline void printf(const char *, ...) {}
	inline void puts(const char* ) {}
#endif


}
}
}





#endif /* SLAVE_FELDBUS_SLAVE_BASE_H_ */
