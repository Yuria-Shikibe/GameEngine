//
// Created by Matrix on 2024/4/21.
//

export module ext.DynamicBuffer;

import std;

export namespace ext{
	template <std::ranges::range T = std::vector<char>>
		requires requires (T t){
			t.push_back(char{});
		}
	class DynamicBuffer final : public std::streambuf{
	public:
		explicit DynamicBuffer(T& vec) : vec(vec){
			setg(vec.data(), vec.data(), vec.data() + vec.size());
		}

	protected:
		int_type overflow(const int_type c) override{
			if(c != std::char_traits<char>::eof()){
				vec.push_back(c);
				return c;
			}
			return std::char_traits<char>::eof();
		}

	private:
		T& vec;
	};
}
