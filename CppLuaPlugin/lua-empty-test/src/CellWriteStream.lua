CellWriteStream = class("CellWriteStream");

function CellWriteStream:ctor(nSize)
	self._obj = CellWriteStream_Create(nSize);
end

function CellWriteStream:release()
	CellWriteStream_Release(self._obj);
end

function CellWriteStream:data()
	return self._obj;
end

function CellWriteStream:setNetCmd(n)
	return CellWriteStream_WriteUInt16(self._obj, n);
end

function CellWriteStream:WriteInt8(n)
	return CellWriteStream_WriteInt8(self._obj, n);
end

function CellWriteStream:WriteInt16(n)
	return CellWriteStream_WriteInt16(self._obj, n);
end

function CellWriteStream:WriteInt32(n)
	return CellWriteStream_WriteInt32(self._obj, n);
end

function CellWriteStream:WriteInt64(n)
	return CellWriteStream_WriteInt64(self._obj, n);
end

function CellWriteStream:WriteUInt8(n)
	return CellWriteStream_WriteUInt8(self._obj, n);
end

function CellWriteStream:WriteUInt16(n)
	return CellWriteStream_WriteUInt16(self._obj, n);
end

function CellWriteStream:WriteUInt32(n)
	return CellWriteStream_WriteUInt32(self._obj, n);
end

function CellWriteStream:WriteUInt64(n)
	return CellWriteStream_WriteUInt64(self._obj, n);
end

function CellWriteStream:WriteFloat(n)
	return CellWriteStream_WriteFloat(self._obj, n);
end

function CellWriteStream:WriteDouble(n)
	return CellWriteStream_WriteDouble(self._obj, n);
end

function CellWriteStream:WriteString(n)
	return CellWriteStream_WriteString(self._obj, n);
end

function CellWriteStream:WriteInt32s(tabArr)
	local len = #tabArr
	--写入数组长度 无符号32位整数
	self:WriteUInt32(len);
	--写入数组元素
	for n = 1, len do
		self:WriteInt32(tabArr[n]);
	end
end