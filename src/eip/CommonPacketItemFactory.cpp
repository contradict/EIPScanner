//
// Created by Aleksey Timin on 11/16/19.
//

#include "CommonPacketItemFactory.h"
#include "utils/Buffer.h"

namespace eipScanner {
namespace eip {
	using utils::Buffer;

	CommonPacketItem CommonPacketItemFactory::createNullAddressItem() const {
		return CommonPacketItem();
	}

	CommonPacketItem CommonPacketItemFactory::createUnconnectedDataItem(const std::vector<uint8_t> &data) const {
		return CommonPacketItem(CommonPacketItemIds::UNCONNECTED_MESSAGE, data);
	}

	CommonPacketItem CommonPacketItemFactory::createConnectedDataItem(const std::vector<uint8_t> &data) const {
		return CommonPacketItem(CommonPacketItemIds::CONNECTED_TRANSPORT_PACKET, data);
	}

	CommonPacketItem
	CommonPacketItemFactory::createSequenceAddressItem(cip::CipUdint connectionId, cip::CipUdint seqNumber) const{
		Buffer buffer;
		buffer << connectionId << seqNumber;
		return CommonPacketItem(CommonPacketItemIds::SEQUENCED_ADDRESS_ITEM, buffer.data());
	}
    CommonPacketItem
    CommonPacketItemFactory::createT2OSockaddrInfo(cip::CipUint port, cip::CipUdint address) const {
        Buffer buffer;
        buffer << htons(cip::CipInt(2)) << htons(port) << htonl(address) << cip::CipUdint(0) << cip::CipUdint(0);
        return CommonPacketItem(CommonPacketItemIds::T2O_SOCKADDR_INFO, buffer.data());
    }
}
}
