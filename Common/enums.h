#pragma once
#include <iostream>

enum class CustomMsgTypes : uint32_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageServer,
	MessageAll,
	ServerMessage,
	ServerMessageToClient,
	UploadFileName,
	UploadChunk,
	DownloadFilePath,
	DownloadChunk,
};