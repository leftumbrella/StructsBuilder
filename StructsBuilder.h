#ifndef LFTUBRLA_SWAY_STRUCTSBUILDER_STRUCTSBUILDER_
#define LFTUBRLA_SWAY_STRUCTSBUILDER_STRUCTSBUILDER_

/*
*	Sway Create this file. at November 28, 2021.
*	-----------------------------------------------------------------------------------------
*  	| ������ܹ��������κλ���������ɣ������κ��ֽڶ��뷽ʽ������ṹ�岢ʹ���ԡ�void*����ʽ���أ�			|
*	| �ˡ�void*��ָ���ܹ�ǿ��ת��Ϊָ����ʵ�ṹ���ָ�룬���ܶ�ȡ����ʵ���ݡ�							|
*   -----------------------------------------------------------------------------------------
*  	| ���಻�Ƕ��̰߳�ȫ�ģ�����ζ�������Ҫ�ڶ��̳߳�����ʹ�ã�����뱣֤���ȡ������ֲ���Ԥ֪�Ĵ���	 	|
*	| push_back/push_front/insert �����ṩ�����ڴ��ݽṹ���е���������� 							|
*	| operator<< �����޷������д����������ͣ������������ͻ������� 								|
*   | �����޷������ṹ���к�����Ҫ�����乹�캯������Ľṹ��  										|
*   -----------------------------------------------------------------------------------------
*/

#include <deque>
#include <memory>
#include <utility>

#ifndef _LFUBRLA_CHANGE_ARG_
	#define _LFUBRLA_CHANGE_ARG_	// Parameters marked with this macro will be modified after the function call succeeds
#endif

class StructsBuilder{

public:
	StructsBuilder(){

	}
	~StructsBuilder()	{
		release_memory();
	}

public:
	/************************************
	* Description:	Append a field to the end of the struct being built
	*
	* Param:	const T & data_ The value to append
	*
	* Return:	StructsBuilder& Reference to the current object
	*************************************/
	template<typename T> StructsBuilder& push_back(const T& data_) {
		_data.push_back(std::pair<std::size_t, void*>(sizeof(T), new_data(data_)));
		return *this;
	}
	/************************************
	* Description:	Append a field to the end of the struct being built (also supports arrays of struct fields)
	*
	* Param:	const T * data_ Pointer to the array to append
	* Param:	std::size_t size_ Size of the array to append
	*
	* Return:	StructsBuilder& Reference to the current object
	*************************************/
	template<typename T> StructsBuilder& push_back(const T* data_, std::size_t size_) {
		for (std::size_t i = 0; i < size_; ++i) {
			push_back(data_[i]);
		}
		return *this;
	}
	/************************************
	* Description:	Prepend a field to the front of the struct being built
	*
	* Param:	const T & data_ The value to prepend
	*
	* Return:	StructsBuilder& Reference to the current object
	*************************************/
	template<typename T> StructsBuilder& push_front(const T& data_) {
		_data.push_front(std::pair<std::size_t, void*>(sizeof(T), new_data(data_)));
		return *this;
	}
	/************************************
	* Description:	Prepend a field to the front of the struct being built (also supports arrays of struct fields)
	*
	* Param:	const T * data_ Pointer to the array to prepend
	* Param:	std::size_t size_ Size of the array to prepend
	*
	* Return:	StructsBuilder& Reference to the current object
	*************************************/
	template<typename T> StructsBuilder& push_front(const T* data_, std::size_t size_) {
		for (std::size_t i = 0; i < size_; ++i) {
			push_front(data_[i]);
		}
		return *this;
	}
	/************************************
	* Description:	Insert a field at a specific position in the struct being built
	*
	* Param:	std::size_t position_ The position to insert at (0-based index)
	* Param:	const T & data_ The value to insert
	*
	* Return:	StructsBuilder& Reference to the current object
	*************************************/
	template<typename T> StructsBuilder& insert(std::size_t position_, const T& data_) {
		_data.insert(_data.begin() + position_, std::pair<std::size_t, void*>(sizeof(T), new_data(data_)));
		return *this;
	}
	/************************************
	* Description:	Insert a field at a specific position in the struct being built (also supports arrays of struct fields)
	*
	* Param:	std::size_t position_ The position to insert at (0-based index)
	* Param:	const T * data_ Pointer to the array to insert
	* Param:	std::size_t size_ Size of the array to insert
	*
	* Return:	StructsBuilder& Reference to the current object
	*************************************/
	template<typename T> StructsBuilder& insert(std::size_t position_, const T* data_, std::size_t size_) {
		for (std::size_t i = 0; i < size_; ++i) {
			insert(position_ + i, data_[i]);
		}
		return *this;
	}
	/************************************
	* Description:	Continuously append data to the end of the struct being built
	*
	* Param:	const T & data_ The value to append
	*
	* Return:	StructsBuilder& Reference to the current object
	*************************************/
	template<typename T> StructsBuilder& operator<<(const T& data_) {
		return push_back(data_);
	}
	/************************************
	* Description:	Read memory from the currently built struct
	*
	* Param:	_LF_CHANGE_ARG_ void * & data_ On success, the struct memory will be stored at the address pointed to by this pointer
	* Param:	std::size_t byte_alignment_ Byte alignment for the struct (0 for system default byte alignment)
	*
	* Return:	std::size_t The byte size of the memory read on success
	*************************************/
	std::size_t read_data(_LFUBRLA_CHANGE_ARG_ void*& data_, std::size_t byte_alignment_ = 0) const {
		std::size_t over_size = 0;
		if (byte_alignment_ == 0) {
			over_size = default_memory_setting(data_);
		}
		else {
			over_size = memory_setting(data_, byte_alignment_);
		}
		return over_size;
	}
	/************************************
	* Description:	Check whether no struct fields have been built
	*
	* Param:	void
	*
	* Return:	bool true: the current struct is empty; false: the current struct is not empty
	*************************************/
	bool empty(void) const {
		return _data.empty();
	}
	/************************************
	* Description:	Clear all data in the builder
	*
	* Param:	void
	*
	* Return:	StructsBuilder& Reference to the current object
	*************************************/
	StructsBuilder& clear(void) {
		release_memory();
		_data.clear();
		return *this;
	}

private:
	// Allocate a new piece of memory
	template<typename T> void* new_data(const T& data_) {
		return new T(data_);
	}
	// Get memory of the currently built struct under default byte alignment
	std::size_t default_memory_setting(_LFUBRLA_CHANGE_ARG_ void*& data_) const {
		std::size_t all_size = 0;
		std::deque<std::pair<std::size_t, void*> >::const_iterator itr;
		std::size_t max_size = 0;
		for (itr = _data.cbegin(); itr != _data.cend(); ++itr) {
			if (itr->first > max_size) {
				max_size = itr->first;
			}
			// Under default byte alignment, struct memory layout aligns each field to a memory boundary offset that is a multiple of the field's own size
			if (all_size % itr->first != 0) {
				std::size_t next_size = round_up(itr->first, all_size);
				push_nullptr(data_, all_size, next_size - all_size);
				all_size = next_size;
			}
			memory_push(data_, all_size, itr->second, itr->first);
			all_size += itr->first;
		}
		// Under default byte alignment, the struct's total memory is padded to a multiple of the largest element's memory size
		if (all_size % max_size != 0) {
			std::size_t next_size = round_up(max_size, all_size);
			push_nullptr(data_, all_size, next_size - all_size);
			all_size = next_size;
		}
		return all_size;
	}
	// Get memory of the currently built struct under custom byte alignment
	std::size_t memory_setting(_LFUBRLA_CHANGE_ARG_ void*& data_, std::size_t byte_alignment_) const {
		std::size_t all_size = 0;
		std::deque<std::pair<std::size_t, void*> >::const_iterator itr;
		std::size_t max_size = 0;
		for (itr = _data.cbegin(); itr != _data.cend(); ++itr) {
			// Under custom byte alignment, the current element's alignment is the smaller of the element's own alignment and the specified alignment
			std::size_t now_byte_alignment = std::min(itr->first, byte_alignment_);
			if (now_byte_alignment > max_size) {
				max_size = now_byte_alignment;
			}
			if (all_size % now_byte_alignment != 0) {
				std::size_t next_size = round_up(now_byte_alignment, all_size);
				push_nullptr(data_, all_size, next_size - all_size);
				all_size = next_size;
			}
			memory_push(data_, all_size, itr->second, itr->first);
			all_size += itr->first;
		}
		// Under custom byte alignment, the struct's total memory is padded to a multiple of the effective alignment boundary calculated above
		if (all_size % max_size != 0) {
			std::size_t next_size = round_up(max_size, all_size);
			push_nullptr(data_, all_size, next_size - all_size);
			all_size = next_size;
		}
		return all_size;
	}
	// Append new memory to an existing memory block
	std::size_t memory_push(_LFUBRLA_CHANGE_ARG_ void*& src_ptr_, std::size_t src_size_, const void* new_ptr_, std::size_t new_size_) const {
		char* tmp_data = (char*)std::malloc(src_size_ + new_size_);
		if (tmp_data == nullptr) {
			return 0;
		}
		std::memcpy(tmp_data, src_ptr_, src_size_);
		std::memcpy(tmp_data + src_size_, new_ptr_, new_size_);
		free(src_ptr_);
		src_ptr_ = tmp_data;
		return src_size_ + new_size_;
	}
	// Append null bytes (padding) to an existing memory block
	std::size_t push_nullptr(_LFUBRLA_CHANGE_ARG_ void*& src_ptr_, std::size_t src_size_, std::size_t nullptr_size_) const {
		if (nullptr_size_ == 0) {
			return src_size_;
		}
		void* tmp_ptr = malloc(src_size_ + nullptr_size_);
		if (tmp_ptr == 0) {
			return 0;
		}
		std::memset(tmp_ptr, 0, src_size_ + nullptr_size_);
		std::memcpy(tmp_ptr, src_ptr_, src_size_);
		free(src_ptr_);
		src_ptr_ = tmp_ptr;
		return src_size_ + nullptr_size_;
	}
	// Get the smallest multiple of num_ that is greater than or equal to max_num_
	std::size_t round_up(std::size_t num_, std::size_t max_num_) const {
		if (num_ == 0) {
			return max_num_;
		}
		std::size_t remainder = max_num_ % num_;
		if (remainder == 0) {
			return max_num_;
		}
		return num_ + max_num_ - remainder;
	}
	// Release all allocated memory
	void release_memory(void) {
		std::deque<std::pair<std::size_t, void*> >::iterator itr;
		for (itr = _data.begin(); itr != _data.end(); ++itr) {
			delete itr->second;
		}
	}

private:
	std::deque<std::pair<std::size_t,void*> > _data;
};

#endif