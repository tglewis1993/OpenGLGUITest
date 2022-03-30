#include <Modules/Graphics/IndexBuffer.h>
#include <Modules/Graphics/Common.h>

IndexBuffer::IndexBuffer(const unsigned int* data, const unsigned int count) : m_Count(count)
{
	GL_CALL(glGenBuffers(1, &m_ID));
	GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW))
}

IndexBuffer::~IndexBuffer()
{
	GL_CALL(glDeleteBuffers(1, &m_ID));
}

void IndexBuffer::Bind(bool makeBind) const
{
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, makeBind ? m_ID : 0));
}
