#pragma once
#include <iostream>

enum class CustomMsgTypes : uint32_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageServer,
	Publish,
	ServerMessage,
	ServerMessageToClient,
	UploadFileInfo,
	UploadChunk,
	DownloadFilePath,
	DownloadChunk,
	FileEnd
};