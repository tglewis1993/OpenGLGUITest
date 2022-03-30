#pragma once

class VertexBuffer
{
	unsigned int m_ID = 0;

public: 
	VertexBuffer(const void* data, const unsigned int size);
	~VertexBuffer();

	void Bind(bool makeBind) const;
	
};
