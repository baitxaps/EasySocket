CellClient = class("CellClient");

function CellClient:ctor(cb, sendSize, recvSize)
	self._obj = CellClient_Create(cb, sendSize,recvSize);
end

function CellClient:Connect(ip, port)
	return CellClient_Connect(self._obj, ip, port);
end

function CellClient:OnRun()
	return CellClient_OnRun(self._obj);
end

function CellClient:Close()
	CellClient_Close(self._obj);
end

function CellClient:SendWriteStream(writeStream)
	return CellClient_SendWriteStream(self._obj, writeStream:data())
end

--const char* data 
--int datalen
function CellClient:SendData(data, datalen)
	return CellClient_SendData(self._obj, data,datalen);
end
