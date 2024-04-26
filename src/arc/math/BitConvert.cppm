module ;

export module Math.Bit;

import std;
// Char

static std::stringstream tmpSS;
static const std::string EMPTY;

export namespace Math{
	constexpr size_t INT_MASK_BACK = 0x00000000ffffffffL;
	constexpr size_t INT_MASK_FRONT = 0xffffffff00000000L;

	int charToDigitValue(const char c, const int radix) {
		int digitValue = 0;

		if (std::isdigit(c)) {
			if (const int numericValue = c - '0'; numericValue < radix) {
				digitValue = numericValue;
			}
		}

		else if (std::isalpha(c)) {
			if (const int numericValue = std::toupper(c) - 'A' + 10; numericValue < radix) {
				digitValue = numericValue;
			}
		}

		return digitValue;
	}


	inline std::string uintToHesStr(const unsigned int i, std::stringstream& ss) {
		ss << std::hex << i;

		return ss.str();
	}

	inline std::string uintToHesStr(const unsigned int i) {
		tmpSS.str(EMPTY);

		return uintToHesStr(i, tmpSS);
	}

	inline std::string ulongToHesStr(const size_t i, std::stringstream& ss) {
		ss << std::hex << i;

		return ss.str();
	}

	inline std::string ulongToHesStr(const size_t l) {
		tmpSS.str(EMPTY);

		return ulongToHesStr(l, tmpSS);
	}

	// Double Long Converts

	constexpr size_t pack(const float x, const float y) {
		return (static_cast<size_t>(std::bit_cast<unsigned>(x)) << 32) + std::bit_cast<unsigned>(y);
	}

	constexpr unsigned int unpackX(const size_t hash) {
		return static_cast<int>(hash & INT_MASK_BACK);
	}

	constexpr unsigned int unpackY(const size_t hash) {
		return static_cast<int>((hash & INT_MASK_FRONT) >> 32);
	}

	constexpr float unpackX_Float(const size_t hash) {
		return std::bit_cast<float>(static_cast<int>(hash & INT_MASK_BACK));
	}

	constexpr float unpackY_Float(const size_t hash) {
		return std::bit_cast<float>(static_cast<int>((hash & INT_MASK_FRONT) >> 32));
	}
}