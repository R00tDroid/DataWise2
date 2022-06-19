#pragma once
#include <vector>

class ByteStream
{
public:
	template<class T>
	void Write(void* data, unsigned int element_count)
	{
		unsigned int data_size = sizeof(T) * element_count;
		unsigned insertion_index = data_.size();
		data_.resize(data_.size() + data_size);
		memcpy(&data_[insertion_index], data, data_size);
		//std::copy((unsigned char*)data, (unsigned char*)data + data_size, &data_[insertion_index]);

		cursor_ = 0;
	}

	template<class T>
	void Write(T value)
	{
		Write<T>(&value, 1);
	}

	template<>
	void Write<std::string>(std::string value)
	{
		unsigned int length = value.length();
		Write<unsigned int>(&length, 1);
		Write<char>((void*)value.c_str(), length);
	}

	template<class T>
	void Read(void* target, unsigned int element_count)
	{
		unsigned int data_size = sizeof(T) * element_count;
		memcpy(target, &data_[cursor_], data_size);

		cursor_ += data_size;
	}

	void Erase(unsigned int first_byte, unsigned int byte_count)
	{
		data_.erase(data_.begin() + first_byte, data_.begin() + first_byte + byte_count);
		cursor_ = 0;
	}

	template<class T>
	void Skip(unsigned int element_count)
	{
		cursor_ += sizeof(T) * element_count;
	}

	unsigned int Size() { return data_.size(); }

	unsigned int Available() { return data_.size() - cursor_; }

	void* operator [] (unsigned int index)
	{
		return &data_[index];
	}

	unsigned char* Data()
	{
		return &data_[0];
	}

private:
	std::vector<unsigned char> data_;
	unsigned int cursor_ = 0;
};
