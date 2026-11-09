#pragma once

class IndexBuffer
{
private:
	unsigned int m_RendererID;
	size_t m_Count;

public:
	IndexBuffer(const unsigned int* data, size_t size);
	~IndexBuffer();

	void Bind() const;

	unsigned int GetCount() const;
};