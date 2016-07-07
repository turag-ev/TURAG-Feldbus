
#include <tina++/tina.h>

#if TURAG_USE_TURAG_FELDBUS_HOST


#include <tina++/feldbus/host/feldbusabstraction.h>
#include <tina++/debug.h>
#include <tina++/crc/xor.h>
#include <tina++/crc/crc.h>


namespace TURAG {
namespace Feldbus {

FeldbusAbstraction::ResultStatus FeldbusAbstraction::transceive(const uint8_t *transmit, int *transmit_length, uint8_t *receive, int *receive_length, unsigned targetAddress, ChecksumType checksumType)
{
	if (threadSafe_) {
		sem_.wait();
	}

	// we need to delay the transmission for protocol compliance reasons in the following
	// cases:
	// - our last transmission was a broadcast
	// - the last transmission was sent to a different slave
	bool insertTransmissionDelay;
	if (lastTargetAddress_ == TURAG_FELDBUS_BROADCAST_ADDR ||
			lastTargetAddress_ != targetAddress) {
		insertTransmissionDelay = true;
	} else {
		insertTransmissionDelay = false;
	}

	bool transceiveSuccessful = doTransceive(transmit, transmit_length, receive, receive_length, insertTransmissionDelay);
	ResultStatus status = ResultStatus::TransmissionError;

	// Transmission seems fine, lets look at the checksum.
	// It might seem less than ideal to let the device calculate the transmit checksum and then
	// check the received one here but it's due to performance reasons. The device should calculate
	// the outgoing one to save it in case the first transmission attempt fails. But we also want checksum
	// errors to be reflected in the statistics thus the need to calculate the received checksum here.
	// Depending on the implementation of the checksum this calculation might take some time
	// while the lock is still acquired. But the other option would be to release it and reacquire it
	// afterwards, which I felt would in most cases probably be more costly.
	if (transceiveSuccessful)
	{
		if (!receive || receive_length == 0) {
			// if the caller doesn't want to receive anything and the transmission was successful
			// we can't check the checksum
			status = ResultStatus::Success;
		} else {
			// transmission seems fine, lets look at the checksum
			bool checksum_correct = false;
			switch (checksumType) {
			case ChecksumType::xor_based:
				checksum_correct = XOR::check(receive, *receive_length-1, receive[*receive_length-1]);
				break;

			case ChecksumType::crc8_icode:
				checksum_correct = CRC8::check(receive, *receive_length-1, receive[*receive_length-1]);
				break;

			case ChecksumType::none:
				checksum_correct = true;
			}
			if (checksum_correct) {
				status = ResultStatus::Success;
			} else {
				status = ResultStatus::ChecksumError;
			}
		}
	}


	if (busTransmissionStatistics_.doErrorOutput(status == ResultStatus::Success))
	{
		turag_warningf("%s: %u/%u failed transmissions on the bus so far (%u %% success).",
					   name(),
					   busTransmissionStatistics_.getErrorCount(),
					   busTransmissionStatistics_.getTotalCount(),
					   busTransmissionStatistics_.getSuccessRatio());
	}

	if (threadSafe_) {
		sem_.signal();
	}

	return status;
}


} // namespace Feldbus
} // namespace TURAG

#endif // TURAG_USE_TURAG_FELDBUS_HOST
