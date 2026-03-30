#ifndef LFTUBRLA_GTTC_STRUCTSBUILDER_STRUCTSBUILDER_
#define LFTUBRLA_GTTC_STRUCTSBUILDER_STRUCTSBUILDER_

/*
*	Z.S. Create this file for GTTC. at November 28, 2021.
*	-----------------------------------------------------------------------------------------
*  	| 这个类能够构建以任何基础类型组成，具有任何字节对齐方式的任意结构体并使其以“void*”形式返回，			|
*	| 此“void*”指针能够强制转换为指向真实结构体的指针，用能读取其真实数据。							|
*   -----------------------------------------------------------------------------------------
*  	| 此类不是多线程安全的，这意味着如果需要在多线程程序中使用，则必须保证其读取不会出现不可预知的错误	 	|
*	| push_back/push_front/insert 函数提供了用于传递结构体中的数组的重载 							|
*	| operator<< 函数无法向函数中传递数组类型，仅可用于推送基本类型 								|
*   | 此类无法构建结构体中含有需要调用其构造函数的项的结构体  										|
*   -----------------------------------------------------------------------------------------
*/

#include <deque>
#include <memory>
#include <utility>

#ifndef _LFUBRLA_CHANGE_ARG_
	#define _LFUBRLA_CHANGE_ARG_	// 以此宏修饰的参数在函数调用成功将被更改
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
	* 函数说明:	向未构建的结构体后插入一个项
	* 
	* 参数:		const T & data_ 要插入的值
	* 
	* 返回值:	StructsBuilder& 当前对象的引用
	*************************************/
	template<typename T> StructsBuilder& push_back(const T& data_) {
		_data.push_back(std::pair<std::size_t, void*>(sizeof(T), new_data(data_)));
		return *this;
	}
	/************************************
	* 函数说明:	向未构建的结构体后插入一个项(用于支持结构体中数组类型的重载)
	* 
	* 参数:		const T * data_ 要插入的数组的指针
	* 参数:		std::size_t size_ 要插入的数组的大小
	* 
	* 返回值:	StructsBuilder& 当前对象的引用
	*************************************/
	template<typename T> StructsBuilder& push_back(const T* data_, std::size_t size_) {
		for (std::size_t i = 0; i < size_; ++i) {
			push_back(data_[i]);
		}
		return *this;
	}
	/************************************
	* 函数说明:	向未构建的结构体前插入一个项
	*
	* 参数:		const T & data_ 要插入的值
	*
	* 返回值:	StructsBuilder& 当前对象的引用
	*************************************/
	template<typename T> StructsBuilder& push_front(const T& data_) {
		_data.push_front(std::pair<std::size_t, void*>(sizeof(T), new_data(data_)));
		return *this;
	}
	/************************************
	* 函数说明:	向未构建的结构体前插入一个项(用于支持结构体中数组类型的重载)
	* 
	* 参数:		const T * data_ 要插入的数组的指针
	* 参数:		std::size_t size_ 要插入的数组的大小
	* 
	* 返回值:	StructsBuilder& 当前对象的引用
	*************************************/
	template<typename T> StructsBuilder& push_front(const T* data_, std::size_t size_) {
		for (std::size_t i = 0; i < size_; ++i) {
			push_front(data_[i]);
		}
		return *this;
	}
	/************************************
	* 函数说明:	向未构建的结构体中插入一个项
	* 
	* 参数:		std::size_t position_ 要插入到的位置(下标[以0为第一个])
	* 参数:		const T & data_ 要插入的值
	* 
	* 返回值:	StructsBuilder& 当前对象的引用
	*************************************/
	template<typename T> StructsBuilder& insert(std::size_t position_, const T& data_) {
		_data.insert(_data.begin() + position_, std::pair<std::size_t, void*>(sizeof(T), new_data(data_)));
		return *this;
	}
	/************************************
	* 函数说明:	向未构建的结构体中插入一个项(用于支持结构体中数组类型的重载)
	* 
	* 参数:		std::size_t position_ 要插入到的位置(下标[以0为第一个])
	* 参数:		const T * data_ 要插入的数组的指针
	* 参数:		std::size_t size_ 要插入的数组的大小
	* 
	* 返回值:	StructsBuilder& 当前对象的引用
	*************************************/
	template<typename T> StructsBuilder& insert(std::size_t position_, const T* data_, std::size_t size_) {
		for (std::size_t i = 0; i < size_; ++i) {
			insert(position_ + i, data_[i]);
		}
		return *this;
	}
	/************************************
	* 函数说明:	源源不断地向未构建的结构体末尾插入项
	* 
	* 参数:		const T & data_ 要插入的值
	* 
	* 返回值:	StructsBuilder& 当前对象的引用
	*************************************/
	template<typename T> StructsBuilder& operator<<(const T& data_) {
		return push_back(data_);
	}
	/************************************
	* 函数说明:	从当前构建的结构体中读取其内存
	* 
	* 参数:		_LF_CHANGE_ARG_ void * & data_ 成功读取后，该结构体的会被存入此指针指向的地址
	* 参数:		std::size_t byte_alignment_ 结构体字节对齐方式(0为系统默认字节对齐方式)
	* 
	* 返回值:	std::size_t 成功读取到的内存所占字节大小
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
	* 函数说明:	判断当前是否仍未构建任何结构体
	* 
	* 参数:		void
	* 
	* 返回值:	bool true:当前结构体是空的，false:当前结构体不为空
	*************************************/
	bool empty(void) const {
		return _data.empty();
	}
	/************************************
	* 函数说明:	清除所有正在构建的内容
	* 
	* 参数:		void
	* 
	* 返回值:	StructsBuilder& 当前对象的引用
	*************************************/
	StructsBuilder& clear(void) {
		release_memory();
		_data.clear();
		return *this;
	}

private:
	// 开辟一块新的内存
	template<typename T> void* new_data(const T& data_) {
		return new T(data_);
	}
	// 默认字节对齐下获取当前构建的结构体内存
	std::size_t default_memory_setting(_LFUBRLA_CHANGE_ARG_ void*& data_) const {
		std::size_t all_size = 0;
		std::deque<std::pair<std::size_t, void*> >::const_iterator itr;
		std::size_t max_size = 0;
		for (itr = _data.cbegin(); itr != _data.cend(); ++itr) {
			if (itr->first > max_size) {
				max_size = itr->first;
			}
			// 默认字节对齐下，结构体内存总是朝着下一数据的内存倍数偏移
			if (all_size % itr->first != 0) {
				std::size_t next_size = round_up(itr->first, all_size);
				push_nullptr(data_, all_size, next_size - all_size);
				all_size = next_size;
			}
			memory_push(data_, all_size, itr->second, itr->first);
			all_size += itr->first;
		}
		// 默认字节对齐下，结构体总内存总是向结构体内最大元素占用内存的倍数偏移
		if (all_size % max_size != 0) {
			std::size_t next_size = round_up(max_size, all_size);
			push_nullptr(data_, all_size, next_size - all_size);
			all_size = next_size;
		}
		return all_size;
	}
	// 非默认字节对齐下获取当前构建的结构体内存
	std::size_t memory_setting(_LFUBRLA_CHANGE_ARG_ void*& data_, std::size_t byte_alignment_) const {
		std::size_t all_size = 0;
		std::deque<std::pair<std::size_t, void*> >::const_iterator itr;
		std::size_t max_size = 0;
		for (itr = _data.cbegin(); itr != _data.cend(); ++itr) {
			// 非默认字节对齐下，当前元素的内存对其方式取决于元素的对齐方式与指定对其方式中较小的一个
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
		// 非默认字节对齐下，结构体总内存总是向使用过的最大偏移量的倍数偏移
		if (all_size % max_size != 0) {
			std::size_t next_size = round_up(max_size, all_size);
			push_nullptr(data_, all_size, next_size - all_size);
			all_size = next_size;
		}
		return all_size;
	}
	// 在某内存后添加新的内存
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
	// 在某内存后填充空字节
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
	// 获取某数最接近某值并比其大的倍数
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
	// 释放所有填充的内存
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