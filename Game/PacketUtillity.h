#pragma once

enum PacketType
{
	PACKET_TYPE_NONE = 0,
	PACKET_TYPE_JOIN_PLAYER,
	PACKET_TYPE_LEAVE_PLAYER,
	PACKET_TYPE_UPDATE_PLAYER,
	PACKET_TYPE_JOIN_ROOM,
	PACKET_TYPE_CHAT_MESSAGE,
};

struct PacketHeader
{
	char	TypeName[30] = "\0";
	size_t	BodySize = 0;
};

template<typename T>
concept PacketT =
!std::is_pointer_v<std::remove_cvref_t<T>> &&
std::is_trivially_copyable_v<std::remove_cvref_t<T>>;

inline PacketHeader GetHeaderFromPacketBuffer(const char* pBuffer)
{
	if (pBuffer == nullptr)
	{
		return {};
	}
	return *reinterpret_cast<const PacketHeader*>(pBuffer);
}
template<typename T>
inline const T* PacketCast(const char* pBuffer)
{
	return reinterpret_cast<const T*>(pBuffer);
}
inline const char* GetBodyFromPacketBuffer(const char* pBuffer)
{
	if (pBuffer == nullptr)
	{
		return nullptr;
	}
	return reinterpret_cast<const char*>(pBuffer + sizeof(PacketHeader));
}