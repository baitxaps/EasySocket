CellReadStream = class("CellReadStream");

function CellReadStream:ctor(nSize)
	self._obj = CellReadStream_Create(nSize);
end

function CellReadStream:release()
	CellReadStream_Release(self._obj);
end

function CellReadStream:getNetCmd()
	return CellReadStream_ReadUInt16(self._obj);
end

function CellReadStream:ReadInt8()
	return CellReadStream_ReadInt8(self._obj);
end

function CellReadStream:ReadInt16()
	return CellReadStream_ReadInt16(self._obj);
end

function CellReadStream:ReadInt32()
	return CellReadStream_ReadInt32(self._obj);
end

function CellReadStream:ReadInt64()
	return CellReadStream_ReadInt64(self._obj);
end

function CellReadStream:ReadUInt8()
	return CellReadStream_ReadUInt8(self._obj);
end

function CellReadStream:ReadUInt16()
	return CellReadStream_ReadUInt16(self._obj);
end

function CellReadStream:ReadUInt32()
	return CellReadStream_ReadUInt32(self._obj);
end

function CellReadStream:ReadUInt64()
	return CellReadStream_ReadUInt64(self._obj);
end

function CellReadStream:ReadFloat()
	return CellReadStream_ReadFloat(self._obj);
end

function CellReadStream:ReadDouble()
	return CellReadStream_ReadDouble(self._obj);
end

function CellReadStream:ReadString()
	return CellReadStream_ReadString(self._obj);
end

function CellReadStream:ReadInt32s()
	local tabArr = {}
	--读取数组长度 无符号32位整数
	local len = self:ReadUInt32(len);
	--写入数组元素
	for n = 1, len do
		tabArr[n] = self:ReadInt32();
	end
	return tabArr,len;
end