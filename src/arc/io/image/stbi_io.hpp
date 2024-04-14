//
// Created by Matrix on 2024/3/24.
//

#ifndef STBI_IO_HPP
#define STBI_IO_HPP
inline void* resize(const void* p, const size_t oldsz, const size_t newsz){
	void* next = operator new[](newsz);

	for(int i = 0; i < oldsz; ++i){ //Dumb
		static_cast<char*>(next)[i] = static_cast<const char*>(p)[i];
	}

	return next;
}

#define STBI_MALLOC(sz)           operator new [](sz)
#define STBI_REALLOC_SIZED(p,oldsz,newsz)     resize(p,oldsz,newsz)
#define STBI_FREE(p)              operator delete [](p)
#endif //STBI_IO_HPP
