#include <Modules/Graphics/VertexBuffer.h>
#include <Modules/Graphics/Common.h>

VertexBuffer::VertexBuffer(const void* data, const unsigned int size)
{
	GL_CALL(glGenBuffers(1, &m_ID));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW))
}

VertexBuffer::~VertexBuffer()
{
	GL_CALL(glDeleteBuffers(1, &m_ID));
}

void VertexBuffer::Bind(bool makeBind) const
{
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, makeBind ? m_ID : 0));
}
