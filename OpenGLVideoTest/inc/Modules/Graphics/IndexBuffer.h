#pragma once

class IndexBuffer
{
	unsigned int m_ID = 0;
	unsigned int m_Count = 0;

public: 
	IndexBuffer(const unsigned int* data, const unsigned int size);
	~IndexBuffer();

	void Bind(bool makeBind) const;
	
	inline unsigned int GetCount() const { return m_Count; }
};
